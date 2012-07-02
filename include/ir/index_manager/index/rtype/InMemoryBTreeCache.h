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
    struct ValueType
    {
        typedef std::vector<ValueItemType> VectorType;
        typedef boost::dynamic_bitset<> FlagType;
        
        typedef typename VectorType::iterator VectorIteratorType;
        
        typedef typename VectorType::const_iterator VectorConstIteratorType;
        
        
        VectorType item;
        FlagType flag;
        
        
        void add(const ValueItemType& item_value, bool iorr)
        {
//             std::size_t index = item.size();
//             if(iorr)
//             {
//                 flag.set(index);
//             }
//             else
//             {
//                 flag.clear(index);
//             }
            
            item.push_back(item_value);
            flag.push_back(iorr);
            
        }
        
        friend std::ostream& operator<<(std::ostream& output, const ValueType& v) {
            for(std::size_t i=0;i<v.item.size();i++)
            {
                output<<v.item[i]<<":"<<(int)v.flag.test(i)<<",";
            }
            return output;
        }
    };
    
    typedef std::map<KeyType, ValueType> AMType;
//     typedef AMType::iterator iterator;
    
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
    
    void add(const KeyType& key, const ValueItemType& value_item)
    {
//         boost::lock_guard<boost::shared_mutex> lock(mutex_);
        if(is_full()) 
        {
            std::cout<<"cache full"<<std::endl;
            return;
        }
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            it = data_.insert(std::make_pair(key, ValueType())).first;
        }
        it->second.add(value_item, 1);

        //expand the capacity
        ++capacity_;
        
    }
    
    void remove(const KeyType& key, const ValueItemType& value_item)
    {
//         boost::lock_guard<boost::shared_mutex> lock(mutex_);
        if(is_full()) 
        {
            std::cout<<"cache full"<<std::endl;
            return;
        }
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            it = data_.insert(std::make_pair(key, ValueType())).first;
        }
        it->second.add(value_item, 0);
        //expand the capacity
        ++capacity_;
    }
    
    std::size_t key_size() const
    {
        return data_.size();
    }
    
    bool empty() const
    {
        return key_size()==0;
    }
    
    std::size_t capacity() const
    {
        return capacity_;
    }
    
    bool is_full()
    {
        return max_capacity_>0 && capacity_>=max_capacity_;
    }
    
    void iterate(const boost::function<void (const std::pair<KeyType, ValueType>&)>& func)
    {
        typename AMType::iterator it = data_.begin();
        while(it!=data_.end())
        {
//             KeyType key = it->first;
//             ValueType value = it->second;
            func(*it);
            ++it;
        }
        
    }
    
    void clear()
    {
        data_.clear();
        capacity_ = 0;
    }
    
    //search apis
    bool get(const KeyType& key, ValueType& value)
    {
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            return false;
        }
        value = it->second;
        return true;
    }

    bool exist(const KeyType& key)
    {
        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
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

