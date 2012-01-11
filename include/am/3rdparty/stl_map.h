#ifndef STL_MAP_H_
#define STL_MAP_H_

#include <util/hashFunction.h>
#include <am/am.h>
#include <am/concept/DataType.h>

#include <map>


NS_IZENELIB_AM_BEGIN

template <class KeyType, class ValueType,
          class Compare = less<KeyType> >
class stl_map: public AccessMethod<KeyType, ValueType>
{
    typedef typename std::map<KeyType, ValueType, Compare>::iterator IT;
    typedef pair<IT, bool> PAIR;

public:
    bool insert(const KeyType& key, const ValueType& value)
    {
        PAIR ret = map_.insert(std::make_pair(key, value));
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

private:
    map<KeyType, ValueType, Compare> map_;
};


NS_IZENELIB_AM_END

#endif /*_H_*/
