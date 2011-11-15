#ifndef IZENELIB_IR_INMEMORYBTREECACHE_H_
#define IZENELIB_IR_INMEMORYBTREECACHE_H_


#include <boost/function.hpp>
#include <3rdparty/am/concurrent_hash/hashmap.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager {

///@brief should be concurrency, one thread writing and multi-threads reading
template <class KeyType, class ValueItemType>
class InMemoryBTreeCache
{
public:
    struct ValueType
    {
        std::vector<ValueItemType> insert_item;//should be ordered, and larger than current
        std::vector<ValueItemType> delete_item;
        std::vector<ValueItemType> update_item;//similar to insert_item but not ordered.
    };
    
    typedef concurrent::hashmap<KeyType, ValueType> AMType;
    
    InMemoryBTreeCache()
    :capacity_(0), max_capacity_(0)
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
    
    void add(const KeyType& key, const ValueItemType& value_item, bool is_u = false)
    {
        if(is_full()) return;
        std::pair<const KeyType, ValueType> kvp(key, ValueType());
        data_.get(key, kvp.second);
        if(!is_u)
        {
            kvp.second.insert_item.resize( kvp.second.insert_item.size()+1, value_item);//just expand 1
        }
        else
        {
            kvp.second.update_item.resize( kvp.second.update_item.size()+1, value_item);//just expand 1
        }
        data_.insert(kvp);
        //expand the capacity
        ++capacity_;
    }
    
    void remove(const KeyType& key, const ValueItemType& value_item)
    {
        if(is_full()) return;
        std::pair<const KeyType, ValueType> kvp(key, ValueType());
        data_.get(key, kvp.second);
        kvp.second.delete_item.resize( kvp.second.delete_item.size()+1, value_item);//just expand 1
        //TODO remove the value_item in insert_item and update_item to save memories?
        data_.insert(kvp);
        //expand the capacity
        ++capacity_;
    }
    
    bool is_full()
    {
        return max_capacity_>0 && capacity_>=max_capacity_;
    }
    
    void clear(const boost::function<void (const std::pair<KeyType, ValueType>&) >& func)
    {
        typename AMType::unsafe_iterator it = data_.unsafe_begin();
        std::pair<KeyType, ValueType> kvp;
        while(data_.get_and_next(it, kvp))
        {
            func(kvp);
        }
        //real clear
        data_.clear();
        capacity_ = 0;
    }
    
    bool get(const KeyType& key, ValueType& value)
    {
        return data_.get(key, value);
    }
    
    
private:
    AMType data_;
    std::size_t capacity_;
    std::size_t max_capacity_;
};

}
NS_IZENELIB_IR_END

#endif
