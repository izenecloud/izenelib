
#ifndef IZENELIB_IR_BTREEINDEXER_H
#define IZENELIB_IR_BTREEINDEXER_H

#include <util/BoostVariantUtil.h>
#include <am/bitmap/Ewah.h>
#include <am/luxio/BTree.h>
#include <am/range/AmIterator.h>
#include <ir/index_manager/index/rtype/InMemoryBTreeCache.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/store/Directory.h>

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/bind.hpp>
#include <functional>

#include <algorithm>
#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{



/**
 * BTreeIndexer
 * @brief BTreeIndexer is a wrapper which takes charges of managing all b-tree handlers.
 */

template <class KeyType>
class CBTreeIndexer
{
    typedef CBTreeIndexer<KeyType> ThisType;
    typedef izenelib::am::EWAHBoolArray<uint32_t> ValueType;
    typedef izenelib::am::luxio::BTree<KeyType, ValueType> DbType;
    typedef InMemoryBTreeCache<KeyType, docid_t> CacheType;
    typedef typename CacheType::ValueType CacheValueType;
    
    typedef izenelib::am::AMIterator<DbType> ForwardIterator;
    typedef izenelib::am::AMReverseIterator<DbType> ReverseIterator;
public:
    CBTreeIndexer(const std::string& path, const std::string& property_name, std::size_t cacheSize = 12500000)//50M for uint32_t
    :path_(path), property_name_(property_name)
    {
        cache_.set_max_capacity(cacheSize);
    }

    ~CBTreeIndexer()
    {
    }
public:
    
    bool open()
    {
        return db_.open(path_);
    }
    
    bool seek(const KeyType& key)
    {
        ValueType value;
        return getValueImpl_(key, value);
    }

    void getNoneEmptyList(const KeyType& key, BitVector& docs)
    {
        getValueImpl_(key, docs);
    }

    void add(const KeyType& key, docid_t docid, bool isUpdate = false)
    {
        cache_.add(key, docid, isUpdate);
        if(cache_.is_full())
        {
            cacheFull_();
        }
    }

    void remove(const KeyType& key, docid_t docid)
    {
        cache_.remove(key, docid);
        if(cache_.is_full())
        {
            cacheFull_();
        }
    }

    void getValue(const KeyType& key, BitVector& docs)
    {
        getValueImpl_(key, docs);
    }

    void getValue(const KeyType& key, std::vector<docid_t>& docList)
    {
        BitVector docmap;
        getValue(key, docmap);
        for(std::size_t docid=1;docid<docmap.size();docid++)
        {
            if(docmap.test(docid))
            {
                docList.push_back(docid);
            }
        }
    }
    
    std::size_t getValueBetween(const KeyType& lowKey, const KeyType& highKey, KeyType* & data, std::size_t maxDoc )
    {
//         BitVector docmap;
//         getValueBetween(lowKey, highKey, docmap);
//         std::size_t count = 0;
//         for(std::size_t docid=1;docid<docmap.size();docid++)
//         {
//             if (docid >= maxDoc)
//                 break;
//             if(docmap.test(docid))
//             {
//                 data
//                 docList.push_back(docid);
//             }
//         }
        return 0;
    }

    void getValueBetween(const KeyType& key1, const KeyType& key2, BitVector& docs)
    {
    }

    void getValueLess(const KeyType& key, BitVector& docs)
    {
        
        
    }

    void getValueLessEqual(const KeyType& key, BitVector& docs)
    {
        ReverseIterator it(db_, key);
        ReverseIterator it_end;
        for(;it!=it_end;++it)
        {
            const KeyType& key = it->first;
            ValueType& value = it->second;
            getValueImpl3_(key, value);
            decompress_(value, docs);
        }
    }

    void getValueGreat(const KeyType& key, BitVector& docs)
    {
    }

    void getValueGreatEqual(const KeyType& key, BitVector& docs)
    {
    }

    void getValueIn(const std::vector<KeyType>& keys, BitVector& docs)
    {
    }

