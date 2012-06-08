///
/// @file   SeqFileDef.hpp
/// @brief  
/// @author Jia Guo
/// @date   Created 2010-03-22
/// @date   Updated 2010-03-22
///
#ifndef SEQFILEDEF_HPP_
#define SEQFILEDEF_HPP_

#include <string>
#include <algorithm>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/thread/locks.hpp> 
#include <boost/format.hpp>

#include <am/dyn_array.hpp>
#include <am/tokyo_cabinet/tc_fixdb.h>
#include <cache/IzeneCache.h>
#include <util/izene_serialization.h>
#include <util/Exception.h>
#include <util/bzip.h>

NS_IZENELIB_AM_BEGIN


template <typename T>
class CommonSeqFileSerializeHandler
{
    public:
    static char* serialize(const T& data, std::size_t& vsize)
    {
        char* ptr;
        izenelib::util::izene_serialization<T> izs(data);
        izs.write_image(ptr, vsize);
        char* result = NULL;
        if( vsize>0 )
        {
            result = (char*)malloc(vsize);
            memcpy(result, ptr, vsize);
        }
        return result;
    }
    
    static void deserialize(char*& ptr, std::size_t vsize, T& data)
    {
        izenelib::util::izene_deserialization<T> izd_value(ptr, vsize);
        izd_value.read_image(data);
    }
};

template <typename T>
class CompressSeqFileSerializeHandler
{
    public:
    static char* serialize(const T& data, std::size_t& vsize)
    {
        char* ptr;
        izenelib::util::izene_serialization<T> izs(data);
        std::size_t ivsize;
        izs.write_image(ptr, ivsize);
        int sp;
        char* result = _tc_bzcompress(ptr, (int)ivsize, &sp);
        vsize = (std::size_t) sp;
        return result;
    }
    
    static void deserialize(char* ptr, std::size_t vsize, T& data)
    {
        int sp;
        char* result = _tc_bzdecompress(ptr, (int)vsize, &sp);
        izenelib::util::izene_deserialization<T> izd_value(result, (std::size_t)sp);
        izd_value.read_image(data);
        free(result);
    }
};

template <>
class CompressSeqFileSerializeHandler<std::vector<uint32_t> >
{
    public:
    static char* serialize(const std::vector<uint32_t>& data, std::size_t& len)
    {
        if (data.size()==0)
        {
            len=0;
            return NULL;
        }
        
        
        uint32_t count = data.size();
        uint16_t max = 65535;
        if(count >= max/sizeof(uint32_t))
        {
            count = max/sizeof(uint32_t);
        }
        izenelib::am::DynArray<uint32_t> ar;
        for(uint32_t i=0;i<count;i++)
            ar.push_back(data[i]);
        ar.sort();
        uint32_t last = ar.at(0);
        //std::cout<<"[ID]: "<<ar.at(0)<<", ";
        for(uint32_t i=1;i<ar.length();i++)
        {
            //std::cout<<ar.at(i)<<", ";
            ar[i] = ar.at(i)-last;
            last += ar.at(i);
        }
        //std::cout<<std::endl;
        
        char* result = (char*) malloc(2*count*sizeof(uint32_t));
        len = 0;
        
        int32_t ui = 0;
        for(uint32_t i=0;i<ar.length();i++)
        {
            ui = ar.at(i);
            while ((ui & ~0x7F) != 0)
            {
                *(result + len) = ((uint8_t)((ui & 0x7f) | 0x80));
                ui >>= 7;
                ++len;
            }
            *(result + len) = (uint8_t)ui;
            ++len;
        }
        return result;
    }
    
    static void deserialize(char* data, std::size_t len, std::vector<uint32_t>& vdata)
    {
        if (len ==0)
            return;
    
        izenelib::am::DynArray<uint32_t> ar;
        for (uint16_t p = 0; p<len;)
        { 
            uint8_t b = *(data+p);
            ++p;
            
            int32_t i = b & 0x7F;
            for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
            {
                b = *(data+p);
                ++p;
                i |= (b & 0x7FL) << shift;
            }
            
            ar.push_back(i);
        }
        
        vdata.push_back(ar.at(0));
        
        for (uint32_t i =1; i<ar.length(); ++i)
        {
            uint32_t id = ar.at(i)+vdata[i-1];
            vdata.push_back(id);
        }
    }
};

template <typename T, template <class SerialType> class SerialHandler >
class SeqFileObjectCacheHandler
{
    public:
    SeqFileObjectCacheHandler():cache_(0), bucketSize_(10000), cacheSize_(0), maxCacheId_(0)
    {
    }
    ~SeqFileObjectCacheHandler()
    {
    }
    
