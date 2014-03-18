#ifndef IZENELIB_CONCURRENT_CACHE_H
#define IZENELIB_CONCURRENT_CACHE_H

#include "IzeneCacheTraits.h"
#include <stdint.h>
#include <boost/atomic.hpp>
#include <3rdparty/folly/RWSpinLock.h>
#include <util/hashFunction.h>

namespace izenelib { namespace concurrent_cache
{

struct ItemAccessInfo
{
    int64_t last_time;
    boost::atomic<int64_t> get_cnt;
    boost::atomic<int32_t> item_cache_index;
    ItemAccessInfo()
        :last_time(0), get_cnt(0), item_cache_index(-1)
    {
    }
};

template <class KeyType, class ValueType> class CacheItem
{
public:
    typedef std::list<std::pair<KeyType, ValueType> > ContainerT;
    folly::RWSpinLock rw_lock;
    ContainerT item_list;
    int32_t  access_info_index;
    CacheItem()
        :access_info_index(-1)
    {
    }
};

enum { PRIME_NUM = 28 };

static const int64_t PRIME_LIST[PRIME_NUM] =
{
    53l, 97l, 193l, 389l, 769l,
    1543l, 3079l, 6151l, 12289l, 24593l,
    49157l, 98317l, 196613l, 393241l, 786433l,
    1572869l, 3145739l, 6291469l, 12582917l, 25165843l,
    50331653l, 100663319l, 201326611l, 402653189l, 805306457l,
    1610612741l, 3221225473l, 4294967291l
};

inline int64_t cal_next_prime(int64_t n)
{
    int64_t ret = n;
    if (n > 0)
    {
        const int64_t* first = PRIME_LIST;
        const int64_t* last = PRIME_LIST + PRIME_NUM;
        const int64_t* pos = std::lower_bound(first, last, n);
        ret = ((pos == last) ? *(last - 1) : *pos);
    }
    return ret;
}

template <class KeyType, class ValueType> class ConcurrentCache
{
public:
    typedef CacheItem<KeyType, ValueType> ItemT;

    class spinlock {
        boost::atomic_flag lock_;
    public:
        spinlock()
        {
            lock_.clear();
        }
        inline void lock()
        {
            while (lock_.test_and_set(boost::memory_order_acquire));
        }
        inline void unlock()
        {
            lock_.clear(boost::memory_order_release);
        }
        inline bool try_lock()
        {
            return !lock_.test_and_set(boost::memory_order_acquire);
        }
    }__attribute__((aligned(64)));

    ConcurrentCache(std::size_t cache_size, izenelib::cache::REPLACEMENT_TYPE evit_strategy,
        int32_t wash_out_interval_sec = 10, double wash_out_threshold = 0.1)
    {
        evit_strategy_ = evit_strategy;
        wash_out_interval_sec_ = wash_out_interval_sec;
        wash_out_threshold_ = wash_out_threshold;
        need_exit_ = false;
        wash_out_by_full_ = false;
        init(cache_size);
    }
    ~ConcurrentCache()
    {
        need_exit_ = true;
        wash_out_cond_.notify_all();
        wash_out_thread_.join();
        delete[] item_buffer_;
        item_buffer_ = NULL;
        delete[] access_info_list_;
        access_info_list_ = NULL;
        item_buffer_size_ = 0;
        access_info_list_size_ = 0;
    }

    void init(std::size_t cache_size)
    {
        // keep some free space to reduce collision. Use the prime as bucket size to reduce collision.
        item_buffer_size_ = cal_next_prime(cache_size*4);
        std::cout << "init hash bucket size : " << item_buffer_size_ << std::endl;
        item_buffer_ = new ItemT[item_buffer_size_];
        access_info_list_size_ = cache_size;
        access_info_list_ = new ItemAccessInfo[access_info_list_size_];

        for (std::size_t i = 0; i < access_info_list_size_; ++i)
        {
            free_access_info_list_.push_back(i);
        }

        wash_out_thread_ = boost::thread(boost::bind(&ConcurrentCache::wash_out_bg, this));
    }

    bool insert(const KeyType& key, const ValueType& value, bool overwrite = true)
    {
        std::size_t bucket_index = getBucketIndex(key);
        folly::RWSpinLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
        ItemT& item = item_buffer_[bucket_index];
        if (item.access_info_index == -1)
        {
            assert(item.item_list.empty());
            // first key in this bucket.
            // found a non-used access info for this bucket.
            for (std::size_t i = 0; i < access_info_list_size_; ++i)
            {
                int32_t nonused = -1;
                if (access_info_list_[i].item_cache_index.compare_exchange_weak(nonused, (int32_t)bucket_index))
                {
                    item.item_list.push_back(std::make_pair(key, value));
                    item.access_info_index = i;
                    update_access_info(i);
                    return true;
                }
            }
            //std::cerr << "cache is full, need wash out." << std::endl;
            wash_out_by_full_ = true;
            wash_out_cond_.notify_all();
            return false;
        }
        else
        {
            // collision key
            bool is_exist = false;
            for(typename ItemT::ContainerT::iterator it = item.item_list.begin();
                it != item.item_list.end(); ++it)
            {
                if (it->first == key)
                {
                    if (overwrite)
                    {
                        it->second = value;
                    }
                    is_exist = true;
                }
            }
            if (!is_exist)
            {
                item.item_list.push_back(std::make_pair(key, value));
                if (item.item_list.size() > 3)
                    std::cerr << "hash collision is heavy : " << item.item_list.size() << std::endl;
            }

            update_access_info(item.access_info_index);
        }
        return true;
    }

    bool get(const KeyType& key, ValueType& value)
    {
        std::size_t bucket_index = getBucketIndex(key);
        folly::RWSpinLock::ReadHolder guard(item_buffer_[bucket_index].rw_lock);
        ItemT& item = item_buffer_[bucket_index];
        if (item.item_list.empty())
        {
            // not found in cache.
            return false;
        }
        else
        {
            // get item and udpate the LRU.
            for(typename ItemT::ContainerT::const_iterator it = item.item_list.begin();
                it != item.item_list.end(); ++it)
            {
                if (it->first == key)
                {
                    value = it->second;
                    update_access_info(item.access_info_index);
                    return true;
                }
            }
        }
        return false;
    }

    void remove(const KeyType& key)
    {
        std::size_t bucket_index = getBucketIndex(key);
        folly::RWSpinLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
        ItemT& item = item_buffer_[bucket_index];
        for (typename ItemT::ContainerT::iterator it = item.item_list.begin();
            it != item.item_list.end(); ++it)
        {
            if (it->first == key)
            {
                item.item_list.erase(it);
                break;
            }
        }
        if (!item.item_list.empty())
            return;
        if (item.access_info_index >= 0 && item.access_info_index < (int32_t)access_info_list_size_)
        {
            access_info_list_[item.access_info_index].last_time = 0;
            access_info_list_[item.access_info_index].get_cnt = 0;
            access_info_list_[item.access_info_index].item_cache_index = -1;
        }
        item.access_info_index = -1;
    }

    void clear()
    {
        for(std::size_t i = 0; i < item_buffer_size_; ++i)
        {
            clear_bucket(i);
        }
    }

private:
    class CmpFunc
    {
    public:
        CmpFunc(const ItemAccessInfo* const access_info, std::size_t access_info_size, izenelib::cache::REPLACEMENT_TYPE evit_strategy)
            : access_info_(access_info), access_info_size_(access_info_size), evit_strategy_(evit_strategy)
        {
        }
        bool is_left_useless(std::size_t left, std::size_t right) const
        {
            if (evit_strategy_ == izenelib::cache::LRU)
            {
                if (access_info_[left].last_time == access_info_[right].last_time)
                    return access_info_[left].get_cnt <= access_info_[right].get_cnt;
                return access_info_[left].last_time < access_info_[right].last_time;
            }
            else if (evit_strategy_ == izenelib::cache::LFU)
            {
                if (access_info_[left].get_cnt == access_info_[right].get_cnt)
                    return access_info_[left].last_time <= access_info_[right].last_time;
                return access_info_[left].get_cnt < access_info_[right].get_cnt;
            }
            else
            {
                // for the frequency used cache item, we add some timestamp to leverage its importance.
                int64_t l = access_info_[left].last_time + access_info_[left].get_cnt*60*1000;
                int64_t r = access_info_[right].last_time + access_info_[right].get_cnt*60*1000;
                return l <= r;
            }
        }
        bool operator()(std::size_t left, std::size_t right) const
        {
            // return true if left less than right
            return left >= 0 && right >= 0 && left < access_info_size_
                && right < access_info_size_ && access_info_[left].last_time != 0
                && (0 == access_info_[right].last_time
                    || is_left_useless(left, right));
        }
    private:
        const ItemAccessInfo* const access_info_;
        const std::size_t access_info_size_;
        const izenelib::cache::REPLACEMENT_TYPE& evit_strategy_;
    };
    std::size_t getBucketIndex(const KeyType& key)
    {
        uint32_t hashkey = hash_func_(key);
        return hashkey % item_buffer_size_;
    }

    void clear_bucket(std::size_t bucket_index)
    {
        if (bucket_index >= item_buffer_size_)
            return;
        folly::RWSpinLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
        ItemT& item = item_buffer_[bucket_index];
        item.item_list.clear();
        if (item.access_info_index >= 0 && item.access_info_index < (int32_t)access_info_list_size_)
        {
            access_info_list_[item.access_info_index].last_time = 0;
            access_info_list_[item.access_info_index].get_cnt = 0;
            access_info_list_[item.access_info_index].item_cache_index = -1;
        }
        item.access_info_index = -1;
    }

    void update_access_info(int32_t access_info_index)
    {
        if (access_info_index >= 0 &&
            access_info_index < (int32_t)access_info_list_size_)
        {
            ItemAccessInfo& info = access_info_list_[access_info_index];
            struct timespec cur_time;
            clock_gettime(CLOCK_MONOTONIC, &cur_time);
            info.last_time = cur_time.tv_sec * 1000 + cur_time.tv_nsec / 1000000;
            info.get_cnt.fetch_add(1, boost::memory_order_seq_cst);
        }
        else
        {
            std::cerr << "the cache access info data is not ready." << access_info_index;
        }
    }

    void wash_out_bg()
    {
        while(true)
        {
            if (need_exit_)
                break;
            boost::unique_lock<boost::mutex> lock(wash_out_mutex_);
            wash_out_cond_.timed_wait(lock, boost::posix_time::seconds(wash_out_interval_sec_));
            if (need_exit_)
                break;
            bool wash_out_many = false;
            if (wash_out_by_full_)
            {
                wash_out_by_full_ = false;
                wash_out_many = true;
            }
            // here, we ignore the updated access info during scan to avoid performance degrade.
            // There may be some inaccurate to evict some useful cache, but I think it will be rare.
            std::vector<int32_t> washheap;
            std::size_t heapsize = access_info_list_size_ * wash_out_threshold_;
            if (heapsize < 1)
                heapsize = 1;
            if (wash_out_many)
                heapsize *= 4; 
            washheap.reserve(heapsize);
            std::size_t freesize = 0;
            CmpFunc cmp_func(access_info_list_, access_info_list_size_, evit_strategy_);
            for (std::size_t i = 0; i < access_info_list_size_; ++i)
            {
                if (access_info_list_[i].item_cache_index == -1)
                {
                    freesize++;
                }
                else if (washheap.size() < heapsize || cmp_func(i, washheap.front()))
                {
                    if (washheap.size() < heapsize)
                    {
                        washheap.push_back(i);
                    }
                    else
                    {
                        std::pop_heap(washheap.begin(), washheap.end(), cmp_func);
                        washheap.back() = i;
                    }
                    std::push_heap(washheap.begin(), washheap.end(), cmp_func);
                }
                //std::cout << access_info_list_[i].item_cache_index << "-" << access_info_list_[i].last_time << ", ";
            }
            if (freesize >= heapsize)
            {
                //std::cout << "wash out ignore since there are some free. " << freesize << std::endl;
                continue;
            }
            for (std::size_t i = 0; i < washheap.size(); ++i)
            {
                //std::cout << access_info_list_[washheap[i]].item_cache_index << "-" << access_info_list_[washheap[i]].last_time << ", ";
                clear_bucket(access_info_list_[washheap[i]].item_cache_index);
            }
            std::cout << "wash out cache item num : " << washheap.size() << std::endl;
        }
        std::cerr << "cache wash out thread exit." << std::endl;
    }

    ItemT *item_buffer_;
    std::size_t item_buffer_size_;
    ItemAccessInfo*  access_info_list_;  // used for evict strategy.
    std::size_t access_info_list_size_;
    bool wash_out_by_full_;
    bool need_exit_;
    int32_t wash_out_interval_sec_;
    double wash_out_threshold_;
    izenelib::cache::REPLACEMENT_TYPE evit_strategy_;
    boost::mutex wash_out_mutex_;
    boost::condition_variable wash_out_cond_;
    boost::thread wash_out_thread_;
    HashIDTraits<KeyType, uint32_t>  hash_func_;
    std::list<std::size_t>  free_access_info_list_;
    spinlock  free_list_lock_;
};


}}

#endif
