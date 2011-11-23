
#ifndef IZENELIB_IR_BTREEINDEXER_H_
#define IZENELIB_IR_BTREEINDEXER_H_

#include <util/BoostVariantUtil.h>
#include <am/bitmap/Ewah.h>
#include <am/luxio/BTree.h>
#include <am/tc/BTree.h>
#include <am/leveldb/Table.h>
#include <am/range/AmIterator.h>
#include <ir/index_manager/index/rtype/InMemoryBTreeCache.h>
#include <ir/index_manager/index/rtype/TermEnum.h>
#include <ir/index_manager/index/rtype/Compare.h>
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

// #define CACHE_DEBUG
// #define BT_DEBUG
// #define BT_INFO
// #define DOCS_INFO

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
//     typedef izenelib::am::luxio::BTree<KeyType, ValueType, Compare<KeyType> > DbType;
//     typedef izenelib::am::tc::BTree<KeyType, ValueType> DbType;
    typedef izenelib::am::leveldb::Table<KeyType, ValueType> DbType;
    typedef InMemoryBTreeCache<KeyType, docid_t> CacheType;
    typedef typename CacheType::ValueType CacheValueType;
    
    typedef izenelib::am::AMIterator<DbType> ForwardIterator;
    typedef izenelib::am::AMReverseIterator<DbType> ReverseIterator;
    
//     typedef BTTermEnum<KeyType, CacheValueType> EnumType1;
//     typedef AMTermEnum<DbType> EnumType2;
    typedef TwoWayTermEnum<KeyType, CacheValueType, DbType, BitVector> EnumType;
    typedef TermEnum<KeyType, BitVector> BaseEnumType;
    typedef boost::function<void (const CacheValueType&,const ValueType&, BitVector&) > EnumCombineFunc;
    
