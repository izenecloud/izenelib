#ifndef IZENELIB_IR_INMEMORYBTREEINDEXER_H_
#define IZENELIB_IR_INMEMORYBTREEINDEXER_H_
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <3rdparty/am/stx/btree_map.h>

// #define BTCACHE_DEBUG

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

///@brief should be concurrency, one thread writing and multi-threads reading
template <class KeyType, class ValueItemType>
class InMemoryBTreeIndexer
{
public:
//     typedef boost::unordered_map<ValueItemType, bool> ValueType;

    typedef std::vector<ValueItemType> ValueType;

    typedef std::map<KeyType, ValueType, Compare<KeyType> > AMType;
    typedef typename AMType::iterator iterator;
    typedef typename AMType::const_iterator const_iterator;

    InMemoryBTreeIndexer()
    {
    }

    ~InMemoryBTreeIndexer()
    {
    }


    void add(const KeyType& key, const ValueItemType& value_item)
    {

        typename AMType::iterator it = data_.find(key);
        if(it==data_.end())
        {
            it = data_.insert(std::make_pair(key, ValueType())).first;
        }
        it->second.push_back(value_item);
//         update_value_(it->second, value_item, true);
    }

    void remove(const KeyType& key, const ValueItemType& value_item)
    {
        typename AMType::iterator it = data_.find(key);
        if(it!=data_.end())
        {
            VectorRemove_(it->second, value_item);
            if(it->second.empty())
            {
                data_.erase(it);
            }
        }
//         update_value_(it->second, value_item, false);
    }

    bool seek(const KeyType& key)
    {
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
        Bitset value;
        return getValue_(key, value);
    }


    void getValue(const KeyType& key, Bitset& docs)
    {
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
        getValue_(key, docs);
    }

//     void getValue(const KeyType& key, std::vector<docid_t>& docList)
//     {
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
//         Bitset docmap;
//         getValue(key, docmap);
//         for(std::size_t docid=1;docid<docmap.size();docid++)
//         {
//             if(docmap.test(docid))
//             {
//                 docList.push_back(docid);
//             }
//         }
//     }

//     std::size_t getValueBetween(const KeyType& lowKey, const KeyType& highKey, std::size_t maxDoc, KeyType* & data)
//     {
//         if( compare_(lowKey,highKey)>0 ) return 0;
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
//         std::size_t result = 0;
//         std::unique_ptr<BaseEnumType> term_enum(getEnum_(lowKey));
//         std::pair<KeyType, Bitset> kvp;
//         while(term_enum->next(kvp))
//         {
//             if( compare_(kvp.first,highKey)>0 ) break;
//             for(std::size_t docid=1;docid<kvp.second.size();docid++)
//             {
//                 if (docid >= maxDoc) break;
//                 if(kvp.second.test(docid))
//                 {
//                     data[docid] = kvp.first;
//                     ++result;
//                 }
//             }
//         }
//
//         return result;
//     }

    void getValueBetween(const KeyType& key1, const KeyType& key2, Bitset& docs)
    {
        if( compare_(key1,key2)>0 ) return;
//         boost::shared_lock<boost::shared_mutex> lock(mutex_);
        for(const_iterator it = data_.lower_bound(key1); it!=data_.end();++it)
        {
            if( compare_(it->first,key2)>0 ) break;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueLess(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.begin(); it!=data_.end();++it)
        {
            if( compare_(it->first,key)>=0 ) break;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueLessEqual(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.begin(); it!=data_.end();++it)
        {
            if( compare_(it->first,key)>0 ) break;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueGreat(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.upper_bound(key); it!=data_.end();++it)
        {
//             std::cout<<"in memory getValueGreat key : "<<it->first<<std::endl;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueGreatEqual(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.lower_bound(key); it!=data_.end();++it)
        {
//             std::cout<<"in memory getValueGreatEqual key : "<<it->first<<std::endl;
            valueToBitset_(it->second, docs);
        }
    }


    void getValueStart(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.lower_bound(key); it!=data_.end();++it)
        {
            if(!Compare<KeyType>::start_with(it->first, key)) break;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueEnd(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.begin(); it!=data_.end();++it)
        {
            if(!Compare<KeyType>::end_with(it->first, key)) continue;
            valueToBitset_(it->second, docs);
        }
    }

    void getValueSubString(const KeyType& key, Bitset& docs)
    {
        for(const_iterator it = data_.begin(); it!=data_.end();++it)
        {
            if(!Compare<KeyType>::contains(it->first, key)) continue;
            valueToBitset_(it->second, docs);
        }
    }


private:
    template<typename T>
    static int compare_(const T& t1, const T& t2)
    {
        return Compare<T>::compare(t1, t2);
    }

    template <typename T>
    static void VectorRemove_(std::vector<T>& vec, const T& value)
    {
        vec.erase( std::remove(vec.begin(), vec.end(), value),vec.end());
    }

    bool getValue_(const KeyType& key, Bitset& docs)
    {
        const_iterator it = data_.find(key);
        if(it == data_.end()) return false;
        const ValueType& value = it->second;
        valueToBitset_(value, docs);
        return true;
    }

    void valueToBitset_(const ValueType& value, Bitset& docs)
    {
        for(uint32_t i=0;i<value.size();i++)
        {
            docs.set(value[i]);
        }
    }

//     void update_value_(ValueType& map, const ValueItemType& item, bool v)
//     {
//         typename ValueType::iterator it = map.find(item);
//         if(it==map.end())
//         {
//             map.insert(std::make_pair(item, v));
//         }
//         else
//         {
//             it->second = v;
//         }
//     }
private:
    AMType data_;
//     boost::shared_mutex mutex_;
};

}

NS_IZENELIB_IR_END

#endif
