/**
* @file        FieldIndexer.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index the documents into barrels
*/
#ifndef FIELDINDEXER_H
#define FIELDINDEXER_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/MemCache.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/RTPostingWriter.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/SortHelper.h>
#include <ir/index_manager/utility/IndexManagerConfig.h>
#include <am/external_sort/izene_sort.hpp>
#include <ir/index_manager/index/IndexBinlog.h>

#include <3rdparty/am/stx/btree_map>

#include <boost/thread.hpp>

#include <string>
#include <deque>

using namespace izenelib::util;
using namespace izenelib::am;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class FieldIndexIO
{
public:
    FieldIndexIO(FILE* fd,const string& mode = "r")
    {
        fd_ = fd;
        bufferSize_ = 4096*4;

        writeBufferPosition_ = 0;
        readBufferStart_ = 0;
        readBufferLength_ = 0;
        readBufferPosition_ = 0;

        if (mode.compare("w") == 0)
        {
            writeBuffer_ = new char[bufferSize_];
            readBuffer_ = 0;
            length_ = 0;
        }
        else if (mode.compare("r") == 0)
        {
            readBuffer_ = new char[bufferSize_];
            writeBuffer_ = 0;

            fseek(fd_, 0, SEEK_END);
            length_ = ftell(fd_);
            fseek(fd_, 0, SEEK_SET);
        }
        else
            SF1V5_THROW(ERROR_FILEIO,"Izenesort :not supported IO");
    }

    ~FieldIndexIO()
    {
        if(readBuffer_) delete[] readBuffer_;
        if(writeBuffer_) delete[] writeBuffer_;
    }

public:
    bool _isCompression() { return true;}

    size_t _write(char* const data, size_t length)
    {
        size_t count = RECORD_ALL_LENGTH;
        char* p = data;
        uint32_t* q = 0;
        while(count <= length)
        {
            if (writeBufferPosition_ + RECORD_ALL_LENGTH >= bufferSize_)
                _flush();
            //assert(*p == 12);
            _writeByte(*p++);
            q = (uint32_t*)p;
            _writeInt(*q++);
            _writeVInt(*q++);
            _writeVInt(*q++);
            p += RECORD_VALUE_LENGTH;
            count += RECORD_ALL_LENGTH;
        }
        if (writeBufferPosition_)
            _flush();
        return 1;
    }

    void writeRecord(TermId* const data, size_t length)
    {
        size_t count = RECORD_VALUE_LENGTH;
        uint32_t* q = (uint32_t*)data;
        while(count <= length)
        {
            if (writeBufferPosition_ + RECORD_ALL_LENGTH >= bufferSize_)
                _flush();
            _writeByte(RECORD_VALUE_LENGTH);
            _writeInt(*q++);
            _writeVInt(*q++);
            _writeVInt(*q++);
            count += RECORD_VALUE_LENGTH;
        }
        if (writeBufferPosition_)
            _flush();
    }

    size_t _read(char* data, size_t length)
    {
        size_t count = RECORD_ALL_LENGTH;
        //uint8_t len;
        char* p = data;
        uint32_t* q = 0;
        while((count <= length)&&(!_isEof()))
        {
            //len = _readByte();
            _readByte();
            //assert(len<=12);
            *p++ = RECORD_VALUE_LENGTH;
            q = (uint32_t*)p;
            *q++ = _readInt();
            *q++ = _readVInt();
            *q++ = _readVInt();
            p += RECORD_VALUE_LENGTH;
            count += RECORD_ALL_LENGTH;
        }
        return count - RECORD_ALL_LENGTH;
    }

    size_t _length()
    {
        return length_;
    }

    void _readBytes(char* _b, size_t _len){
        int32_t len = _len;
        char* b = _b;

        if(len <= (readBufferLength_-readBufferPosition_))
        {
            // the buffer contains enough data to satisfy this request
            if(len>0) // to allow b to be null if len is 0...
                memcpy(b, readBuffer_+ readBufferPosition_, len);
            readBufferPosition_+=len;
        }
        else
        {
            // the buffer does not have enough data. First serve all we've got.
            int32_t available = readBufferLength_- readBufferPosition_;
            if(available > 0)
            {
                memcpy(b, readBuffer_ + readBufferPosition_, available);
                b += available;
                len -= available;
                readBufferPosition_ += available;
            }
            // and now, read the remaining 'len' bytes:
            if ( len<(int32_t)bufferSize_)
            {
                // If the amount left to read is small enough, and
                // we are allowed to use our buffer, do it in the usual
                // buffered way: fill the buffer and copy from it:
                _refill();
                if(readBufferLength_<len)
                {
                    // Throw an exception when refill() could not read len bytes:
                    memcpy(b, readBuffer_, readBufferLength_);
                    //_CLTHROWA(CL_ERR_IO, "read past EOF");
                }
                else
                {
                    memcpy(b, readBuffer_, len);
                    readBufferPosition_=len;
                }
            }
            else
            {
                // The amount left to read is larger than the buffer
                // or we've been asked to not use our buffer -
                // there's no performance reason not to read it all
                // at once. Note that unlike the previous code of
                // this function, there is no need to do a seek
                // here, because there's no need to reread what we
                // had in the buffer.
                size_t after = readBufferStart_+readBufferPosition_+len;
                //if(after > length_)
                //_CLTHROWA(CL_ERR_IO, "read past EOF");
                IASSERT(fread(b,len,1,fd_) == 1);
                readBufferStart_ = after;
                readBufferPosition_ = 0;
                readBufferLength_ = 0; 				   // trigger refill() on read
            }
        }
    }

    size_t _tell()
    {
        return readBufferStart_ + readBufferPosition_;
    }

    bool _isEof()
    {
        return ( (readBufferStart_ + readBufferPosition_) >= length_);
    }

    void _seek(size_t pos, int origin = SEEK_SET)
    {
        if (pos > length_)
            SF1V5_THROW(ERROR_FILEIO,"Izenesort :pos>length_");
        if (pos >= readBufferStart_ && pos < (readBufferStart_ + readBufferLength_))
            readBufferPosition_ = pos - readBufferStart_;
        else
        {
            readBufferStart_ = pos;
            readBufferPosition_ = 0;
            readBufferLength_ = 0;
            fseek(fd_, pos, SEEK_SET);
        }
    }

    static void addVInt(char* &data, uint32_t value)
    {
        while ((value & ~0x7F) != 0)
        {
            *data++ = (uint8_t)((value & 0x7f) | 0x80);
            value >>= 7;
        }
        *data++ = (uint8_t)(value);
    }

private:
    uint8_t _readByte()
    {
        if (readBufferPosition_ >= readBufferLength_)
            _refill();
        return readBuffer_[readBufferPosition_++];
    }
    void _refill()
    {
        size_t start = readBufferStart_ + readBufferPosition_;
        size_t end = start + bufferSize_;
        if (end > length_)
            end = length_;
        readBufferLength_ = end - start;
        if (readBufferLength_ <= 0)
           SF1V5_THROW(ERROR_FILEIO,"Izenesort :read past EOF.");

        IASSERT(fread(readBuffer_, readBufferLength_, 1, fd_) == 1);
        readBufferStart_ = start;
        readBufferPosition_ = 0;
    }

    uint32_t _readVInt()
    {
        uint8_t b = _readByte();
        int32_t i = b & 0x7F;
        for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
        {
            b = _readByte();
            i |= (b & 0x7FL) << shift;
        }
        return i;
    }

    uint32_t _readInt()
    {
        uint8_t b1 = _readByte();
        uint8_t b2 = _readByte();
        uint8_t b3 = _readByte();
        uint8_t b4 = _readByte();
        return ((b4 & 0xFF) << 24) | ((b3 & 0xFF) << 16) | ((b2 & 0xFF) <<  8)
           | (b1 & 0xFF);
    }

    void _writeByte(uint8_t b)
    {
        writeBuffer_[writeBufferPosition_++] = b;
    }

    void  _writeInt(uint32_t i)
    {
        _writeByte((uint8_t) i);
        _writeByte((uint8_t) (i >> 8));
        _writeByte((uint8_t) (i >> 16));
        _writeByte((uint8_t) (i >> 24));
    }

    void  _writeVInt(uint32_t ui)
    {
        while ((ui & ~0x7F) != 0)
        {
            _writeByte((uint8_t)((ui & 0x7f) | 0x80));
            ui >>= 7;
        }
        _writeByte( (uint8_t)ui);
    }

    void _flush()
    {
        IASSERT(fwrite(writeBuffer_, 1, writeBufferPosition_, fd_));
        writeBufferPosition_ = 0;
    }

private:
    FILE* fd_;

    size_t bufferSize_;

    char* readBuffer_;

    size_t readBufferStart_;	 /// position in file of buffer_

    int32_t readBufferLength_; /// end of valid bytes

    int32_t readBufferPosition_; /// next byte to read

    size_t length_;	 /// we only read one file for each FieldIO instance

    char* writeBuffer_;

    size_t writeBufferPosition_;

friend class FieldIndexer;
    static const uint8_t RECORD_ALL_LENGTH = 13;
    static const uint8_t RECORD_VALUE_LENGTH = 12;
};


