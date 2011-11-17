#ifndef IZENELIB_IR_INMEMORYBTREECACHE_H_
#define IZENELIB_IR_INMEMORYBTREECACHE_H_
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <3rdparty/am/stx/btree_map.h>

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
        
        friend std::ostream& operator<<(std::ostream& output, const ValueType& v) {
            output<<"[I] ";
            for(uint32_t i=0;i<v.insert_item.size();i++)
            {
                output<<v.insert_item[i]<<",";
            }
            output<<"[D] ";
            for(uint32_t i=0;i<v.delete_item.size();i++)
            {
                output<<v.delete_item[i]<<",";
            }
            output<<"[U] ";
            for(uint32_t i=0;i<v.update_item.size();i++)
            {
                output<<v.update_item[i]<<",";
            }
            return output;
        }
    };
    
    typedef stx::btree_map<KeyType, ValueType> AMType;
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
        bool ordered = false;
        if(it.data().insert_item.size()>0)
        {
            if( it.data().insert_item.back()<value_item )
            {
                it.data().insert_item.push_back(value_item);
                ordered = true;
            }
        }
        else
        {
            it.data().insert_item.push_back(value_item);
            ordered = true;
        }
        if(!ordered)
        {
            it.data().update_item.push_back(value_item);
        }
        //expand the capacity
        ++capacity_;
        
        //test
        {
            typename AMType::iterator fit = data_.find(key);
            if(fit==data_.end())
            {
                std::cout<<"!!error"<<std::endl;
            }
            else
            {
                std::cout<<"fit : "<<fit->first<<","<<fit->second<<std::endl;
            }
        }
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
        it.data().delete_item.push_back(value_item);
        //expand the capacity
        ++capacity_;
    }
    
    std::size_t key_size() const
    {
        return data_.size();
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
//             KeyType key = it.key();
//             ValueType value = it.data();
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
        value = it.data();
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

