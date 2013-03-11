///
/// @file   SequenceFile.hpp
/// @brief
/// @author Jia Guo
/// @date   Created 2009-11-03
/// @date   Updated 2009-11-30
///
#ifndef SEQUENCEFILE_HPP_
#define SEQUENCEFILE_HPP_

#include <string>
#include <algorithm>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
//#include <boost/serialization/deque.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
//#include <boost/format.hpp>

#include <am/tokyo_cabinet/tc_fixdb.h>
//#include <cache/IzeneCache.h>
//#include <util/izene_serialization.h>
//#include <util/Exception.h>

#include "SeqFileDef.hpp"

NS_IZENELIB_AM_BEGIN


template <typename ValueType
, class Container = izenelib::am::tc_fixdb<ValueType >
, template <typename T> class SerializeHandler = CommonSeqFileSerializeHandler
, template <typename U, template <typename VT> class V> class CacheHandler = SeqFileObjectCacheHandler>
class SequenceFile : public CacheHandler<ValueType, SerializeHandler>
{
typedef CacheHandler<ValueType, SerializeHandler> CacheHandlerType;
typedef std::vector<ValueType> CacheType;
typedef typename CacheType::size_type KeyType;
typedef SerializeHandler<ValueType> SerializeType;
public:
typedef Container ContainerType;

SequenceFile(const std::string& file) :
file_(file), isOpen_(false), fileData_(file)
{

}

~SequenceFile()
{
    close();
}

Container& getContainer()
{
    return fileData_;
}

void open()
{
    if( isOpen() ) return;
    fileData_.open();
    CacheHandlerType::initCache_(fileData_.numItems());
    ValueType value;
    for(uint32_t i=0;i<fileData_.numItems();i++)
    {
        bool b = getOnFile(i+1, value);
        if(b) this->insertToCache(i+1, value);
        else break;
    }
    isOpen_ = true;

}

bool isOpen() const
{
    return isOpen_;
}



KeyType getItemCount() const
{
    return fileData_.numItems();
}


void update(KeyType id, const ValueType& value)
{
    if( !isOpen() ) return;
    std::size_t vsize;
    char* data = SerializeType::serialize(value, vsize);
    if( vsize>0 )
    {
        boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
        fileData_.update(id, data, vsize);
        this->insertToCache(id, value);
        free(data);
    }

}


bool get(KeyType id, ValueType& value)
{
    if( !isOpen() ) return false;
    if (id > getItemCount() )
    {
        return false;
    }
    if(this->getInCache(id, value))
    {
        return true;
    }
    else
    {

        return getOnFile(id, value);
    }


}

template <typename T>
bool get(const std::vector<T>& idList, std::vector<ValueType>& valueList)
{
    if( !isOpen() ) return false;
    std::vector<T> sortedIdList(idList);
    std::sort(sortedIdList.begin(), sortedIdList.end());
    uint32_t falseCount = 0;
    valueList.resize(sortedIdList.size());
    for(uint32_t i=0;i<sortedIdList.size();i++)
    {
        ValueType value;
        bool b = get(sortedIdList[i], value);
        valueList[i] = value;
        if(!b) falseCount++;
    }
    if( falseCount == valueList.size() )
    {
        return false;
    }
    return true;


}

bool getOnFile(KeyType id, ValueType& value)
{
    char* ptr = NULL;
    int sp;
    {
        boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
        ptr = fileData_.get(id, sp);
    }
    if(ptr==NULL) return false;
    SerializeType::deserialize(ptr, (std::size_t)sp, value);
    free(ptr);
    return true;
}

void flush()
{
    if( !isOpen() ) return;
    fileData_.flush();
}

void close()
{
    if(isOpen())
    {
        fileData_.close();
        isOpen_ = false;
    }
}




private:
    std::string file_;
    bool isOpen_;
    Container fileData_;
    uint32_t valueSize_;
    uint32_t fileLimitSize_;
    boost::mutex readWriteMutex_;
    boost::shared_mutex loadCacheMutex_;
};

NS_IZENELIB_AM_END
#endif
