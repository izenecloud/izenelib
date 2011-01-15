///
/// @file SimpleSequenceFile.hpp
/// @brief A class for read, write and sort on simple sequence file
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2009-12-01 09:58:32
/// @date Updated 2009-12-01 09:58:32
///

#ifndef _SIMPLESEQUENCEFILE_HPP_
#define _SIMPLESEQUENCEFILE_HPP_

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <util/izene_serialization.h>
#include <am/external_sort/izene_sort.hpp>
#include <util/Exception.h>

NS_IZENELIB_AM_BEGIN
template <typename KeyType, typename ValueType, typename LenType=uint16_t>
class SimpleSequenceFileReader {
public:
    SimpleSequenceFileReader( const std::string& file ):
    file_(file), isOpen_(false), stream_(), itemCount_(0), index_(0), lastSet_(false), lastKey_(), lastValue_()
    {
    }
    ~SimpleSequenceFileReader()
    {
    }
    void open()
    {
        if( isOpen() ) return;
        stream_.open(file_.c_str());
        if (!stream_.is_open())
        {
            IZENELIB_THROW("SimpleSequenceFileReader open on "+file_);
        }
        else
        {
            isOpen_ = true;
            loadItemCount_();
            
        }
    }
    
    bool isOpen() const
    {
        return isOpen_;
    }
    
    std::string getPath() const
    {
        return file_;
    }
    
    
    bool next()
    {
        if( !isOpen() ) return false;
        if(index_ >= itemCount_ ) 
        {
            lastSet_ = false;
            return false;
        }
        LenType recodeSize = 0;
        stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read on "+file_);
        }
        char* data = new char[recodeSize];
        stream_.read( data, recodeSize );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read on "+file_);
        }
        char* pData = data;
        memcpy( &lastKey_, pData, sizeof(KeyType) );
        pData += sizeof(KeyType);
        std::size_t valueSize = recodeSize - sizeof(KeyType);
        izenelib::util::izene_deserialization<ValueType> izd_value(pData,valueSize);
        izd_value.read_image(lastValue_);
        delete[] data;
        index_++;
        lastSet_ = true;
        return true;
    }
    
    bool nextKeyList(std::vector<KeyType>& keyList)
    {
        if( !isOpen() ) return false;
        if(index_ >= itemCount_ ) 
        {
            lastSet_ = false;
            return false;
        }
        LenType recodeSize = 0;
        stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }
        if( recodeSize % sizeof(KeyType) != 0 )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist error on "+file_);
        }
        char* data = new char[recodeSize];
        stream_.read( data, recodeSize );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }
        keyList.resize( recodeSize / sizeof(KeyType) );
        char* pData = data;
        for( uint32_t i=0;i<keyList.size();i++)
        {
            memcpy( &keyList[i], pData, sizeof(KeyType) );
            pData += sizeof(KeyType);
        }
        delete[] data;
        index_++;
        lastSet_ = true;
        return true;
    }
    
    bool nextKey(KeyType& key)
    {
        if( !isOpen() ) return false;
        if(index_ >= itemCount_ ) 
        {
            lastSet_ = false;
            return false;
        }
        LenType recodeSize = 0;
        stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }
        if( recodeSize != sizeof(KeyType) )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist error on "+file_);
        }
        char* data = new char[recodeSize];
        stream_.read( data, recodeSize );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }

        memcpy( &key, data, sizeof(KeyType) );

        delete[] data;
        index_++;
        lastSet_ = true;
        return true;
    }
    
    bool next(KeyType& key, std::vector<ValueType>& valueList)
    {
        if( !isOpen() ) return false;
        if(index_ >= itemCount_ ) 
        {
            lastSet_ = false;
            return false;
        }
        LenType recodeSize = 0;
        stream_.read( (char*)&recodeSize, sizeof(recodeSize) );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }
        if( (recodeSize-sizeof(KeyType)) % sizeof(ValueType) != 0 )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read valuelist error on "+file_);
        }
        char* data = new char[recodeSize];
        stream_.read( data, recodeSize );
        if ( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader read keylist on "+file_);
        }
        
        char* pData = data;
        memcpy( &key, pData, sizeof(KeyType) );
        pData += sizeof(KeyType);

        valueList.resize( (recodeSize-sizeof(KeyType)) / sizeof(ValueType) );
        for( uint32_t i=0;i<valueList.size();i++)
        {
            memcpy( &valueList[i], pData, sizeof(ValueType) );
            pData += sizeof(ValueType);
        }
        delete[] data;
        index_++;
        lastSet_ = true;
        return true;
    }
    
    bool next(KeyType& key, ValueType& value)
    {
        if( !isOpen() ) return false;
        bool r = next();
        if(r)
        {
            key = lastKey_;
            value = lastValue_;
        }
        return r;
    }
    
    bool next(std::pair<KeyType, ValueType>& pairValue)
    {
        if( !isOpen() ) return false;
        bool r = next();
        if(r)
        {
            pairValue.first = lastKey_;
            pairValue.second = lastValue_;
        }
        return r;
    }
    
    bool hasCurrentPair()
    {
        if( !isOpen() ) return false;
        return lastSet_;
    }
    
    bool getCurrentKey(KeyType& key)
    {
        if( !isOpen() ) return false;
        if( lastSet_ )
        {
            key = lastKey_;
            return true;
        }
        return false;
    }
    
    bool getCurrentValue(ValueType& value)
    {
        if( !isOpen() ) return false;
        if( lastSet_ )
        {
            value = lastValue_;
            return true;
        }
        return false;
    }
    
    bool getCurrentPair(std::pair<KeyType, ValueType>& pairValue)
    {
        if( !isOpen() ) return false;
        if( lastSet_ )
        {
            pairValue.first = lastKey_;
            pairValue.second = lastValue_;
            return true;
        }
        return false;
    }
    
    uint64_t tellg()
    {
        return stream_.tellg();
    }
    
    void recordPosition()
    {
        posSet_ = stream_.tellg();
        indexSet_ = index_;
    }
    
    void backTo()
    {
        stream_.seekg(posSet_, ios::beg);
        index_ = indexSet_;
    }
    
    uint64_t getItemCount()
    {
        return itemCount_;
    }
    
    void flush()
    {
        
    }
    
    void close()
    {
        if( isOpen() )
        {
            stream_.close();
            isOpen_ = false;
        }
    }
    
