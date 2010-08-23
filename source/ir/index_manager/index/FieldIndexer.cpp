#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/TermPositions.h>
#include <ir/index_manager/store/IndexInput.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;

namespace bfs = boost::filesystem;

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

FieldIndexer::FieldIndexer(const char* field, MemCache* pCache, Indexer* pIndexer)
        :field_(field),pMemCache_(pCache),pIndexer_(pIndexer),vocFilePointer_(0),alloc_(0),sorter_(0),termCount_(0)
{
    skipInterval_ = pIndexer_->getSkipInterval();
    maxSkipLevel_ = pIndexer_->getMaxSkipLevel();
    if (! pIndexer_->isRealTime())
    {
        //std::string sorterName = field_+".tmp";
        sorterFileName_ = field_+".tmp";
        bfs::path path(bfs::path(pIndexer->pConfigurationManager_->indexStrategy_.indexLocation_) /bfs::path(sorterFileName_));
        sorterFullPath_= path.string();
#if COMPRESSED_SORT
        sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true,SortIO<FieldIndexIO> >(sorterFullPath_.c_str(), 100000000);
#else
        sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>(sorterFullPath_.c_str(), 100000000);
#endif
    }
    else
        alloc_ = new boost::scoped_alloc(recycle_);
}

FieldIndexer::~FieldIndexer()
{
    /*
        InMemoryPostingMap::iterator iter = postingMap_.begin()
        for(; iter !=postingMap_.end(); ++iter)
        {
            delete iter->second;
        }
    */
    if (alloc_) delete alloc_;
    pMemCache_ = NULL;
    if (sorter_) delete sorter_;
}

#if COMPRESSED_SORT
uint8_t convert(char* data, uint32_t termId, uint32_t docId, uint32_t offset)
{
    char* pData = data;
    memcpy(pData, &(termId), sizeof(termId));
    pData += sizeof(uint32_t);
    FieldIndexIO::addVInt(pData, docId);
    FieldIndexIO::addVInt(pData, offset);
    return pData - data;
}
#else
void convert(char* data, uint32_t termId, uint32_t docId, uint32_t offset)
{
    char* pData = data;
    memcpy(pData, &(termId), sizeof(termId));
    pData += sizeof(termId);
    memcpy(pData, &(docId), sizeof(docId));
    pData += sizeof(docId);
    memcpy(pData, &(offset), sizeof(offset));
    pData += sizeof(offset);
}
#endif

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<LAInput> laInput)
{
    if (pIndexer_->isRealTime())
    {
        InMemoryPosting* curPosting;
        for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
        {
            InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->termId_);
            if (postingIter == postingMap_.end())
            {
                //curPosting = new InMemoryPosting(pMemCache_, skipInterval_, maxSkipLevel_);
                curPosting = BOOST_NEW(*alloc_, InMemoryPosting)(pMemCache_, skipInterval_, maxSkipLevel_);
                postingMap_[iter->termId_] = curPosting;
            }
            else
                curPosting = postingIter->second;
            curPosting->add(docid, iter->offset_);
        }
    }
    else
    {
        if (!sorter_)
#if COMPRESSED_SORT
            sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true,SortIO<FieldIndexIO> >(sorterFullPath_.c_str(), 100000000);
#else
            sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>(sorterFullPath_.c_str(), 100000000);
#endif

        char data[12];
        uint8_t len = 12;
        for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
        {
#if COMPRESSED_SORT
            len = convert(data,iter->termId_,docid,iter->offset_);
#else
            convert(data,iter->termId_,docid,iter->offset_);
#endif
            sorter_->add_data(len,data);
        }
    }
}

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<ForwardIndex> forwardindex)
{
    InMemoryPosting* curPosting;

    for (ForwardIndex::iterator iter = forwardindex->begin(); iter != forwardindex->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->first);
        if (postingIter == postingMap_.end())
        {
            curPosting = BOOST_NEW(*alloc_, InMemoryPosting)(pMemCache_, skipInterval_, maxSkipLevel_);
            postingMap_[iter->first] = curPosting;
        }
        else
            curPosting = postingIter->second;

        ForwardIndexOffset::iterator	endit = iter->second->end();
        for (ForwardIndexOffset::iterator it = iter->second->begin(); it != endit; ++it)
            curPosting->add(docid, *it);
    }
}

void FieldIndexer::reset()
{
    InMemoryPosting* pPosting;
    for (InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        pPosting = iter->second;
        if (!pPosting->hasNoChunk())
        {
            pPosting->reset();		///clear posting data
        }
    }
    postingMap_.clear();
    termCount_ = 0;
}

void writeTermInfo(IndexOutput* pVocWriter, termid_t tid, const TermInfo& termInfo)
{
    pVocWriter->writeInt(tid);					///write term id
    pVocWriter->writeInt(termInfo.docFreq_);		///write df
    pVocWriter->writeInt(termInfo.ctf_);			///write ctf
    pVocWriter->writeInt(termInfo.lastDocID_);		///write last doc id
    pVocWriter->writeInt(termInfo.skipLevel_);		///write skip level
    pVocWriter->writeLong(termInfo.skipPointer_);	///write skip list offset offset
    pVocWriter->writeLong(termInfo.docPointer_);	///write document posting offset
    pVocWriter->writeInt(termInfo.docPostingLen_);	///write document posting length (without skiplist)
    pVocWriter->writeLong(termInfo.positionPointer_);	///write position posting offset
    pVocWriter->writeInt(termInfo.positionPostingLen_);///write position posting length
}