    bool insertToCache(std::size_t id, const T& data)
    {
        if( id == 0 ) return false;
        if( id > cacheSize_ ) return false;
        if( id > cache_.size() )
        {
            uint32_t need = id - cache_.size();
            uint32_t bucketNum = ( need )/bucketSize_;
            if( need%bucketSize_ != 0 )
            {
                ++bucketNum;
            }
            boost::lock_guard<boost::shared_mutex> mLock(mutex_);
            cache_.resize( cache_.size()+bucketNum*bucketSize_ );
            
        }
        cache_[id-1] = data;
        if( id > maxCacheId_ )
        {
            maxCacheId_ = id;
        }
        return true;
    }
    
    bool getInCache(std::size_t id, T& data)
    {
        if( id == 0 ) return false;
        if( id > maxCacheId_ ) return false;
        boost::shared_lock<boost::shared_mutex> mLock(mutex_);
        data = cache_[id-1];
        return true;
    }
    
    void setCacheSize(std::size_t cacheSize)
    {
        cacheSize_ = cacheSize;
    }
    
    std::size_t getCacheSize() const
    {
        return cacheSize_;
    }

    
    protected:
        void initCache_(std::size_t size)
        {
            uint32_t bucketNum = size/bucketSize_;
            if( size%bucketSize_ != 0 )
            {
                ++bucketNum;
            }
            if( size == 0 ) bucketNum = 1;
            std::size_t space = bucketNum*bucketSize_;
            std::size_t fspace = space<=cacheSize_?space:cacheSize_;
            cache_.resize( fspace );
            
        }
        
        void initCache_()
        {
            initCache_(bucketSize_);
            
        }
        
    
    private:
        std::vector<T> cache_;
        std::size_t bucketSize_;
        std::size_t cacheSize_;
        std::size_t maxCacheId_;
        boost::shared_mutex mutex_;
};

template <typename T = uint32_t>
class CharCacheItem
{
public:
    CharCacheItem():data_(NULL), len_(0)
    {
    }
    CharCacheItem(char* data, T len):data_(NULL), len_(len)
    {
        if( len_ >0 )
        {
            data_ = (char*) malloc(len_);
            memcpy(data_, data, len_);
        }
    }
    CharCacheItem(const CharCacheItem<T>& rhs):data_(NULL), len_(rhs.len_)
    {
        if( len_ >0 )
        {
            data_ = (char*) malloc(len_);
            memcpy(data_, rhs.data_, len_);
        }
    }
    CharCacheItem<T>& operator=(const CharCacheItem<T>& rhs)
    {
        if( rhs.data_ == data_ ) return *this;
        if( data_ != NULL && len_>0 )
        {
            free(data_);
            data_ = NULL;
        }
        len_ = rhs.len_;
        if( len_ >0 )
        {
            data_ = (char*) malloc(len_);
            memcpy(data_, rhs.data_, len_);
        }
        return *this;
    }
    ~CharCacheItem()
    {
        if( data_ != NULL )
        {
            free(data_);
        }
    }
public:
    char* data_;
    T len_;
};


template <typename T, template <class SerialType> class SerialHandler>
class SeqFileCharCacheHandler
{
    typedef SerialHandler<T> SerType;
    typedef CharCacheItem<uint32_t> ValueType;
    public:
    SeqFileCharCacheHandler():cache_(0), bucketSize_(10000), cacheSize_(0), maxCacheId_(0)
    {
    }
    ~SeqFileCharCacheHandler()
    {

    }
    
    bool insertToCache(std::size_t id, const T& data)
    {
        if( id == 0 ) return false;
        if( id > cacheSize_ ) return false;
        if( id > cache_.size() )
        {
            uint32_t need = id - cache_.size();
            uint32_t bucketNum = ( need )/bucketSize_;
            if( need%bucketSize_ != 0 )
            {
                ++bucketNum;
            }
            boost::unique_lock<boost::shared_mutex> mLock(mutex_);
            cache_.resize( cache_.size()+bucketNum*bucketSize_ );
            
        }

        std::size_t vsize;
        char* ptr = SerType::serialize(data, vsize);
        if( vsize>0 )
        {
            CharCacheItem<uint32_t> item( ptr, (uint32_t)vsize);
            cache_[id-1] = item;
            free(ptr);
        }
        
        if( id > maxCacheId_ )
        {
            maxCacheId_ = id;
        }
        return true;
    }
    
