#ifndef IZENELIB_IR_INMEMORYBTREECACHE_H_
#define IZENELIB_IR_INMEMORYBTREECACHE_H_
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <3rdparty/am/stx/btree_map.h>
#include <ir/index_manager/utility/BitVector.h>
#include <boost/dynamic_bitset.hpp>
// #define BTCACHE_DEBUG

NS_IZENELIB_IR_BEGIN
namespace indexmanager {

///@brief should be concurrency, one thread writing and multi-threads reading
template <class KeyType, class ValueItemType>
class InMemoryBTreeCache
{
public:

    class ValueType;
    class ValueTypeIterator
    {
    public:
        const ValueType* value;
        std::size_t index;
        ValueTypeIterator() : value(NULL), index(0)
        {
        }
        ValueTypeIterator(const ValueType* p1, std::size_t p2)
        : value(p1), index(p2)
        {
            forward(0);
        }
        void forward(std::size_t p)
        {
            if(index>=value->size()) return;
            std::size_t new_index = index+p;
            while(true)
            {
                std::size_t next = new_index+1;
                if(next>=value->size()) break;
                if((*value).item[new_index]!=(*value).item[next]) break;
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
            return (*value).item[index];
        }

        bool test() const
        {
            return (*value).flag[index];
        }
    };

    struct ValueType
    {

        typedef std::vector<ValueItemType> VectorType;
        typedef boost::dynamic_bitset<> FlagType;

        typedef typename VectorType::iterator VectorIteratorType;
        typedef typename VectorType::const_iterator VectorConstIteratorType;
        typedef ValueTypeIterator const_iterator;

        VectorType item;
        FlagType flag;

        void merge(std::size_t low, std::size_t mid, std::size_t high)
        {
            std::size_t h=low, i=low, j=mid+1;
            ValueType b(*this);
            while( h<=mid && j<=high )
            {
                if(item[h]<=item[j])
                {
                    b.item[i] = item[h];
                    b.flag[i] = flag[h];
                    ++h;
                }
                else
                {
                    b.item[i] = item[j];
                    b.flag[i] = flag[j];
                    ++j;
                }
                ++i;
            }
            if(h>mid)
            {
                for(std::size_t k=j;k<=high;k++)
                {
                    b.item[i] = item[k];
                    b.flag[i] = flag[k];
                    ++i;
                }
            }
            else
            {
                for(std::size_t k=h;k<=mid;k++)
                {
                    b.item[i] = item[k];
                    b.flag[i] = flag[k];
                    ++i;
                }
            }
            for(std::size_t k=low;k<=high;k++)
            {
                item[k] = b.item[k];
                flag[k] = b.flag[k];
            }
        }

        void merge_sort(std::size_t low, std::size_t high)
        {
            if(low<high)
            {
                std::size_t mid = (low+high)/2;
                merge_sort(low, mid);
                merge_sort(mid+1, high);
                merge(low, mid, high);
            }
        }
        ///stable merge sort
        void sort()
        {
            //return;
            if(size()<=1) return;
            //std::cerr<<"before"<<std::endl;
            //for(uint32_t i=0;i<size();i++)
            //{
                //std::cerr<<item[i]<<","<<flag[i]<<std::endl;
            //}
            merge_sort(0, size()-1);
            //std::cerr<<"after"<<std::endl;
            //for(uint32_t i=0;i<size();i++)
            //{
                //std::cerr<<item[i]<<","<<flag[i]<<std::endl;
            //}
        }

        void add(const ValueItemType& item_value, bool iorr)
        {
            item.push_back(item_value);
            flag.push_back(iorr);
        }

        void swap(ValueType& from)
        {
            std::swap(item, from.item);
            std::swap(flag, from.flag);
        }

        std::size_t size() const
        {
            return item.size();
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
                output << v.item[i] << ":" << (int)v.flag.test(i) << ",";
            }
            return output;
        }
    };

    typedef std::map<KeyType, ValueType> AMType;

    InMemoryBTreeCache()
        :capacity_(0), max_capacity_(-1)
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
//      boost::lock_guard<boost::shared_mutex> lock(mutex_);
        if (is_full())
        {
            std::cout << "cache full" << std::endl;
            return;
        }
        data_[key].add(value_item, 1);
        ++capacity_;

    }

    void remove(const KeyType& key, const ValueItemType& value_item)
    {
//      boost::lock_guard<boost::shared_mutex> lock(mutex_);
        if (is_full())
        {
            std::cout << "cache full" << std::endl;
            return;
        }
        data_[key].add(value_item, 0);
        ++capacity_;
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

    bool is_full()
    {
        return capacity_ >= max_capacity_;
    }

    void iterate(const boost::function<void (const std::pair<KeyType, ValueType> &)>& func)
    {
        typename AMType::iterator it = data_.begin();
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
        AMType data;
        data_.swap(data);
        capacity_ = 0;
    }

    //search apis
    bool get(const KeyType& key, ValueType& value)
    {
//      boost::shared_lock<boost::shared_mutex> lock(mutex_);
        typename AMType::iterator it = data_.find(key);
        if (it == data_.end())
        {
            return false;
        }
        value = it->second;
        value.sort();
        return true;
    }

    bool exist(const KeyType& key)
    {
        typename AMType::iterator it = data_.find(key);
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

private:
    AMType data_;
    std::size_t capacity_;
    std::size_t max_capacity_;
    boost::shared_mutex mutex_;
};

}

NS_IZENELIB_IR_END

#endif