#pragma pack(push,1)
struct Record
{
    uint8_t len;
    uint32_t tid;
    uint32_t docId;
    uint32_t offset;
};
#pragma pack(pop)

fileoffset_t FieldIndexer::write(OutputDescriptor* pWriterDesc)
{
    vocFilePointer_ = pWriterDesc->getVocOutput()->getFilePointer();

    IndexOutput* pVocWriter = pWriterDesc->getVocOutput();

    termid_t tid;
    InMemoryPosting* pPosting;
    fileoffset_t vocOffset = pVocWriter->getFilePointer();
    TermInfo termInfo;

    if (pIndexer_->isRealTime())
    {
        izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(rwLock_);
        for (InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
        {
            pPosting = iter->second;
            pPosting->setDirty(true);
            if (!pPosting->hasNoChunk())
            {
                tid = iter->first;
                pPosting->write(pWriterDesc, termInfo);		///write posting data
                writeTermInfo(pVocWriter,tid,termInfo);
                pPosting->reset();								///clear posting data
                termInfo.reset();
                termCount_++;
            }
            //delete pPosting;
            //pPosting = NULL;
        }

        postingMap_.clear();

        delete alloc_;
        alloc_ = new boost::scoped_alloc(recycle_);
    }
    else
    {
        if( !boost::filesystem::exists(sorterFullPath_) )
        {
            SF1V5_THROW(ERROR_FILEIO,"Open file error: " + sorterFullPath_);
        }
        sorter_->sort();

#if COMPRESSED_SORT
        FILE* f = fopen(sorterFullPath_.c_str(),"r");
        uint64_t count;

        FieldIndexIO ioStream(f);
        Record r;
        ioStream._readBytes((char*)(&count),sizeof(uint64_t));
#else
        IndexInput* pSortedInput = pIndexer_->getDirectory()->openInput(sorterFileName_);
        uint64_t count = pSortedInput->readLongBySmallEndian();
#endif
        pPosting = new InMemoryPosting(pMemCache_, skipInterval_, maxSkipLevel_);
        termid_t lastTerm = BAD_DOCID;
        try
        {
        for (uint64_t i = 0; i < count; ++i)
        {
#if COMPRESSED_SORT
            ioStream._read((char *)(&r), 13);
            if(r.tid != lastTerm && lastTerm != BAD_DOCID)
            {
                if (!pPosting->hasNoChunk())
                {
                    pPosting->write(pWriterDesc, termInfo); 	///write posting data
                    writeTermInfo(pVocWriter, lastTerm, termInfo);
                    pPosting->reset();								///clear posting data
                    pMemCache_->flushMem();
                    termInfo.reset();
                    termCount_++;
                }
            }
            pPosting->add(r.docId, r.offset);
            lastTerm = r.tid;
#else
            pSortedInput->readByte();
            tid = pSortedInput->readIntBySmallEndian();
            uint32_t docId = pSortedInput->readIntBySmallEndian();
            uint32_t offset = pSortedInput->readIntBySmallEndian();

            if(tid != lastTerm && lastTerm != BAD_DOCID)
            {
                if (!pPosting->hasNoChunk())
                {
                    pPosting->write(pWriterDesc, termInfo); 	///write posting data
                    writeTermInfo(pVocWriter, lastTerm, termInfo);
                    pPosting->reset();								///clear posting data
                    pMemCache_->flushMem();
                    termInfo.reset();
                    termCount_++;
                }
            }
            pPosting->add(docId, offset);
            lastTerm = tid;

#endif
        }
        }catch(std::exception& e)
        {
        cout<<e.what()<<endl;
        }

        if (!pPosting->hasNoChunk())
        {
            pPosting->write(pWriterDesc, termInfo);	///write posting data
            writeTermInfo(pVocWriter, lastTerm, termInfo);
            pPosting->reset(); 							///clear posting data
            pMemCache_->flushMem();
            termInfo.reset();
            termCount_++;
        }
#if COMPRESSED_SORT
        fclose(f);
#else
        delete pSortedInput;
#endif
        sorter_->clear_files();
        delete sorter_;
        sorter_ = NULL;
        delete pPosting;
    }

    fileoffset_t vocDescOffset = pVocWriter->getFilePointer();
    int64_t vocLength = vocDescOffset - vocOffset;
    ///begin write vocabulary descriptor
    pVocWriter->writeLong(vocLength);	///<VocLength(Int64)>
    pVocWriter->writeLong(termCount_);	///<TermCount(Int64)>
    ///end write vocabulary descriptor

    return vocDescOffset;
}

TermReader* FieldIndexer::termReader()
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(rwLock_);
    return new InMemoryTermReader(getField(),this);
}

}

NS_IZENELIB_IR_END