    void getValueNotIn(const std::vector<KeyType>& keys, BitVector& docs)
    {
    }

    void getValueNotEqual(const KeyType& key, BitVector& docs)
    {
    }

    void getValueStart(const KeyType& key, BitVector& docs)
    {
    }

    void getValueEnd(const KeyType& key, BitVector& docs)
    {
    }

    void getValueSubString(const KeyType& key, BitVector& docs)
    {
    }

    void flush()
    {
        cacheFull_();
        db_.flush();
    }

    


private:
    
    void cacheFull_()
    {
        std::cout<<"!!!cacheFull_ "<<property_name_<<std::endl;
        boost::function<void (const std::pair<KeyType, CacheValueType>&) > func=boost::bind( &ThisType::cacheIterator_, this, _1);
        cache_.clear(func);
    }
    
    void cacheIterator_(const std::pair<KeyType, CacheValueType>& kvp)
    {
        ValueType bitmap;
        getValueImpl2_(kvp.first, kvp.second, bitmap);
        db_.update(kvp.first, bitmap);
    }
    
    bool getDbValueImpl_(const KeyType& key, ValueType& value)
    {
        return db_.get(key, value);
    }
    
    bool getCacheValueImpl_(const KeyType& key, CacheValueType& value)
    {
        return cache_.get(key, value);
    }
    
    bool getValueImpl_(const KeyType& key, ValueType& bitmap)
    {
        bool b_db = getDbValueImpl_(key, bitmap);
        CacheValueType cache_value;
        bool b_cache = getCacheValueImpl_(key, cache_value);
        if(!b_db && !b_cache) return false;
        if(b_cache)
        {
            setCacheValue_(bitmap, cache_value);
        }
        return true;
    }
    
    bool getValueImpl_(const KeyType& key, BitVector& value)
    {
        ValueType compressed;
        if(!getValueImpl_(key, compressed)) return false;
        decompress_(compressed, value);
        return true;
    }
    
    void getValueImpl2_(const KeyType& key, const CacheValueType& cacheValue, ValueType& value)
    {
        getDbValueImpl_(key, value);
        setCacheValue_(value, cacheValue);
    }
    
    void getValueImpl3_(const KeyType& key, ValueType& dbValue)
    {
        CacheValueType cacheValue;
        if(getCacheValueImpl_(key, cacheValue))
        {
            setCacheValue_(dbValue, cacheValue);
        }
    }
    
    static void decompress_(const ValueType& compressed, BitVector& value)
    {
        //TODO optimize?
        izenelib::am::EWAHBoolArrayBitIterator<uint32_t> it = compressed.bit_iterator();
        while(it.next())
        {
            value.set(it.getCurr() );
        }
    }
    
    static void compress_(const BitVector& value, ValueType& compressed)
    {
        value.compressed(compressed);
    }
    
    static void setCacheValue_(ValueType& bitmap, const CacheValueType& cacheValue)
    {
        bool need_decompress = false;
        if(cacheValue.delete_item.size()>0 || cacheValue.update_item.size()>0 )
        {
            need_decompress = true;
        }
        
        if(!need_decompress)
        {
            for(uint32_t i=0;i<cacheValue.insert_item.size();i++)
            {
                bitmap.add(cacheValue.insert_item[i]);
            }
        }
        else
        {
            BitVector value;
            decompress_(bitmap, value);
            for(uint32_t i=0;i<cacheValue.insert_item.size();i++)
            {
                value.set(cacheValue.insert_item[i]);
            }
            for(uint32_t i=0;i<cacheValue.update_item.size();i++)
            {
                value.set(cacheValue.update_item[i]);
            }
            for(uint32_t i=0;i<cacheValue.delete_item.size();i++)
            {
                value.clear(cacheValue.delete_item[i]);
            }
            bitmap.reset();
            compress_(value, bitmap);
        }
    }
    
private:
    std::string path_;
    DbType db_;
    CacheType cache_;
    

};


}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/
