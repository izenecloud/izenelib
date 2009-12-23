#include <ir/index_manager/index/FieldMerger.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/MultiPostingIterator.h>

#define MEMPOOL_SIZE_FOR_MERGING    50*1024*1024

using namespace izenelib::ir::indexmanager;

FieldMerger::FieldMerger(bool sortingMerge)
        :sortingMerge_(sortingMerge)
        ,buffer_(NULL)
        ,bufsize_(0)
        ,pMergeQueue_(NULL)
        ,pPostingMerger_(NULL)
        ,nNumTermCached_(0)
        ,pDirectory_(NULL)
        ,ppFieldInfos_(NULL)
        ,nNumInfos_(0)
        ,termCount_(0)
        ,lastTerm_(0)
        ,lastPOffset_(0)
        ,beginOfVoc_(0)
        ,nMergedTerms_(0)
        ,pDocFilter_(0)
        ,pMemCache_(0)
{
    memset(cachedTermInfos_,0,NUM_CACHEDTERMINFO*sizeof(MergeTermInfo*));
}

FieldMerger::~FieldMerger(void)
{
    buffer_ = NULL;
    bufsize_ = 0;
    vector<MergeFieldEntry*>::iterator iter = fieldEntries_.begin();
    while (iter != fieldEntries_.end())
    {
        delete (*iter);
        iter++;
    }
    fieldEntries_.clear();

    if (pMergeQueue_)
    {
        delete pMergeQueue_;
        pMergeQueue_ = NULL;
    }
    if (pPostingMerger_)
    {
        delete pPostingMerger_;
        pPostingMerger_ = NULL;
    }

    for (int32_t i = 0;i<NUM_CACHEDTERMINFO;i++)
    {
        if (cachedTermInfos_[i])
        {
            delete cachedTermInfos_[i];
            cachedTermInfos_[i] = NULL;
        }
    }

    if (ppFieldInfos_)
    {
        delete[] ppFieldInfos_;
        ppFieldInfos_ = NULL;
        nNumInfos_ = 0;
    }
    pDocFilter_ = 0;
    if(pMemCache_)
    {
        delete pMemCache_;
        pMemCache_ = NULL;
    }
}
void FieldMerger::addField(BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{
    fieldEntries_.push_back(new MergeFieldEntry(pBarrelInfo,pFieldInfo));
    if (pPostingMerger_ == NULL)
        pPostingMerger_ = new PostingMerger();

}

fileoffset_t FieldMerger::merge(OutputDescriptor* pOutputDescriptor)
{
    if (initQueue() == false)///initialize merge queue
        //return 0; 
        //When collection is empty in a barrel, it still needs to write some information.
        return endMerge(pOutputDescriptor);


    pPostingMerger_->setOutputDescriptor(pOutputDescriptor);

    nMergedTerms_ = 0;
    int64_t mergedTerms = 0;
    int32_t nMatch = 0;
    FieldMergeInfo** match = new FieldMergeInfo*[pMergeQueue_->size()];
    Term* pTerm = NULL;
    FieldMergeInfo* pTop = NULL;
    fileoffset_t postingoffset = 0;
    freq_t sortingMergeDF = 0;
    while (pMergeQueue_->size() > 0)
    {
        nMatch = 0;
        //Pop the first
        match[nMatch++] = pMergeQueue_->pop();
        pTerm = match[0]->pCurTerm_;
        //Get the current top of the queue
        pTop = pMergeQueue_->top();
        while (pTop != NULL && (pTerm->compare(pTop->pCurTerm_)==0) )
        {
            //A match has been found so add the matching FieldMergeInfo to the match array
            match[nMatch++] = pMergeQueue_->pop();
            //Get the next FieldMergeInfo
            pTop = pMergeQueue_->top();
        }

        postingoffset = mergeTerms(match,nMatch,sortingMergeDF);

        if (postingoffset > 0)
        {
            ///store merged terms to term info cache
            if (cachedTermInfos_[nNumTermCached_] == NULL)
            {
                cachedTermInfos_[nNumTermCached_] = new MergeTermInfo(pTerm->clone(),new TermInfo(pPostingMerger_->getPostingDescriptor().df,postingoffset));

            }
            else
            {
                cachedTermInfos_[nNumTermCached_]->pTerm_->setValue(pTerm->getValue());
                if(sortingMerge_)
                    cachedTermInfos_[nNumTermCached_]->pTermInfo_->set(sortingMergeDF,postingoffset);					
                else
                    cachedTermInfos_[nNumTermCached_]->pTermInfo_->set(pPostingMerger_->getPostingDescriptor().df,postingoffset);
            }
            nNumTermCached_++;
            if (nNumTermCached_ >= NUM_CACHEDTERMINFO)///cache is exhausted
            {
                flushTermInfo(pOutputDescriptor,cachedTermInfos_,nNumTermCached_);
                nNumTermCached_ = 0;
            }
            mergedTerms++;
        }

        while (nMatch > 0)
        {
            pTop = match[--nMatch];

            //Move to the next term i
            if (pTop->next())
            {
                //There still are some terms so restore it in the queue
                pMergeQueue_->put(pTop);
            }
            else
            {
                //No terms anymore
                delete pTop;
            }
        }
    }
    if (nNumTermCached_ > 0)///flush cache
    {
        flushTermInfo(pOutputDescriptor,cachedTermInfos_,nNumTermCached_);
        nNumTermCached_ = 0;
    }
    delete[] match;
    nMergedTerms_ = mergedTerms;
    return endMerge(pOutputDescriptor);///merge end here
}

void FieldMerger::setBuffer(char* buf,size_t bufSize)
{
    buffer_ = buf;
    bufsize_ = bufSize;
}

bool FieldMerger::initQueue()
{
    if (fieldEntries_.size() <= 0)
        return false;
    pMergeQueue_ = new FieldMergeQueue(fieldEntries_.size());
    TermReader* pTermReader = NULL;
    FieldMergeInfo* pMI = NULL;
    MergeFieldEntry* pEntry;

    size_t nSubBufSize = 0;
    size_t nCurBufferSize = 0;
    size_t nTotalUsed = 0;
    size_t nSubBufUsed = 0;

    ///dispatch buffer
    if ( (bufsize_ > 0) && (fieldEntries_.size() > 0))
    {
        nSubBufSize = bufsize_/fieldEntries_.size();
        nCurBufferSize = nSubBufSize;
    }

    int32_t order = 0;
    nNumInfos_ = fieldEntries_.size();
    if (nNumInfos_ > 0)
    {
        ppFieldInfos_ = new MergeFieldEntry*[fieldEntries_.size()];
        memset(ppFieldInfos_,0,nNumInfos_ * sizeof(MergeFieldEntry*));
    }
    vector<MergeFieldEntry*>::iterator iter = fieldEntries_.begin();
    while (iter != fieldEntries_.end())
    {
        pEntry = (*iter);
        ppFieldInfos_[order] = pEntry;
        if (pEntry->pBarrelInfo_->getWriter()) ///in-memory index barrel
        {
            pTermReader = pEntry->pBarrelInfo_->getWriter()->getCollectionIndexer(pEntry->pFieldInfo_->getColID())
				->getFieldIndexer(pEntry->pFieldInfo_->getName())->termReader();
        }
        else
        {
            ///on-disk index barrel
            pTermReader = new DiskTermReader(pDirectory_,pEntry->pBarrelInfo_->getName().c_str(),pEntry->pFieldInfo_);
        }
        pMI = new FieldMergeInfo(order,pEntry->pFieldInfo_->getColID(),pEntry->pBarrelInfo_,pTermReader);
        if (nSubBufSize > 0)
        {
            nSubBufUsed = pMI->pIterator_->setBuffer(buffer_ + nTotalUsed,nCurBufferSize);
            nTotalUsed += nSubBufUsed;
        }
        if (pMI->next())	///get first term
        {
            pMergeQueue_->put(pMI);
            nCurBufferSize = nSubBufSize + (nCurBufferSize - nSubBufUsed);
            order++;
        }
        else
        {
            nTotalUsed -= nSubBufUsed;
            delete pMI;
        }
        iter++;
    }
    nNumInfos_ = order;

    ///buffer for posting merging
    if ( (bufsize_ - nTotalUsed) > POSTINGMERGE_BUFFERSIZE)
        pPostingMerger_->setBuffer(buffer_ + nTotalUsed,bufsize_ - nTotalUsed);

    return (pMergeQueue_->size() > 0);
}

void FieldMerger::flushTermInfo(OutputDescriptor* pOutputDescriptor,MergeTermInfo** ppTermInfo,int32_t numTermInfos)
{
    IndexOutput* pVocOutput = pOutputDescriptor->getVocOutput();

    if (termCount_ == 0)
        beginOfVoc_ = pVocOutput->getFilePointer();
    termid_t tid;
    fileoffset_t poffset;
    for (int32_t i = 0;i < numTermInfos;i++)
    {
        tid = ppTermInfo[i]->getTerm()->getValue();
        //pVocOutput->writeInt(tid - lastTerm_);
        pVocOutput->writeInt(tid);
        pVocOutput->writeInt(ppTermInfo[i]->getTermInfo()->docFreq());
        poffset = ppTermInfo[i]->getTermInfo()->docPointer();
        pVocOutput->writeLong(poffset);
        //lastTerm_ = tid;
        //lastPOffset_ = poffset;
        termCount_++;
    }

}

fileoffset_t FieldMerger::endMerge(OutputDescriptor* pOutputDescriptor)
{
    IndexOutput* pVocOutput = pOutputDescriptor->getVocOutput();
    fileoffset_t voffset = pVocOutput->getFilePointer();
    ///begin write vocabulary descriptor
    pVocOutput->writeLong(voffset - beginOfVoc_);
    pVocOutput->writeLong(termCount_);
    ///end write vocabulary descriptor
    return voffset;
}

fileoffset_t FieldMerger::sortingMerge(FieldMergeInfo** ppMergeInfos,int32_t numInfos, BitVector* pFilter, freq_t& df)
{
    if(!pMemCache_)
        pMemCache_ = new MemCache(MEMPOOL_SIZE_FOR_MERGING);
    MultiPostingIterator postingIterator(numInfos);

    for (int32_t i = 0;i< numInfos;i++)
    {
        TermPositions* pPosition = new TermPositions(ppMergeInfos[i]->pIterator_->termPosting());
        if(ppMergeInfos[i]->pBarrelInfo_->hasUpdateDocs)
            postingIterator.addTermPosition(pPosition);
        else
            postingIterator.addTermPosition(pPosition, pFilter);
    }
    InMemoryPosting* newPosting = new InMemoryPosting(pMemCache_);

    docid_t docId = 0;
    while(postingIterator.next())
    {
        docId = postingIterator.doc();
        loc_t pos = postingIterator.nextPosition();
        while (pos != BAD_POSITION)
        {
            newPosting->addLocation(docId, pos);
            pos = postingIterator.nextPosition();
        }
    }
    fileoffset_t offset = newPosting->write(pPostingMerger_->getOutputDescriptor());
    df = newPosting->docFreq();
    delete newPosting;
    pMemCache_->flushMem();
    return offset;
}


