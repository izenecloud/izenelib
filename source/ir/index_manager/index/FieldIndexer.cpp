#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/TermPositions.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;

namespace bfs = boost::filesystem;

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

FieldIndexer::FieldIndexer(const char* field, MemCache* pCache, Indexer* pIndexer)
        :field_(field),pMemCache_(pCache),pIndexer_(pIndexer),vocFilePointer_(0),alloc_(0),sorter_(0)
{
    skipInterval_ = pIndexer_->getSkipInterval();
    maxSkipLevel_ = pIndexer_->getMaxSkipLevel();
/*
    if (! pIndexer_->isRealTime())
    {
        std::string sorterName = field_+".tmp";
        bfs::path path(bfs::path(pIndexer->pConfigurationManager_->indexStrategy_.indexLocation_) /bfs::path(sorterName));
        sorterFullPath_= path.string();
        sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>(sorterFullPath_.c_str(), 100000000);
    }
    else
*/		
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

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<LAInput> laInput)
{
    //if (pIndexer_->isRealTime())
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
/*
    else
    {
        if (!sorter_)
            sorter_=new izenelib::am::IzeneSort<uint32_t, uint8_t, true>(sorterFullPath_.c_str(), 100000000);

        char data[12];
        for (LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
        {
            convert(data,iter->termId_,docid,iter->offset_);
            sorter_->add_data(12, data);
        }
    }
*/
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

fileoffset_t FieldIndexer::write(OutputDescriptor* pWriterDesc)
{
    vocFilePointer_ = pWriterDesc->getVocOutput()->getFilePointer();

    IndexOutput* pVocWriter = pWriterDesc->getVocOutput();

    termid_t tid;
    int32_t termCount = 0;
    InMemoryPosting* pPosting;
    fileoffset_t vocOffset = pVocWriter->getFilePointer();
    TermInfo termInfo;

    //if (pIndexer_->isRealTime())
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
                termCount++;
            }
            //delete pPosting;
            //pPosting = NULL;
        }

        postingMap_.clear();

        delete alloc_;
        alloc_ = new boost::scoped_alloc(recycle_);
    }
/*
    else
    {
        sorter_->sort();
        FILE* f = fopen(sorterFullPath_.c_str(),"r");
        fseek(f, 0, SEEK_SET);
        uint64_t count;
        fread(&count, sizeof(uint64_t), 1, f);
        pPosting = new InMemoryPosting(pMemCache_, skipInterval_, maxSkipLevel_);
        termid_t lastTerm = 0;
        try
        {
        for (uint64_t i = 0; i < count; ++i)
        {
            uint8_t len;
            fread(&len, sizeof(uint8_t), 1, f);
            uint32_t docId,offset;
            fread(&tid, sizeof(uint32_t), 1, f);
            fread(&docId, sizeof(uint32_t), 1, f);
            fread(&offset, sizeof(uint32_t), 1, f);

            if ((lastTerm == 0)||(tid == lastTerm))
            {
                pPosting->add(docId, offset);
            }
            else
            {
                if (!pPosting->hasNoChunk())
                {
                    pPosting->write(pWriterDesc, termInfo); 	///write posting data
                    writeTermInfo(pVocWriter, lastTerm, termInfo);
                    pPosting->reset();								///clear posting data
                    termInfo.reset();
                    termCount++;
                }
            }
            lastTerm = tid;
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
            termInfo.reset();
            termCount++;
        }

        fclose(f);
        sorter_->clear_files();
        delete sorter_;
        sorter_ = NULL;
        delete pPosting;
    }
*/

    fileoffset_t vocDescOffset = pVocWriter->getFilePointer();
    int64_t vocLength = vocDescOffset - vocOffset;
    ///begin write vocabulary descriptor
    pVocWriter->writeLong(vocLength);	///<VocLength(Int64)>
    pVocWriter->writeLong(termCount);	///<TermCount(Int64)>
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

