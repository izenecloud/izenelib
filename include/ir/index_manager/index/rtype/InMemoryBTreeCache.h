#ifndef IZENELIB_IR_INMEMORYBTREECACHE_H_
#define IZENELIB_IR_INMEMORYBTREECACHE_H_
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <3rdparty/am/stx/btree_map.h>
#include <am/concurrent/slfvector.h>
#include <boost/dynamic_bitset.hpp>
#include <util/ReadFavorLock.h>
#include <util/functional.h>
// #define BTCACHE_DEBUG

NS_IZENELIB_IR_BEGIN
namespace indexmanager {

///@brief should be concurrency, one thread writing and multi-threads reading
template <class KeyType, class ValueItemType, class MutexType>
class InMemoryBTreeCache
{
public:

    class ValueType;
    class ValueTypeIterator
    {
    public:
        const ValueType* value;
        std::size_t index;
        std::size_t vsize;
        ValueTypeIterator() : value(NULL), index(0), vsize(-1)
        {
        }
        ValueTypeIterator(const ValueType* p1, std::size_t p2)
        : value(p1), index(p2), vsize(value->count)
        {
            forward(0);
        }
        void forward(std::size_t p)
        {
            if(index>=vsize) return;
            std::size_t new_index = index+p;
            while(true)
            {
                std::size_t next = new_index+1;
                if(next>=vsize) break;
                if((*value).item[new_index].first!=(*value).item[next].first) break;
                ++new_index;
            }
            //if(new_index>value->size())
            //{
                //new_index = value->size();
            //}
            index = new_index;
        }
        ValueTypeIterator operator++()
        {
            if(value!=NULL)
            {
                forward(1);
            }
            return *this;
        }
        ValueTypeIterator operator++(int)
        {
            ValueTypeIterator o(*this);
            if(value!=NULL)
            {
                forward(1);
            }
            return o;
        }
        bool operator==(const ValueTypeIterator& o) const
        {
            return value==o.value&&index==o.index;
            //if(value==NULL&&o.value==NULL) return true;
            //if(value!=NULL&&o.value!=NULL)
            //{
                //return value.index==o.value.index;
            //}
            //return false;
        }
        bool operator!=(const ValueTypeIterator& o) const
        {
            return !(operator==(o));
        }
        ValueItemType operator*() const
        {
            return (*value).item[index].first;
        }

        bool test() const
        {
            return (*value).item[index].second;
        }
    };

    struct ValueType
    {

        typedef std::pair<ValueItemType, bool> VectorItemType;
        typedef std::vector<VectorItemType> VectorType;
        //typedef izenelib::am::concurrent::slfvector<VectorItemType> VectorType;
        //typedef typename VectorType::iterator VectorIteratorType;
        //typedef typename VectorType::const_iterator VectorConstIteratorType;
        typedef ValueTypeIterator const_iterator;
        //typedef boost::dynamic_bitset<> FlagType;

        ValueType() : count(0) {}

        std::size_t count;
        VectorType item;
        //FlagType flag;

        ValueType(const ValueType& v)
        : count(v.count), item(v.item.begin(), v.item.begin()+count)
        {
        }
        void merge(std::size_t low, std::size_t mid, std::size_t high, ValueType& b)
        {
            std::size_t h=low, i=low, j=mid+1;
            //ValueType b(*this);
            while( h<=mid && j<=high )
            {
                if(item[h].first<=item[j].first)
                {
                    b.item[i] = item[h];
                    //b.flag[i] = flag[h];
                    ++h;
                }
                else
                {
                    b.item[i] = item[j];
                    //b.flag[i] = flag[j];
                    ++j;
                }
                ++i;
            }
            if(h>mid)
            {
                for(std::size_t k=j;k<=high;k++)
                {
                    b.item[i] = item[k];
                    //b.flag[i] = flag[k];
                    ++i;
                }
            }
            else
            {
                for(std::size_t k=h;k<=mid;k++)
                {
                    b.item[i] = item[k];
                    //b.flag[i] = flag[k];
                    ++i;
                }
            }
            for(std::size_t k=low;k<=high;k++)
            {
                item[k] = b.item[k];
                //flag[k] = b.flag[k];
            }
        }

        void merge_sort(std::size_t low, std::size_t high, ValueType& buffer)
        {
            if(low<high)
            {
                std::size_t mid = (low+high)/2;
                merge_sort(low, mid, buffer);
                merge_sort(mid+1, high, buffer);
                merge(low, mid, high, buffer);
            }
        }
        ///stable merge sort
        void sort()
        {
            //std::stable_sort(item.begin(), item.end(), izenelib::util::first_less<VectorItemType>());
            std::size_t vsize = count;
            if(vsize<=1) return;
            ValueType buffer;
            buffer.item.resize(vsize);
            //buffer.flag.resize(size());
            merge_sort(0, vsize-1, buffer);
        }

        void add(const ValueItemType& item_value, bool iorr)
        {
            item.push_back(std::make_pair(item_value,iorr));
            //flag.push_back(iorr);
        }

        void swap(ValueType& from)
        {
            std::swap(item, from.item);
            std::swap(count, from.count);
            //std::swap(flag, from.flag);
        }

        void clear()
        {
            count=0;
            item.clear();
        }

        bool empty() const
        {
            return size()==0;
        }

        std::size_t size() const
        {
            return count;
        }

        const_iterator begin() const
        {
            return const_iterator(this, 0);
        }

        const_iterator end() const
        {
            return const_iterator(this, size());
        }