private:
    void loadItemCount_()
    {
        stream_.seekg(0, ios::end);
        streampos size = stream_.tellg();
        if (size == (streampos) 0)
        {
            itemCount_ = 0;
        }
        else
        {
            stream_.seekg(0, ios::beg);
            stream_.read((char*) &itemCount_, sizeof(itemCount_));
        }
        if( stream_.fail() )
        {
            IZENELIB_THROW("SimpleSequenceFileReader loadItemCount on "+file_);
        }
    }    
    
private:
    std::string file_;
    bool isOpen_;
    std::ifstream stream_;
    uint64_t itemCount_;
    uint64_t index_;
    uint64_t indexSet_;
    uint64_t posSet_;
    bool lastSet_;
    KeyType lastKey_;
    ValueType lastValue_;
};

template <typename KeyType, typename ValueType, typename LenType=uint16_t>
class SimpleSequenceFileWriter {
    public:
        SimpleSequenceFileWriter( const std::string& file ):
        file_(file), isOpen_(false), stream_(), itemCount_(0)
        {
        }
        
        ~SimpleSequenceFileWriter()
        {
        }
        
        void open()
        {
            if (!boost::filesystem::exists(file_))
            {
                std::ofstream ostream(file_.c_str());
                ostream.close();
            }
            stream_.open(file_.c_str());
            if (!stream_.is_open())
            {
                IZENELIB_THROW("SimpleSequenceFileWriter open on "+file_);
                
            }
            isOpen_ = true;
            loadItemCount_();
            stream_.flush();
            SimpleSequenceFileReader<KeyType,ValueType,LenType> reader(file_);
            reader.open();
            while( reader.next() ) {}
            uint64_t lastPos = reader.tellg();
            reader.close();
            stream_.seekg(lastPos);
            if (stream_.fail())
            {
                IZENELIB_THROW("SimpleSequenceFileWriter open on "+file_);
                
            }
            
        }
        
