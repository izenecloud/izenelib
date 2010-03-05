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
#include <boost/serialization/deque.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/format.hpp>

#include <cache/IzeneCache.h>
#include <util/izene_serialization.h>
#include <util/Exception.h>

NS_IZENELIB_AM_BEGIN

template <typename ValueType>
class SequenceFile
{
typedef std::vector<ValueType> CacheType;
typedef typename CacheType::size_type KeyType;

public:
    

SequenceFile(const std::string& file) :
file_(file), isOpen_(false), stream_(), itemCount_(0),valueSize_(0), cacheSize_(10000000), cache_(NULL), nextKey_(0)
{
    
}

~SequenceFile()
{
    close();
}

void setCacheSize(KeyType cacheSize)
{
    cacheSize_ = cacheSize;
}

void setValueSize(const ValueType& value)
{
    char* ptr;
    std::size_t vsize;
    izenelib::util::izene_serialization<ValueType> izs(value);
    izs.write_image(ptr, vsize);
    valueSize_ = vsize;
}

void open()
{
    if( isOpen() ) return;
    if (!boost::filesystem::exists(file_))
    {
        std::ofstream ostream(file_.c_str());
        ostream.close();
    }
    stream_.open(file_.c_str());
    if (!stream_.is_open())
    {
        IZENELIB_THROW("SequenceFile open on "+file_);
    }
    isOpen_ = true;
    if( valueSize_ == 0 )
    {
        ValueType _value;
        setValueSize(_value);
    }
    
    loadItemCount_();
    headSize_ = sizeof(getItemCount());
    stream_.flush();
    
    loadCache_();
    
}

bool isOpen() const
{
    return isOpen_;
}



KeyType getItemCount() const
{
    return itemCount_;
}





bool insert(KeyType id, const ValueType& value)
{
    if( !isOpen() ) return false;
    if( id >= itemCount_)
    {
        return update(id, value);
    }
    return false;
    
}

void update(KeyType id, const ValueType& value)
{
    if( !isOpen() ) return;
    boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
    seekToItemBeginWithExpand_(id);
    nextKey_ = id;
    append_(value);
    writeItemCount_();
    nextKey_ = getItemCount();
}

void append(const ValueType& value)
{
    if( !isOpen() ) return;
    boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
    append_(value);
    itemCount_ += 1;
}




bool get(KeyType id, ValueType& value)
{
    if( !isOpen() ) return false;
    if (id >= itemCount_)
    {
        return false;
    }
    if(id < (*cache_).size())
    {
        value = (*cache_)[id];
    }
    else
    {
        boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
        return getOnFile_(id, value);
    }
    return true;
    
}


void flush()
{
    if( !isOpen() ) return;
    boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
    writeItemCount_();
    stream_.flush();
    if( stream_.fail() )
    {
        IZENELIB_THROW("SequenceFile open "+file_);
    }
}

void close()
{
    if(isOpen())
    {
//         flush();
        stream_.close();
        if(cache_ != NULL)
        {
            delete cache_;
            cache_ = NULL;
        }
        
        isOpen_ = false;
    }
}

private:
    
void loadItemCount_()
{
    if( !isOpen() ) return;
//     std::cout<<" loadItemCount"<<std::endl;
    boost::lock_guard<boost::mutex> mLock(readWriteMutex_);
    stream_.seekg(0, ios::end);
    streampos size = stream_.tellg();
//     std::cout<<" loadItemCount "<<size<<(streampos) sizeof(itemCount_)<<std::endl;
    if (size < (streampos) sizeof(itemCount_) )
    {
        itemCount_ = 0;
        writeItemCount_();
//         std::cout<<" tellg after load : "<<stream_.tellg()<<std::endl;
    }
    else
    {
        stream_.seekg(0, ios::beg);
        stream_.read((char*) &itemCount_, sizeof(itemCount_));
    }
    if( stream_.fail() )
    {
        IZENELIB_THROW("SequenceFile loadItemCount on "+file_);
    }
}    

void writeItemCount_()
{
    if( !isOpen() ) return;
    stream_.seekg(0, ios::beg);
    stream_.write((char*) &itemCount_, sizeof(itemCount_));
    if( stream_.fail() )
    {
        IZENELIB_THROW("SequenceFile writeItemCount on "+file_);
        
    }
    stream_.seekg(0, ios::end);
}

void loadCache_()
{
    if( !isOpen() ) return;
    boost::lock_guard<boost::shared_mutex> mLock(loadCacheMutex_);
    if( cache_ == NULL)
    {
        if(cache_ != NULL)
        {
            delete cache_;
        }
        cache_ = new CacheType(cacheSize_);
        if (itemCount_ > 0)
        {
            KeyType cache_num = getItemCount() > cacheSize_ ? cacheSize_
            : getItemCount();
            ValueType value;
            for(KeyType i=0;i<cache_num;i++)
            {
                bool b = getOnFile_(i, value);
                if(b)
                    (*cache_)[i] = value;
                
            }
        }
    }
    
}



void seekToItemBeginWithExpand_(KeyType id)
{
    if( !isOpen() ) return;
    if (id >= itemCount_)//new item
    {
        stream_.seekg(0, ios::end);
        streampos size = stream_.tellg();
        streampos expectSize = (streampos) (headSize_ + itemCount_ * valueSize_);
        if (size != expectSize )
        {
            IZENELIB_THROW( (boost::format("SequenceFile abnormal file size on %1%, param: %2%,%3%,%4%") % file_ % id % size % expectSize).str() );
        }
        uint64_t toBeAddSize = (id - itemCount_) * (uint64_t)valueSize_;
        uint32_t roundSize = 1000;
        char* tmpData = new char[roundSize];
        while(toBeAddSize > 0)
        {
            uint32_t writeSize = toBeAddSize>roundSize? roundSize : toBeAddSize;
            if( writeSize < roundSize )
            {
                delete[] tmpData;
                tmpData = new char[writeSize];
            }
            stream_.write(tmpData, writeSize);
            toBeAddSize -= writeSize;
        }
        delete[] tmpData;
    }
    else
    {
        stream_.seekg(headSize_ + id * valueSize_, ios::beg);
        if( stream_.fail() )
        {
            IZENELIB_THROW("SequenceFile seekToItemBegin "+file_);
        }
    }
    
    
}

void append_(const ValueType& value)
{
    if( !isOpen() ) return;
    char* ptr;
    std::size_t vsize;
    izenelib::util::izene_serialization<ValueType> izs(value);
    izs.write_image(ptr, vsize);
    
    if(valueSize_ == 0)//first insert
    {
        valueSize_ = vsize;
    }
    if( vsize != valueSize_ )
    {
        IZENELIB_THROW("SequenceFile value size error on "+file_);
    }

    stream_.write(ptr, valueSize_);
    KeyType id = nextKey_;
    if(id >= itemCount_)
    {
        itemCount_ = id+1;
    }
    if( stream_.fail() )
    {
        IZENELIB_THROW("SequenceFile append stream fail on "+file_);
    }
    if( id < (*cache_).size() )
    {
        (*cache_)[id] = value;
    }
    
    
    nextKey_ = id+1;
}

bool getOnFile_(KeyType id, ValueType& value)
{
    if( !isOpen() ) return false;
    if (id >= itemCount_)
    {
        return false;
    }
    char* data = new char[valueSize_];
    stream_.seekg(headSize_ + id * valueSize_, ios::beg);
    stream_.read(data, valueSize_);
    if( stream_.fail() )
    {
        IZENELIB_THROW("SequenceFile getOnFile on "+file_);
    }
    izenelib::util::izene_deserialization<ValueType> izd_value(data,(std::size_t)valueSize_);
    izd_value.read_image(value);
    delete[] data;
    return true;
}

private:
    std::string file_;
    bool isOpen_;
    std::fstream stream_;
    KeyType itemCount_;
    uint32_t headSize_;
    uint32_t valueSize_;
    KeyType cacheSize_;
    CacheType* cache_;
    KeyType nextKey_;
    boost::mutex readWriteMutex_;
    boost::shared_mutex loadCacheMutex_;
};

NS_IZENELIB_AM_END
#endif