        friend std::ostream& operator<<(std::ostream& output, const ValueType& v)
        {
            for (std::size_t i = 0; i < v.item.size(); i++)
            {
                output << v.item[i].first << ":" << (int)v.item[i].second << ",";
            }
            return output;
        }
    };

    typedef std::map<KeyType, ValueType> AMType;

    InMemoryBTreeCache(MutexType& mutex)
        :capacity_(0), max_capacity_(-1), mutex_(mutex)
    {
    }

    ~InMemoryBTreeCache()
    {
    }

    ///capacity means the number of ValueItem
    void set_max_capacity(std::size_t m)
    {
        max_capacity_ = m;
    }

    void add(const KeyType& key, const ValueItemType& value_item)
    {
        insert2_(key, value_item, true);
    }

    void remove(const KeyType& key, const ValueItemType& value_item)
    {
        insert2_(key, value_item, false);
    }

    std::size_t key_size() const
    {
        return data_.size();
    }

    bool empty() const
    {
        return data_.empty();
    }

    std::size_t capacity() const
    {
        return capacity_;
    }

    bool is_full() const
    {
        return capacity_ >= max_capacity_;
    }

    void iterate(const boost::function<void (const std::pair<KeyType, ValueType> &)>& func) const
    {
        typename AMType::const_iterator it = data_.begin();
        while (it != data_.end())
        {
            std::pair<KeyType, ValueType> value = *it;
            value.second.sort();
            func(value);
            ++it;
        }

    }

    void clear()
    {
        data_.clear();
        //AMType data;
        //data_.swap(data);
        capacity_ = 0;
    }

    void clear_key(const KeyType& key)
    {
        typename AMType::iterator it = data_.find(key);
        if(it!=data_.end())
        {
            it->second.clear();
            //data_.erase(it);
        }
    }

    //search apis
    bool get(const KeyType& key, ValueType& value) const
    {
//      boost::shared_lock<boost::shared_mutex> lock(mutex_);
        typename AMType::const_iterator it = data_.find(key);
        if (it == data_.end())
        {
            return false;
        }
        value = it->second;
        value.sort();
        return true;
    }

    bool exist(const KeyType& key) const
    {
        typename AMType::const_iterator it = data_.find(key);
        if (it == data_.end())
            return false;
        else
            return true;
    }

    const AMType& getAM() const
    {
        return data_;
    }

    AMType& getAM()
    {
        return data_;
    }


private:
    void insert_(const KeyType& key, const ValueItemType& value_item, bool b)
    {
        //boost::lock_guard<MutexType> lock(mutex_);
        boost::upgrade_lock<MutexType> lock(mutex_);
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            boost::upgrade_to_unique_lock<MutexType> unique_lock(lock);
            //one thread write
            ValueType value;
            value.item.push_back(std::make_pair(value_item,b));
            it = data_.insert(std::make_pair(key, value)).first;
            ++capacity_;
            return;
            //it = data_.find(key);
            //if(it = data_.end())
            //{
            //}
        }
        //if(it->second.item.capacity()==it->second.item.size())
        //{
            //boost::upgrade_to_unique_lock<MutexType> unique_lock(lock);
            //it->second.item.push_back(std::make_pair(value_item, b));
        //}
        //else
        //{
            //it->second.item.push_back(std::make_pair(value_item, b));
        //}
        it->second.item.push_back(std::make_pair(value_item, b));
        ++capacity_;

        //if (is_full())
        //{
            //std::cout << "cache full" << std::endl;
            //return;
        //}
        //data_[key].add(value_item, b);
    }

    void insert2_(const KeyType& key, const ValueItemType& value_item, bool b)
    {
        //boost::lock_guard<MutexType> lock(mutex_);
        //LOG(ERROR)<<"start insert2"<<std::endl;
        //boost::upgrade_lock<MutexType> lock(mutex_);
        mutex_.lock_shared();
        //LOG(ERROR)<<"start insert2 upgrade lock got"<<std::endl;
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            //LOG(ERROR)<<"try to upgrade to unique1"<<std::endl;
            mutex_.unlock_shared();
            mutex_.lock();
            //boost::upgrade_to_unique_lock<MutexType> unique_lock(lock);
            //LOG(ERROR)<<"unique1 got"<<std::endl;
            //one thread write
            ValueType value;
            value.item.reserve(10);
            value.item.push_back(std::make_pair(value_item,b));
            value.count++;
            it = data_.insert(std::make_pair(key, value)).first;
            ++capacity_;
            mutex_.unlock();
            return;
            //it = data_.find(key);
            //if(it = data_.end())
            //{
            //}
        }
        //it->second.item.push_back(std::make_pair(value_item, b));
        //mutex_.unlock_shared();
        if(it->second.item.capacity()==it->second.item.size())
        {
            //LOG(ERROR)<<"try to upgrade to unique2"<<std::endl;

            mutex_.unlock_shared();
            mutex_.lock();
            //boost::upgrade_to_unique_lock<MutexType> unique_lock(lock);
            //LOG(ERROR)<<"unique2 got"<<std::endl;
            it->second.item.push_back(std::make_pair(value_item, b));
            it->second.count++;
            mutex_.unlock();
        }
        else
        {
            it->second.item.push_back(std::make_pair(value_item, b));
            it->second.count++;
            mutex_.unlock_shared();
        }
        ++capacity_;

        //if (is_full())
        //{
            //std::cout << "cache full" << std::endl;
            //return;
        //}
        //data_[key].add(value_item, b);
    }

private:
    AMType data_;
    std::size_t capacity_;
    std::size_t max_capacity_;
    MutexType& mutex_;
};

}

NS_IZENELIB_IR_END

#endif