        bool isOpen() const
        {
            return isOpen_;
        }
        
        std::string getPath() const
        {
            return file_;
        }
        
        uint64_t getItemCount()
        {
            return itemCount_;
        }
        
        
        
        
        
        void append(const KeyType& key, const ValueType& value)
        {
            if( !isOpen() ) return;
            char* ptr;
            std::size_t valueSize;
            izenelib::util::izene_serialization<ValueType> izs(value);
            izs.write_image(ptr, valueSize);
            LenType keySize = sizeof(KeyType);
            LenType recodeSize = keySize + valueSize;
            char* data = new char[recodeSize + sizeof(recodeSize)];
            char* pData = data;
            memcpy( pData, &recodeSize, sizeof(recodeSize) );
            pData += sizeof(recodeSize);
            memcpy( pData, &key, keySize );
            pData += keySize;
            memcpy( pData, ptr, valueSize );
            stream_.write(data, recodeSize+ sizeof(recodeSize));
            delete[] data;
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter append on "+file_);
                
            }
            itemCount_++;
        }
        
        void append(const KeyType& key, const std::vector<ValueType>& valueList)
        {
            if( !isOpen() ) return;
            
            std::size_t valueSize = sizeof(ValueType) * valueList.size();
            LenType keySize = sizeof(KeyType);
            LenType recodeSize = keySize + valueSize;
            char* data = new char[recodeSize + sizeof(recodeSize)];
            char* pData = data;
            memcpy( pData, &recodeSize, sizeof(recodeSize) );
            pData += sizeof(recodeSize);
            memcpy( pData, &key, keySize );
            pData += keySize;
            for( uint32_t i=0;i<valueList.size();i++)
            {
                memcpy( pData, &valueList[i], sizeof(ValueType) );
                pData += sizeof(ValueType);
            }
            stream_.write(data, recodeSize+ sizeof(recodeSize));
            delete[] data;
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter append on "+file_);
                
            }
            itemCount_++;
        }
        
        void append(const std::vector<KeyType>& keyList)
        {
            if( !isOpen() ) return;

            LenType keySize = sizeof(KeyType);
            LenType recodeSize = keySize * keyList.size();
            char* data = new char[recodeSize + sizeof(recodeSize)];
            char* pData = data;
            memcpy( pData, &recodeSize, sizeof(recodeSize) );
            pData += sizeof(recodeSize);
            for( uint32_t i=0;i<keyList.size();i++)
            {
                memcpy( pData, &keyList[i], keySize );
                pData += keySize;
            }
            
            stream_.write(data, recodeSize+ sizeof(recodeSize));
            delete[] data;
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter append on "+file_);
                
            }
            itemCount_++;
        }
        
        void append(const KeyType& key)
        {
            std::vector<KeyType> keyList(1, key);
            append(keyList);
        }
        
        void flush()
        {
            if( !isOpen() ) return;
            writeItemCount_();
            stream_.flush();
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter flush on "+file_);
                
            }
        }
        
        void close()
        {
            if( !isOpen() ) return;
            writeItemCount_();
            stream_.close();
            isOpen_ = false;
        }
        
    private:
        
        void writeItemCount_()
        {
            if( !isOpen() ) return;
            stream_.seekg(0, ios::beg);
            stream_.write((char*) &itemCount_, sizeof(itemCount_));
            stream_.seekg(0, ios::end);
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter writeItemCount on "+file_);
            }
        }
        
        void loadItemCount_()
        {
            stream_.seekg(0, ios::end);
            streampos size = stream_.tellg();
            if (size == (streampos) 0)
            {
                itemCount_ = 0;
                writeItemCount_();
            }
            else
            {
                stream_.seekg(0, ios::beg);
                stream_.read((char*) &itemCount_, sizeof(itemCount_));
            }
            if( stream_.fail() )
            {
                IZENELIB_THROW("SimpleSequenceFileWriter loadItemCount on "+file_);
            }
        }
        
    private:
        std::string file_;
        bool isOpen_;
        std::fstream stream_;
        uint64_t itemCount_;
        
};

