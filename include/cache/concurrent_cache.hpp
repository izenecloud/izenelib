#ifndef IZENELIB_CONCURRENT_CACHE_H
#define IZENELIB_CONCURRENT_CACHE_H

#include "IzeneCacheTraits.h"
#include <stdint.h>
#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
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
    typedef folly::RWTicketSpinLockT<32, true> ItemRWLock;
    ItemRWLock rw_lock;
    ContainerT item_list;
    boost::atomic<int32_t>  access_info_index;
    CacheItem()
        :access_info_index(-1)
    {
    }
};

static const uint32_t MULT_FREE_LIST_NUM = 16;

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
            int cnt = 0;
            while (lock_.test_and_set(boost::memory_order_acquire))
            {
                if (++cnt > 10) sched_yield();
            }
        }
        inline void unlock()
        {
            lock_.clear(boost::memory_order_release);
        }
        inline bool try_lock()
        {
            return !lock_.test_and_set(boost::memory_order_acquire);
        }
    };

    ConcurrentCache(std::size_t cache_size, izenelib::cache::REPLACEMENT_TYPE evit_strategy,
        int32_t wash_out_interval_sec = 60, double wash_out_threshold = 0.1)
    {
        evit_strategy_ = evit_strategy;
        wash_out_interval_sec_ = wash_out_interval_sec;
        wash_out_threshold_ = wash_out_threshold;
        need_exit_ = false;
        wash_out_by_full_ = false;
        total_get_cnt_ = 0;
        total_hit_cnt_ = 0;
        init(cache_size);
    }

    ~ConcurrentCache()
    {
        need_exit_ = true;
        wash_out_cond_.notify_all();
        wash_out_thread_.join();
        free_access_info_list_.clear();
        if (free_list_lock_)
            delete[] free_list_lock_;
        if (item_buffer_)
            delete[] item_buffer_;
        item_buffer_ = NULL;
        if (access_info_list_)
            delete[] access_info_list_;
        access_info_list_ = NULL;
        item_buffer_size_ = 0;
        access_info_list_size_ = 0;
    }

    void init(std::size_t cache_size)
    {
        // keep some free space to reduce collision. Use the prime as bucket size to reduce collision.
        item_buffer_size_ = cal_next_prime(cache_size*4);
        item_buffer_ = new ItemT[item_buffer_size_];
        access_info_list_size_ = cache_size;
        access_info_list_ = new ItemAccessInfo[access_info_list_size_];

        free_list_lock_ = new spinlock[MULT_FREE_LIST_NUM];
        free_access_info_list_.resize(MULT_FREE_LIST_NUM);
        free_size_list_.resize(MULT_FREE_LIST_NUM, 0);
        for (std::size_t i = 0; i < access_info_list_size_; ++i)
        {
            free_access_info_list_[i%MULT_FREE_LIST_NUM].push_back(i);
            ++free_size_list_[i % MULT_FREE_LIST_NUM];
        }

        std::cout << "init hash bucket size : " << item_buffer_size_ << ", access list size: " << access_info_list_size_ << std::endl;
        wash_out_thread_ = boost::thread(boost::bind(&ConcurrentCache::wash_out_bg, this));
    }

    bool insert(const KeyType& key, const ValueType& value, bool overwrite = true)
    {
        std::size_t bucket_index = getBucketIndex(key);
        typename ItemT::ItemRWLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
        ItemT& item = item_buffer_[bucket_index];
        if (item.access_info_index == -1)
        {
            assert(item.item_list.empty());
            // first key in this bucket.
            // found a non-used access info for this bucket.
            std::size_t free_list_num = bucket_index % MULT_FREE_LIST_NUM;
            while (true)
            {
                std::size_t free_index = -1;

                free_list_lock_[free_list_num].lock();
                if (!free_access_info_list_[free_list_num].empty())
                {
                    free_index = free_access_info_list_[free_list_num].front();
                    free_access_info_list_[free_list_num].pop_front();
                    --free_size_list_[free_list_num];
                }
                free_list_lock_[free_list_num].unlock();

                if (free_index == (std::size_t)-1)
                    break;
                if (free_index >= access_info_list_size_)
                    continue;
                int32_t nonused = -1;
                if (access_info_list_[free_index].item_cache_index.compare_exchange_weak(nonused, (uint32_t)bucket_index))
                {
                    item.item_list.push_back(std::make_pair(key, value));
                    item.access_info_index = free_index;
                    update_access_info(free_index);
                    return true;
                }
                else
                {
                    std::cerr << "exchange failed for item cache index." << std::endl;
                    break;
                }
            }
            //for (std::size_t i = 0; i < access_info_list_size_; ++i)
            //{
            //    int32_t nonused = -1;
            //    if (access_info_list_[i].item_cache_index.compare_exchange_weak(nonused, (int32_t)bucket_index))
            //    {
            //        item.item_list.push_back(std::make_pair(key, value));
            //        item.access_info_index = i;
            //        update_access_info(i);
            //        return true;
            //    }
            //}
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
                //if (item.item_list.size() > 5)
                //    std::cerr << "hash collision is heavy : " << item.item_list.size() << std::endl;
            }

            update_access_info(item.access_info_index);
        }
        return true;
    }

    bool get(const KeyType& key, ValueType& value)
    {
        ++total_get_cnt_;
        std::size_t bucket_index = getBucketIndex(key);
        typename ItemT::ItemRWLock::ReadHolder guard(item_buffer_[bucket_index].rw_lock);
        const ItemT& item = item_buffer_[bucket_index];
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
                    ++total_hit_cnt_;
                    return true;
                }
            }
        }
        return false;
    }

    void remove(const KeyType& key)
    {
        std::size_t bucket_index = getBucketIndex(key);
        int32_t access_index = -1;
        {
            typename ItemT::ItemRWLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
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
            access_index = item.access_info_index;
            reset_access_info(item.access_info_index);
            item.access_info_index = -1;
        }
        free_access_info_to_list(access_index);
    }

    void clear()
    {
        for(std::size_t i = 0; i < item_buffer_size_; ++i)
        {
            clear_bucket(i);
        }
    }

    bool check_correctness()
    {
        // check free list should be really free and all free pos should be in free list
        for (std::size_t i = 0; i < MULT_FREE_LIST_NUM; ++i)
        {
            free_list_lock_[i].lock();
            std::set<std::size_t> diff_set;
            bool check_ok = true;
            for (std::list<std::size_t>::const_iterator it = free_access_info_list_[i].begin();
                it != free_access_info_list_[i].end(); ++it)
            {
                if (*it >= access_info_list_size_)
                {
                    // free pos should be no larger than size.
                    std::cerr << "free access list position larger than size." << *it << std::endl;
                    check_ok = false;
                    break;
                }
                if (access_info_list_[*it].item_cache_index != -1)
                {
                    std::cerr << "free access list position not really free." << std::endl;
                    check_ok = false;
                    break;
                }
                if (diff_set.find(*it) != diff_set.end())
                {
                    std::cerr << "free access list position duplicate." << std::endl;
                    check_ok = false;
                    break;
                }
                diff_set.insert(*it);
            }
            if (free_access_info_list_[i].size() != free_size_list_[i])
            {
                std::cerr << "free access list size mismatch." << std::endl;
                check_ok = false;
            }

            free_list_lock_[i].unlock();
            if (!check_ok)
                return false;
        }
        //
        // check bucket access info consistent.
        for(std::size_t i = 0; i < item_buffer_size_; ++i)
        {
            typename ItemT::ItemRWLock::ReadHolder guard(item_buffer_[i].rw_lock);
            const ItemT& item = item_buffer_[i];
            if (!item.item_list.empty() && item.access_info_index != -1)
            {
                if (item.access_info_index >= (int32_t)access_info_list_size_)
                {
                    std::cerr << "bucket access info pos out of range." << std::endl;
                    return false;
                }
                if (access_info_list_[item.access_info_index].item_cache_index != (int32_t)i)
                {
                    std::cerr << "bucket access info pos is not consistent. " << i
                        << "-" << item.access_info_index << "-" << access_info_list_[item.access_info_index].item_cache_index << std::endl;
                    return false;
                }
            }

        }
        return true;
    }

    std::string get_useful_info()
    {
        std::string retstr("Concurrent cache statistic:\n");
        retstr += "Current free list size: \n";
        for(std::size_t i = 0; i < MULT_FREE_LIST_NUM; ++i)
        {
            retstr += boost::lexical_cast<std::string>(free_size_list_[i]) + ", ";
        }
        retstr += "\n";

        retstr += "Hit ratio: " + boost::lexical_cast<std::string>((int64_t)total_hit_cnt_)
            + " / " + boost::lexical_cast<std::string>((int64_t)total_get_cnt_);
        return retstr;
    }