//Since TermID is got from hashfunc, DynamicArray is not suitable to be used as the container.
typedef stx::btree_map<unsigned int, boost::shared_ptr<RTPostingWriter> > InMemoryPostingMap;

class TermReader;
/**
*@brief  FieldIndexer will is the internal indexer of CollectionIndexer, when indexing a document, it will choose the according
* FieldIndexer to process. Each Field has its own FieldIndexer
*/
class Indexer;
class FieldIndexer
{
public:
    FieldIndexer(const char* field, Indexer* pIndexer);

    ~FieldIndexer();
public:
    void checkBinlog();

    void addBinlog(docid_t docid, boost::shared_ptr<LAInput> laInput);

    const char* getField() { return field_.c_str(); }

    void setField(const char* strfield) { field_ = strfield;}

    void addField(docid_t docid, boost::shared_ptr<LAInput> laInput);

    void setIndexMode(boost::shared_ptr<MemCache> pMemCache, size_t nBatchMemSize, bool realtime);

    void reset();

    uint64_t distinctNumTerms() {return termCount_;}

    fileoffset_t write(OutputDescriptor* pWriterDesc);

    void setFilePointer(fileoffset_t off) {vocFilePointer_ = off;};

    fileoffset_t getFilePointer() { return vocFilePointer_;};

    TermReader* termReader();

    /// whether indices for this field is empty
    bool isEmpty();

    void deletebinlog();
private:
    ///set memory cache size for izene sort
    void setHitBuffer_(size_t size);

    void writeHitBuffer_(int iHits);
    /// whether indices for this field is empty when indexing under batch mode
    bool isBatchEmpty_();

private:

    Binlog* pBinlog_;
    
    InMemoryPostingMap postingMap_;

    std::string field_;

    boost::shared_ptr<MemCache> pMemCache_;

    Indexer* pIndexer_;

    fileoffset_t vocFilePointer_;

    int skipInterval_;

    int maxSkipLevel_;

    boost::shared_mutex rwLock_;

    std::string sorterFullPath_;

    std::string sorterFileName_;

    std::string BinlogPath_;

    FILE* f_;

    uint64_t termCount_;

    size_t iHitsMax_;

    size_t recordCount_;

    size_t run_num_;

    AutoArray<TermId> hits_;

    TermId* pHits_;

    TermId* pHitsMax_;

    bool flush_;

    IndexLevel indexLevel_;

    friend class MemTermReader;
    friend class MemTermIterator;
};

}

NS_IZENELIB_IR_END

#endif
