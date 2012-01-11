#ifndef RDE_HASH_H_
#define RDE_HASH_H_

#include <util/hashFunction.h>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <am/am.h>
#include <am/concept/DataType.h>


NS_IZENELIB_AM_BEGIN

template <class KeyType, class ValueType = NullType,
          class HashFunc = rde::hash<KeyType> >
class rde_hash: public AccessMethod<KeyType, ValueType>
{
    typedef rde::hash_map<KeyType, ValueType, HashFunc> ContainerType;
    typedef typename ContainerType::iterator IT;
    typedef rde::pair<IT, bool> PAIR;

public:
    bool insert(const KeyType& key, const ValueType& value)
    {
        PAIR ret = map_.insert(rde::pair<KeyType, ValueType>(key, value));
        return ret.second;
    }

    bool insert(const DataType<KeyType, ValueType>& rec)
    {
        return insert(rec.key, rec.value);
    }

    bool get(const KeyType&key, ValueType& value)
    {
        IT it = map_.find(key);
        if (it != map_.end() )
        {
            value = it->second;
            return true;
        }
        return false;
    }

    bool del(const KeyType& key)
    {
        size_t ret = map_.erase(key);
        return ret;
    }

    int num_items()
    {
        return map_.size();
    }

    ValueType* find(const KeyType& key)
    {
        IT it = map_.find(key);
        if (it != map_.end())
        {
            return &(it->second);
        }
        else
        {
            return NULL;
        }
    }

    void clear()
    {
        map_.clear();
    }

private:
    ContainerType map_;
};

NS_IZENELIB_AM_END

#endif /*RDE_HASH_H_*/
