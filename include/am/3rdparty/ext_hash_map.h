#ifndef EXT_HASH_MAP_H_
#define EXT_HASH_MAP_H_

#include <util/hashFunction.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <boost/unordered_map.hpp>


NS_IZENELIB_AM_BEGIN

template <class KeyType, class ValueType,
          class HashFunctor = izenelib::util::HashFunctor<KeyType> >
class ext_hash_map: public AccessMethod<KeyType, ValueType>
{
    typedef typename boost::unordered_map<KeyType, ValueType, HashFunctor>::iterator IT;
    typedef pair<IT, bool> PAIR;

public:
    bool insert(const KeyType& key, const ValueType& value)
    {
        PAIR ret = hash_map_.insert(std::make_pair(key, value));
        return ret.second;
    }

    bool insert(const DataType<KeyType, ValueType>& rec)
    {
        return insert(rec.key, rec.value);
    }

    bool get(const KeyType&key, ValueType& value)
    {
        IT it = hash_map_.find(key);
        if (it != hash_map_.end() )
        {
            value = it->second;
            return true;
        }
        return false;

    }

    bool del(const KeyType& key)
    {
        size_t ret = hash_map_.erase(key);
        return ret;
    }

    int num_items()
    {
        return hash_map_.size();
    }

    ValueType* find(const KeyType& key)
    {
        IT it = hash_map_.find(key);
        if (it !=hash_map_.end())
        {
            return &(it->second);
        }
        else
        {
            return NULL;
        }
    }

private:
    boost::unordered_map<KeyType, ValueType, HashFunctor> hash_map_;
};

NS_IZENELIB_AM_END

#endif /*EXT_HASH_MAP_H_*/
