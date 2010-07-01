#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/FieldIndexer.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/TermPositions.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

FieldIndexer::FieldIndexer(MemCache* pCache, Indexer* pIndexer)
    :pMemCache_(pCache),pIndexer_(pIndexer),vocFilePointer_(0)
{
}

FieldIndexer::~FieldIndexer()
{
    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        delete iter->second;
    }

    pMemCache_ = NULL;
}

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<LAInput> laInput)
{
    InMemoryPosting* curPosting;

    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->termId_);
        if(postingIter == postingMap_.end())
        {
            curPosting = new InMemoryPosting(pMemCache_, pIndexer_->getSkipInterval(), pIndexer_->getMaxSkipLevel());
            postingMap_[iter->termId_] = curPosting;
        }
        else
            curPosting = postingIter->second;

        curPosting->add(docid, iter->offset_);
    }
}

void FieldIndexer::addField(docid_t docid, boost::shared_ptr<ForwardIndex> forwardindex)
{
    InMemoryPosting* curPosting;

    for(ForwardIndex::iterator iter = forwardindex->begin(); iter != forwardindex->end(); ++iter)
    {
        InMemoryPostingMap::iterator postingIter = postingMap_.find(iter->first);
        if(postingIter == postingMap_.end())
        {
            curPosting = new InMemoryPosting(pMemCache_, pIndexer_->getSkipInterval(), pIndexer_->getMaxSkipLevel());
            postingMap_[iter->first] = curPosting;
        }
        else
            curPosting = postingIter->second;

        ForwardIndexOffset::iterator	endit = iter->second->end();
        for(ForwardIndexOffset::iterator it = iter->second->begin(); it != endit; ++it)
            curPosting->add(docid, *it);
    }
}

void FieldIndexer::reset()
{
    InMemoryPosting* pPosting;
    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        pPosting = iter->second;
        if (!pPosting->hasNoChunk())
        {
            pPosting->reset();		///clear posting data
        }
    }
    postingMap_.clear();
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
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> lock(rwLock_);
    for(InMemoryPostingMap::iterator iter = postingMap_.begin(); iter !=postingMap_.end(); ++iter)
    {
        pPosting = iter->second;
        if (!pPosting->hasNoChunk())
        {
            tid = iter->first;
            pPosting->write(pWriterDesc, termInfo);		///write posting data
            pVocWriter->writeInt(tid);						///write term id
            pVocWriter->writeInt(termInfo.docFreq_);		///write df
            pVocWriter->writeInt(termInfo.ctf_);			///write ctf
            pVocWriter->writeInt(termInfo.lastDocID_);		///write last doc id
            pVocWriter->writeInt(termInfo.skipLevel_);		///write skip level
            pVocWriter->writeLong(termInfo.skipPointer_);	///write skip list offset offset
            pVocWriter->writeLong(termInfo.docPointer_);	///write document posting offset
            pVocWriter->writeInt(termInfo.docPostingLen_);	///write document posting length (without skiplist)
            pVocWriter->writeLong(termInfo.positionPointer_);	///write position posting offset
            pVocWriter->writeInt(termInfo.positionPostingLen_);///write position posting length

            pPosting->reset();								///clear posting data
            termInfo.reset();

            termCount++;
        }
        delete pPosting;
        pPosting = NULL;
    }

    boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(500));
    postingMap_.clear();

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
    return new InMemoryTermReader(getField(),this);
}