    bool getInCache(std::size_t id, T& data)
    {
        if( id == 0 ) return false;
        if( id > maxCacheId_ ) return false;
        char* ptr = NULL;
        uint32_t len = 0;
        {
            boost::shared_lock<boost::shared_mutex> mLock(mutex_);
            ptr = cache_[id-1].data_;
            len = cache_[id-1].len_;
        }
        if( ptr == NULL || len==0 )
        {
            return false;
        }
        SerType::deserialize(ptr, len, data);
        return true;
    }
    
    void setCacheSize(std::size_t cacheSize)
    {
        cacheSize_ = cacheSize;
    }

    
    protected:
        protected:
        void initCache_(std::size_t size)
        {
            uint32_t bucketNum = size/bucketSize_;
            if( size%bucketSize_ != 0 )
            {
                ++bucketNum;
            }
            if( size == 0 ) bucketNum = 1;
            std::size_t space = bucketNum*bucketSize_;
            std::size_t fspace = space<=cacheSize_?space:cacheSize_;
            ValueType defaultValue(NULL, 0);
            cache_.resize( fspace,defaultValue  );
            
        }
        
        void initCache_()
        {
            initCache_(bucketSize_);
            
        }
        
    
    private:
        std::vector<ValueType > cache_;
        std::size_t bucketSize_;
        std::size_t cacheSize_;
        std::size_t maxCacheId_;
        boost::shared_mutex mutex_;
    
};

// template < template <class SerialType> class SerialHandler>
// class SeqFileCharCacheHandler<izenelib::util::UString, SerialHandler>
// {
//     typedef SerialHandler<izenelib::util::UString> SerType;
//     typedef std::pair<char*, uint32_t> ValueType;
//     public:
//     SeqFileCharCacheHandler():cache_(0), bucketSize_(10000), cacheSize_(0), maxCacheId_(0)
//     {
//     }
//     ~SeqFileCharCacheHandler()
//     {
//         for(uint32_t i=0;i<cache_.size();i++)
//         {
//             if( cache_[i].first!=NULL )
//             {
//                 free(cache_[i].first);
//             }
//         }
//     }
//     
//     bool insertToCache(std::size_t id, const izenelib::util::UString& data)
//     {
//         if( id == 0 ) return false;
//         if( id > cacheSize_ ) return false;
//         if( id > cache_.capacity() )
//         {
//             uint32_t need = id - cache_.capacity();
//             uint32_t bucketNum = ( need )/bucketSize_;
//             if( need%bucketSize_ != 0 )
//             {
//                 ++bucketNum;
//             }
//             ValueType defaultValue(NULL, 0);
//             cache_.resize( cache_.capacity()+bucketNum*bucketSize_, defaultValue );
//             
//         }
//         else
//         {
//             if (cache_[id-1].second > 0 )
//             {
//                 free(cache_[id-1].first);
//                 cache_[id-1].first = NULL;
//                 cache_[id-1].second = 0;
//             }
//         }
//         if( data.length() == 0 ) return false;
//         const char* inner = (const char*)data.c_str();
//         char* ptr = (char*) malloc(data.size());
//         memcpy( ptr, inner, data.size() );
//         cache_[id-1].first = ptr;
//         cache_[id-1].second = (uint32_t)data.length();
//         if( id > maxCacheId_ )
//         {
//             maxCacheId_ = id;
//         }
//         return true;
//     }
//     
//     bool getInCache(std::size_t id, izenelib::util::UString& data)
//     {
//         if( id == 0 ) return false;
//         if( id > maxCacheId_ ) return false;
//         if( cache_[id-1].first == NULL || cache_[id-1].second==0 )
//         {
//             return false;
//         }
//         data = izenelib::util::UString( (uint16_t*)cache_[id-1].first, cache_[id-1].second);
//         return true;
//     }
//     
//     void setCacheSize(std::size_t cacheSize)
//     {
//         cacheSize_ = cacheSize;
//     }
// 
//     
//     protected:
//         protected:
//         void initCache_(std::size_t size)
//         {
//             uint32_t bucketNum = size/bucketSize_;
//             if( size%bucketSize_ != 0 )
//             {
//                 ++bucketNum;
//             }
//             if( size == 0 ) bucketNum = 1;
//             ValueType defaultValue(NULL, 0);
//             cache_.resize( bucketNum*bucketSize_,defaultValue  );
//             
//         }
//         
//         void initCache_()
//         {
//             initCache_(bucketSize_);
//             
//         }
//         
//     
//     private:
//         std::vector<ValueType > cache_;
//         std::size_t bucketSize_;
//         std::size_t cacheSize_;
//         std::size_t maxCacheId_;
//     
// };

NS_IZENELIB_AM_END
#endif