private:
    class CmpFunc
    {
    public:
        CmpFunc(const ItemAccessInfo* const access_info, std::size_t access_info_size, izenelib::cache::REPLACEMENT_TYPE evit_strategy)
            : access_info_(access_info), access_info_size_(access_info_size), evit_strategy_(evit_strategy)
        {
        }
        bool is_left_useless(const ItemAccessInfo& left, const ItemAccessInfo& right) const
        {
            if (evit_strategy_ == izenelib::cache::LRU)
            {
                if (left.last_time == right.last_time)
                    return left.get_cnt <= right.get_cnt;
                return left.last_time < right.last_time;
            }
            else if (evit_strategy_ == izenelib::cache::LFU)
            {
                if (left.get_cnt == right.get_cnt)
                    return left.last_time <= right.last_time;
                return left.get_cnt < right.get_cnt;
            }
            else
            {
                // for the frequency used cache item, we add some timestamp to leverage its importance.
                int64_t l = left.last_time + left.get_cnt*60*1000;
                int64_t r = right.last_time + right.get_cnt*60*1000;
                return l <= r;
            }
        }
        bool operator()(std::size_t left, std::size_t right) const
        {
            // return true if left less than right
            return left >= 0 && right >= 0 && left < access_info_size_
                && right < access_info_size_ && access_info_[left].last_time != 0
                && (0 == access_info_[right].last_time
                    || is_left_useless(access_info_[left], access_info_[right]));
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

    void reset_access_info(int32_t access_info_index)
    {
        if (access_info_index >= 0 && access_info_index < (int32_t)access_info_list_size_)
        {
            access_info_list_[access_info_index].last_time = 0;
            access_info_list_[access_info_index].get_cnt = 0;
            access_info_list_[access_info_index].item_cache_index = -1;
        }
    }

    void free_access_info_to_list(int32_t access_info_index)
    {
        if (access_info_index >= 0 && access_info_index < (int32_t)access_info_list_size_)
        {
            free_list_lock_[access_info_index % MULT_FREE_LIST_NUM].lock();
            free_access_info_list_[access_info_index % MULT_FREE_LIST_NUM].push_back(access_info_index);
            ++free_size_list_[access_info_index % MULT_FREE_LIST_NUM];
            free_list_lock_[access_info_index % MULT_FREE_LIST_NUM].unlock();
        }
    }

    void try_clear_bucket(std::size_t bucket_index)
    {
        if (bucket_index >= item_buffer_size_)
            return;
        int32_t access_index = -1;
        {
            if (!item_buffer_[bucket_index].rw_lock.try_lock())
            {
                return;
            }
            ItemT& item = item_buffer_[bucket_index];
            item.item_list.clear();
            access_index = item.access_info_index;
            reset_access_info(item.access_info_index);
            item.access_info_index = -1;
            item_buffer_[bucket_index].rw_lock.unlock();
        }
        free_access_info_to_list(access_index);
    }
    void clear_bucket(std::size_t bucket_index)
    {
        if (bucket_index >= item_buffer_size_)
            return;
        int32_t access_index = -1;
        {
            typename ItemT::ItemRWLock::WriteHolder guard(item_buffer_[bucket_index].rw_lock);
            ItemT& item = item_buffer_[bucket_index];
            item.item_list.clear();
            access_index = item.access_info_index;
            reset_access_info(item.access_info_index);
            item.access_info_index = -1;
        }
        free_access_info_to_list(access_index);
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
            std::cerr << "the cache access info data is not ready." << access_info_index << std::endl;
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
            bool is_need_scan = wash_out_many;
            if (!wash_out_many)
            {
                for(std::size_t i = 0; i < MULT_FREE_LIST_NUM; ++i)
                {
                    if (free_size_list_[i] <= wash_out_threshold_ * access_info_list_size_ / MULT_FREE_LIST_NUM)
                    {
                        is_need_scan = true;
                        break;
                    }
                }
            }
            if (!is_need_scan)
            {
                //std::cout << "No need to wash out. " << get_useful_info() << std::endl;
                continue;
            }
            struct timespec start_time;
            clock_gettime(CLOCK_MONOTONIC, &start_time);
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
                if (!wash_out_many)
                    std::cout << get_useful_info() << std::endl;
                continue;
            }
            struct timespec sort_time;
            clock_gettime(CLOCK_MONOTONIC, &sort_time);

            for (std::size_t i = 0; i < washheap.size(); ++i)
            {
                //std::cout << access_info_list_[washheap[i]].item_cache_index << "-" << access_info_list_[washheap[i]].last_time << ", ";
                try_clear_bucket(access_info_list_[washheap[i]].item_cache_index);
            }

            struct timespec end_time;
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            int64_t wash_cost = (end_time.tv_sec - start_time.tv_sec)*1000 + (end_time.tv_nsec - start_time.tv_nsec)/1000000;
            int64_t sort_cost = (sort_time.tv_sec - start_time.tv_sec)*1000 + (sort_time.tv_nsec - start_time.tv_nsec)/1000000;
            if (wash_cost > 200 || wash_out_many)
            {
                std::cout << "wash out cache item num : " << washheap.size()
                    << ", cost time:" << wash_cost << ", sort time:" << sort_cost << std::endl;
            }
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
    std::vector<std::list<std::size_t> >  free_access_info_list_;
    std::vector<std::size_t>  free_size_list_;
    spinlock  *free_list_lock_;
    boost::atomic<int64_t>  total_get_cnt_;
    boost::atomic<int64_t>  total_hit_cnt_;
};


}}

#endif
