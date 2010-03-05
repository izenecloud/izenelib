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
            stream_.seekg(0, ios::end);
            if (stream_.fail())
            {
                IZENELIB_THROW("SimpleSequenceFileWriter open on "+file_);
                
            }
            
        }
        
        bool isOpen() const
        {
            return isOpen_;
        }
        
        std::string getPath()
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
        
        void sort(const std::string& file)
        {
            izenelib::am::IzeneSort<KeyType, LenType, COMPARE_ALL> sorter("");
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
            pReader2_ = new reader2_t(anotherFile);
            pReader2_->open();
        }
        
        
        bool next(KeyType& key, std::vector<ValueType>& valueList1, std::vector<ValueType2>& valueList2)
        {
            if( pReader1_ == NULL ) return false;
            if( pReader2_ == NULL ) return false;
            valueList1.resize(0);
            valueList2.resize(0);
            boost::optional<KeyType> theKey1;
            boost::optional<KeyType> theKey2;
            ValueType lastValue1;
            ValueType2 lastValue2;
            while( true )
            {
                std::pair<KeyType, ValueType> pairValue1;
                if( pReader1_->getCurrentPair(pairValue1) )
                {
                    theKey1 = pairValue1.first;
                    lastValue1 = pairValue1.second;
                }
                std::pair<KeyType, ValueType2> pairValue2;
                if( pReader2_->getCurrentPair(pairValue2) )
                {
                    theKey2 = pairValue2.first;
                    lastValue2 = pairValue2.second;
                }
                bool bAddValue1 = false;
                bool bAddValue2 = false;
                if( theKey1 && !theKey2 )
                {
                    key = theKey1.get();
                    valueList1.push_back( lastValue1 );
                    bAddValue1 = true;
                    
                }
                else if ( !theKey1 && theKey2 )
                {
                    key = theKey2.get();
                    valueList2.push_back( lastValue2 );
                    bAddValue2 = true;
                    
                }
                else if (!theKey1 && !theKey2)
                {
                    bool b1 = pReader1_->next();
                    bool b2 = pReader2_->next();
                    if( !b1 && !b2 )
                    {
                        return false;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (theKey1 && theKey2)
                {
                    if( theKey1.get() < theKey2.get() )
                    {
                        key = theKey1.get();
                        valueList1.push_back( lastValue1 );
                        bAddValue1 = true;
                    }
                    else if ( theKey1.get() > theKey2.get() )
                    {
                        key = theKey2.get();
                        valueList2.push_back( lastValue2 );
                        bAddValue2 = true;
                    }
                    else
                    {
                        key = theKey1.get();
                        valueList1.push_back( lastValue1 );
                        valueList2.push_back( lastValue2 );
                        bAddValue1 = true;
                        bAddValue2 = true;
                    }
                }
                if( bAddValue1 )
                {
                    while(true)
                    {
                        bool b = pReader1_->next(pairValue1);
                        if( !b ) return true;
                        else
                        {
                            if( pairValue1.first == key )
                            {
                                valueList1.push_back( pairValue1.second );
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                if( bAddValue2 )
                {
                    while(true)
                    {
                        bool b = pReader2_->next(pairValue2);
                        if( !b ) return true;
                        else
                        {
                            if( pairValue2.first == key )
                            {
                                valueList2.push_back( pairValue2.second );
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                return true;
            }
            
            
        }


        
    private:
        reader_t* pReader1_;
        reader2_t* pReader2_;


                
    
        
};

NS_IZENELIB_AM_END

#endif // _SIMPLESEQUENCEFILE_HPP_