public:
    CBTreeIndexer(const std::string& path, const std::string& property_name, std::size_t cacheSize = 2000000)//an experienced value
    :path_(path), property_name_(property_name)
    {
        cache_.set_max_capacity(cacheSize);
        func_ = &combineValue_;
    }

    ~CBTreeIndexer()
    {
    }
    
    bool open()
    {
        return db_.open(path_);
//         return db_.open(path_, DbType::WRITER | DbType::CREAT | DbType::NOLCK);
    }
    
    void close()
    {
        db_.close();
    }
    
    void add(const KeyType& key, docid_t docid)
    {
        boost::lock_guard<boost::shared_mutex> lock(mutex_);
        cache_.add(key, docid);
        checkCache_();
        
    }

    void remove(const KeyType& key, docid_t docid)
    {
        boost::lock_guard<boost::shared_mutex> lock(mutex_);
        cache_.remove(key, docid);
        checkCache_();
        
    }
    
    bool seek(const KeyType& key)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        ValueType value;
        return getValue_(key, value);
    }

    void getNoneEmptyList(const KeyType& key, BitVector& docs)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        getValue_(key, docs);
    }

    void getValue(const KeyType& key, BitVector& docs)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        getValue_(key, docs);
    }

    void getValue(const KeyType& key, std::vector<docid_t>& docList)
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
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
    
    std::size_t getValueBetween(const KeyType& lowKey, const KeyType& highKey, std::size_t maxDoc, KeyType* & data)
    {
        if( compare_(lowKey,highKey)>0 ) return 0;
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::size_t result = 0;
        std::auto_ptr<BaseEnumType> term_enum(getEnum_(lowKey));
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if( compare_(kvp.first,highKey)>0 ) break;
            for(std::size_t docid=1;docid<kvp.second.size();docid++)
            {
                if (docid >= maxDoc) break;
                if(kvp.second.test(docid))
                {
                    data[docid] = kvp.first;
                    ++result;
                }
            }
        }
        
        return result;
    }

    void getValueBetween(const KeyType& key1, const KeyType& key2, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        if( compare_(key1,key2)>0 ) return;
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::auto_ptr<BaseEnumType> term_enum(getEnum_(key1));
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if( compare_(kvp.first,key2)>0 ) break;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void getValueLess(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::auto_ptr<BaseEnumType> term_enum(getEnum_());
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if( compare_(kvp.first,key)>=0 ) break;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
        
    }

    void getValueLessEqual(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::auto_ptr<BaseEnumType> term_enum(getEnum_());
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if( compare_(kvp.first,key)>0 ) break;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void getValueGreat(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::auto_ptr<BaseEnumType> term_enum(getEnum_(key));
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if( compare_(kvp.first,key)==0 ) continue;
//             std::cout<<"getValueGreat key : "<<kvp.first<<std::endl;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void getValueGreatEqual(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::auto_ptr<BaseEnumType> term_enum(getEnum_(key));
        std::pair<KeyType, BitVector> kvp;
        while(term_enum->next(kvp))
        {
//             std::cout<<"getValueGreatEqual key : "<<kvp.first<<std::endl;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }


    void getValueStart(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        std::auto_ptr<BaseEnumType> term_enum(getEnum_(key));
        std::pair<izenelib::util::UString, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if(!Compare<izenelib::util::UString>::start_with(kvp.first, key)) break;
//             if(kvp.first.find(key)!=0) break;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void getValueEnd(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        std::auto_ptr<BaseEnumType> term_enum(getEnum_());
        std::pair<izenelib::util::UString, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if(!Compare<izenelib::util::UString>::end_with(kvp.first, key)) continue;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void getValueSubString(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        std::auto_ptr<BaseEnumType> term_enum(getEnum_());
        std::pair<izenelib::util::UString, BitVector> kvp;
        while(term_enum->next(kvp))
        {
            if(!Compare<izenelib::util::UString>::contains(kvp.first, key)) continue;
            docs |= kvp.second;
#ifdef DOCS_INFO
            std::cout<<"[this] "<<kvp.second<<std::endl;
            std::cout<<"[after "<<kvp.first<<"] "<<docs<<std::endl;
#endif
        }
    }

    void flush()
    {
        boost::lock_guard<boost::shared_mutex> lock(mutex_);
        cacheClear_();
        db_.flush();
    }

    


private:
    
    template<typename T>
    static int compare_(const T& t1, const T& t2)
    {
        return Compare<T>::compare(t1, t2);
    }
    
    
    BaseEnumType* getEnum_(const KeyType& lowKey)
    {
//         EnumType1 term_enum1(cache_.getAM(), lowKey);
//         EnumType2 term_enum2(db_, lowKey);
        BaseEnumType* term_enum = new EnumType(cache_.getAM(), db_, lowKey, func_);
        return term_enum;
    }
    
    BaseEnumType* getEnum_()
    {
//         EnumType1 term_enum1(cache_.getAM());
//         EnumType2 term_enum2(db_);
        BaseEnumType* term_enum = new EnumType(cache_.getAM(), db_, func_);
        return term_enum;
    }
    
    void checkCache_()
    {
#ifdef BT_INFO
        if(cache_.capacity()%500000==0)
        {
            std::cout<<"!!!cache status : "<<property_name_<<", size: "<<cache_.capacity()<<std::endl;
        }
#endif
        if(cache_.is_full())
        {
            cacheClear_();
        }
    }
    
    void cacheClear_()
    {
#ifdef BT_INFO
        std::cout<<"!!!cacheClear_ "<<property_name_<<", key size: "<<cache_.key_size()<<std::endl;
#endif
        boost::function<void (const std::pair<KeyType, CacheValueType>&) > func=boost::bind( &ThisType::cacheIterator_, this, _1);
        cache_.iterate(func);
        cache_.clear();
    }
    
    void cacheIterator_(const std::pair<KeyType, CacheValueType>& kvp)
    {
        ValueType value;
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator : "<<kvp.first<<","<<kvp.second<<std::endl;
#endif
        getDbValue_(kvp.first, value);
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator before update : "<<kvp.first<<","<<value<<std::endl;
#endif
        applyCacheValue_(value, kvp.second);
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator after update : "<<kvp.first<<","<<value<<std::endl;
#endif
        if(value.numberOfOnes()>0)
        {
            db_.update(kvp.first, value);
        }
        else
        {
//             db_.update(kvp.first, bitmap);
            db_.del(kvp.first);
        }
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator update finish"<<std::endl;
#endif
    }
    
    bool getDbValue_(const KeyType& key, ValueType& value)
    {
        return db_.get(key, value);
    }
    
    bool getCacheValue_(const KeyType& key, CacheValueType& value)
    {
        return cache_.get(key, value);
    }
    
    bool getValue_(const KeyType& key, ValueType& bitmap)
    {
        bool b_db = getDbValue_(key, bitmap);
        CacheValueType cache_value;
        bool b_cache = getCacheValue_(key, cache_value);
        if(!b_db && !b_cache) return false;
        if(b_cache)
        {
            applyCacheValue_(bitmap, cache_value);
        }
        return true;
    }
    
    bool getValue_(const KeyType& key, BitVector& value)
    {
        ValueType compressed;
        if(!getValue_(key, compressed)) return false;
        decompress_(compressed, value);
        return true;
    }
    
    
    /// the dbValue already got before
//     void getValue2_(const KeyType& key, ValueType& dbValue)
//     {
//         CacheValueType cacheValue;
//         if(getCacheValue_(key, cacheValue))
//         {
//             setCacheValue_(dbValue, cacheValue);
//         }
//     }
    
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
    
    
    
    static void combineValue_(const CacheValueType& cacheValue, const ValueType& cbitmap, BitVector& result)
    {
#ifdef BT_DEBUG
        std::cout<<"before decompress"<<std::endl;
        std::cout<<result<<std::endl;
        std::cout<<cbitmap<<std::endl;
#endif
        decompress_(cbitmap, result);
        
#ifdef BT_DEBUG
        std::cout<<"after decompress"<<std::endl;
        std::cout<<result<<std::endl;
        std::cout<<cacheValue<<std::endl;
#endif
        applyCacheValue_(result, cacheValue);

#ifdef BT_DEBUG
        std::cout<<"after combine"<<std::endl;
        std::cout<<result<<std::endl;
#endif
    }
    
    static void applyCacheValue_(ValueType& value, const CacheValueType& cacheValue)
    {
        BitVector docs;
        decompress_(value, docs);
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after decompress_"<<docs<<std::endl;
#endif        
        applyCacheValue_(docs, cacheValue);
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after applyCacheValue_ "<<docs<<std::endl;
#endif
        value.reset();
        compress_(docs, value);
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after compress_"<<value<<std::endl;
#endif        
    }
    
    static void applyCacheValue_(BitVector& value, const CacheValueType& cacheValue)
    {
        for(std::size_t i=0;i<cacheValue.item.size();i++)
        {
            if(cacheValue.flag.test(i))
            {
                value.set(cacheValue.item[i]);
            }
            else
            {
                value.clear(cacheValue.item[i]);
            }
        }
        
//         std::cout<<"[applyCacheValue_] {insert} ";
//         for(std::size_t i=0;i<cacheValue.item.size();i++)
//         {
//             if(cacheValue.flag.test(i))
//             {
//                 std::cout<<cacheValue.item[i]<<",";
//             }
//         }
//         std::cout<<" {delete} ";
//         for(std::size_t i=0;i<cacheValue.item.size();i++)
//         {
//             if(!cacheValue.flag.test(i))
//             {
//                 std::cout<<cacheValue.item[i]<<",";
//             }
//         }
//         std::cout<<std::endl;
    }
    
private:
    std::string path_;
    std::string property_name_;
    DbType db_;
    CacheType cache_;
    EnumCombineFunc func_;
    boost::shared_mutex mutex_;
};


}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/