template <typename KeyType, typename ValueType, typename LenType=uint16_t, bool COMPARE_ALL = false>
class SimpleSequenceFileSorter {
    public:
        SimpleSequenceFileSorter( )
        {
        }
        ~SimpleSequenceFileSorter()
        {
        }
        
        void sort(const std::string& file, uint32_t buf_times = 1)
        {
            izenelib::am::IzeneSort<KeyType, LenType, COMPARE_ALL> sorter("", 100000000*buf_times);
            sorter.sort(file);
        }
        
        void merge(const std::vector<std::string>& inputFileList, const std::string& outputFile)
        {
        }

                
    
        
};


template <typename KeyType, typename ValueType, typename ValueType2 = ValueType, typename LenType = uint16_t, typename LenType2 = LenType>
class SimpleSequenceFileMerger {
    typedef SimpleSequenceFileReader<KeyType, ValueType,LenType> reader_t;
    typedef SimpleSequenceFileReader<KeyType, ValueType2,LenType2> reader2_t;
    
    public:
        SimpleSequenceFileMerger( ):pReader1_(NULL), pReader2_(NULL)
        {
        }

        ~SimpleSequenceFileMerger()
        {
            if( pReader1_ != NULL )
            {
                pReader1_->close();
                delete pReader1_;
                pReader1_ = NULL;
            }
            if( pReader2_ != NULL )
            {
                pReader2_->close();
                delete pReader2_;
                pReader2_ = NULL;
            }
        }
        
        void setObj(const std::string& file, const std::string& anotherFile)
        {
            if( pReader1_ != NULL )
            {
                pReader1_->close();
                delete pReader1_;
                pReader1_ = NULL;
            }
            if( pReader2_ != NULL )
            {
                pReader2_->close();
                delete pReader2_;
                pReader2_ = NULL;
            }
            pReader1_ = new reader_t(file);
            pReader1_->open();
            pReader1_->next();
            pReader2_ = new reader2_t(anotherFile);
            pReader2_->open();
            pReader2_->next();
        }
        
        
        bool next(KeyType& key, std::vector<ValueType>& valueList1, std::vector<ValueType2>& valueList2)
        {
            if( pReader1_ == NULL ) return false;
            if( pReader2_ == NULL ) return false;
            valueList1.resize(0);
            valueList2.resize(0);
            bool keySet = false;
            std::pair<KeyType, ValueType> pairValue1;
            if( pReader1_->getCurrentPair(pairValue1) )
            {
                key = pairValue1.first;
                keySet = true;
            }
            std::pair<KeyType, ValueType2> pairValue2;
            if( pReader2_->getCurrentPair(pairValue2) )
            {
                if( keySet )
                {
                    if( pairValue2.first< key )
                    {
                        key = pairValue2.first;
                    }
                }
                else
                {
                    key = pairValue2.first;
                    keySet = true;
                }
            }
            if( !keySet )
            {
                return false;
            }
            while( true )
            {
                if( pReader1_->getCurrentPair(pairValue1) )
                {
                    if( pairValue1.first == key )
                    {
                        valueList1.push_back(pairValue1.second);
                        pReader1_->next();
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            while( true )
            {
                if( pReader2_->getCurrentPair(pairValue2) )
                {
                    if( pairValue2.first == key )
                    {
                        valueList2.push_back(pairValue2.second);
                        pReader2_->next();
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            return true;
            
            
        }


        
    private:
        reader_t* pReader1_;
        reader2_t* pReader2_;


                
    
        
};

template <typename KT, typename VT, typename LenType=uint16_t, bool COMPARE_ALL = false>
class SSFType
{
    public:
        typedef KT KeyType;
        typedef VT ValueType;
        typedef SimpleSequenceFileReader<KT, VT, LenType> ReaderType;
        typedef SimpleSequenceFileWriter<KT, VT, LenType> WriterType;
        typedef SimpleSequenceFileSorter<KT, VT, LenType, COMPARE_ALL> SorterType;
};

NS_IZENELIB_AM_END

#endif // _SIMPLESEQUENCEFILE_HPP_
