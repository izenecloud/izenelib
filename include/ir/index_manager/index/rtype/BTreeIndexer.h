
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
#include <util/ClockTimer.h>

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
#define BT_INFO
// #define DOCS_INFO
// #define CACHE_TIME_INFO

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
    
    typedef BTTermEnum<KeyType, CacheValueType> MemEnumType;
    typedef AMTermEnum<DbType> AMEnumType;
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
        ValueType compressed;
        bool b_db = getDbValue_(key, compressed);
        bool b_cache = cache_.exist(key);
        if(!b_db && !b_cache) return false;
        return true;
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

    std::size_t getValueBetween(const KeyType& lowKey, const KeyType& highKey, std::size_t maxDoc, KeyType* & data)
    {
        if( compare_(lowKey,highKey)>0 ) return 0;
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        std::size_t result = 0;

        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_(lowKey));
            std::pair<KeyType, ValueType> kvp;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,highKey)>0 ) break;
                izenelib::am::EWAHBoolArrayBitIterator<uint32_t> it = kvp.second.bit_iterator();
                while(it.next())
                {
                    data[it.getCurr()] = kvp.first;
                    ++result;
                }
            }
        }
        else
        {
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
        }

        return result;
    }

    void getValueBetween(const KeyType& key1, const KeyType& key2, BitVector& docs)
    {
        if( compare_(key1,key2)>0 ) return;
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_(key1));
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,key2)>0 ) break;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else
        {
        
            std::auto_ptr<BaseEnumType> term_enum(getEnum_(key1));
            std::pair<KeyType, BitVector> kvp;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,key2)>0 ) break;
                docs |= kvp.second;
            }
        }
    }

    void getValueLess(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_());
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,key)>=0 ) break;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }

    void getValueLessEqual(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_());
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,key)>0 ) break;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }

    void getValueGreat(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_(key));
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if( compare_(kvp.first,key)==0 ) continue;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }

    void getValueGreatEqual(const KeyType& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_(key));
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }


    void getValueStart(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_(key));
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if(!Compare<izenelib::util::UString>::start_with(kvp.first, key)) break;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }

    void getValueEnd(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_());
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if(!Compare<izenelib::util::UString>::end_with(kvp.first, key)) continue;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
    }

    void getValueSubString(const izenelib::util::UString& key, BitVector& docs)
    {
#ifdef DOCS_INFO
        std::cout<<"[start] "<<docs<<std::endl;
#endif
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        if(cache_.empty())
        {
            std::auto_ptr<AMEnumType> term_enum(getAMEnum_());
            std::pair<KeyType, ValueType> kvp;
            ValueType value;
            while(term_enum->next(kvp))
            {
                if(!Compare<izenelib::util::UString>::contains(kvp.first, key)) continue;
                ValueType new_value;
                value.rawlogicalor(kvp.second, new_value);
                value.swap(new_value);
            }
            decompress_(value, docs);
        }
        else {
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
        BaseEnumType* term_enum = new EnumType(cache_.getAM(), db_, lowKey, func_);
        return term_enum;
    }
    
    BaseEnumType* getEnum_()
    {
        BaseEnumType* term_enum = new EnumType(cache_.getAM(), db_, func_);
        return term_enum;
    }
    
    MemEnumType* getMemEnum_(const KeyType& lowKey)
    {
        MemEnumType* term_enum = new MemEnumType(cache_.getAM(), lowKey);
        return term_enum;
    }
    
    MemEnumType* getMemEnum_()
    {
        MemEnumType* term_enum = new MemEnumType(cache_.getAM());
        return term_enum;
    }
    
    AMEnumType* getAMEnum_(const KeyType& lowKey)
    {
        AMEnumType* term_enum = new AMEnumType(db_, lowKey);
        return term_enum;
    }
    
    AMEnumType* getAMEnum_()
    {
        AMEnumType* term_enum = new AMEnumType(db_);
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
        cache_num_ = 0;
        boost::function<void (const std::pair<KeyType, CacheValueType>&) > func=boost::bind( &ThisType::cacheIterator_, this, _1);
        cache_.iterate(func);
        cache_.clear();
    }
    
    void cacheIterator_(const std::pair<KeyType, CacheValueType>& kvp)
    {
#ifdef CACHE_TIME_INFO
        static double t1=0.0;
        static double t2=0.0;
        static double t3=0.0;
//         std::cout<<"start buffer size : "<<common_compressed_.bufferCapacity()<<std::endl;
        izenelib::util::ClockTimer timer;
#endif
        
        
//         ValueType value;
        common_compressed_.reset();
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator : "<<kvp.first<<","<<kvp.second<<std::endl;
#endif
        
#ifdef CACHE_TIME_INFO
//         std::cout<<"initialzed buffer size : "<<common_compressed_.bufferCapacity()<<std::endl;
        timer.restart();
#endif
        getDbValue_(kvp.first, common_compressed_);
#ifdef CACHE_TIME_INFO
        t1 += timer.elapsed();
//         std::cout<<"deserilized buffer size : "<<common_compressed_.bufferCapacity()<<std::endl;
#endif
        
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator before update : "<<kvp.first<<","<<value<<std::endl;
#endif
        
#ifdef CACHE_TIME_INFO
        timer.restart();
#endif
        applyCacheValue_(common_compressed_, kvp.second);
#ifdef CACHE_TIME_INFO
        t2 += timer.elapsed();
#endif
        
#ifdef CACHE_TIME_INFO
        timer.restart();
#endif
        

        
        
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator after update : "<<kvp.first<<","<<value<<std::endl;
#endif
#ifdef CACHE_TIME_INFO
        timer.restart();
#endif
        db_.update(kvp.first, common_compressed_);
#ifdef CACHE_TIME_INFO
        t3 += timer.elapsed();
//         std::cout<<"end buffer size : "<<common_compressed_.bufferCapacity()<<std::endl;
#endif
//         if(value.numberOfOnes()>0)
//         {
//             db_.update(kvp.first, value);
//         }
//         else
//         {
// //             db_.update(kvp.first, bitmap);
//             db_.del(kvp.first);
//         }
#ifdef CACHE_DEBUG
        std::cout<<"cacheIterator update finish"<<std::endl;
#endif
        ++cache_num_;
#ifdef BT_INFO
        if(cache_num_%10000 == 0)
        {
            std::cout<<"cacheIterator number : "<<cache_num_<<std::endl;
        }
#endif
#ifdef CACHE_TIME_INFO
        if(cache_num_%10000 == 0)
        {
            std::cout<<"cacheIterator time cost : "<<t1<<","<<t2<<","<<t3<<std::endl;
        }
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
    
//     bool getValue_(const KeyType& key, ValueType& bitmap)
//     {
//         bool b_db = getDbValue_(key, bitmap);
//         CacheValueType cache_value;
//         bool b_cache = getCacheValue_(key, cache_value);
//         if(!b_db && !b_cache) return false;
//         if(b_cache)
//         {
//             applyCacheValue_(bitmap, cache_value);
//         }
//         return true;
//     }
    
    bool getValue_(const KeyType& key, BitVector& value)
    {
        ValueType compressed;
        bool b_db = getDbValue_(key, compressed);
        CacheValueType cache_value;
        bool b_cache = getCacheValue_(key, cache_value);
        if(!b_db && !b_cache) return false;
        decompress_(compressed, value);
        if(b_cache)
        {
            applyCacheValue_(value, cache_value);
        }
        return true;
    }
    
    
    static void decompress_(const ValueType& compressed, BitVector& value)
    {
        izenelib::am::EWAHBoolArrayBitIterator<uint32_t> it = compressed.bit_iterator();
        while(it.next())
        {
            value.set(it.getCurr() );
        }
//         std::vector<uint32_t> out;
//         compressed.appendRowIDs(out);
//         for(uint32_t i=0;i<out.size();i++)
//         {
//             value.set(out[i]);
//         }
    }
    
    static void compress_(const BitVector& value, ValueType& compressed)
    {
        value.compressed(compressed);
    }
    
    
    /// Be called in range search function.
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
    
    /// called only in cacheIterator_, one thread, common_bv_ is safe.
    void applyCacheValue_(ValueType& value, const CacheValueType& cacheValue)
    {
#ifdef CACHE_TIME_INFO
        static double t21=0.0;
        static double t22=0.0;
        static double t23=0.0;
        izenelib::util::ClockTimer timer;
#endif
        common_bv_.clear();
        decompress_(value, common_bv_);
#ifdef CACHE_TIME_INFO
        t21 += timer.elapsed();
        timer.restart();
#endif
        
        
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after decompress_"<<docs<<std::endl;
#endif        
        applyCacheValue_(common_bv_, cacheValue);
#ifdef CACHE_TIME_INFO
        t22 += timer.elapsed();
        timer.restart();
#endif
        
        
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after applyCacheValue_ "<<docs<<std::endl;
#endif
        value.reset();
        compress_(common_bv_, value);
#ifdef CACHE_TIME_INFO
        t23 += timer.elapsed();
        timer.restart();
#endif
        
#ifdef CACHE_DEBUG
        std::cout<<"applyCacheValue_ after compress_"<<value<<std::endl;
#endif
#ifdef CACHE_TIME_INFO
        if(cache_num_%10000 == 0)
        {
            std::cout<<"applyCacheValue_ time cost : "<<t21<<","<<t22<<","<<t23<<std::endl;
        }
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
    /// used only in cacheIterator_, one thread,  safe.
    BitVector common_bv_;
    ValueType common_compressed_;
    ///
    std::size_t cache_num_;
    boost::shared_mutex mutex_;
};


}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/

