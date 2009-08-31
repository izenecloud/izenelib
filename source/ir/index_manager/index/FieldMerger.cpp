#include <ir/index_manager/index/FieldMerger.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/TermReader.h>

using namespace izenelib::ir::indexmanager;

FieldMerger::FieldMerger()
        :buffer(NULL)
        ,bufsize(0)
        ,pMergeQueue(NULL)
        ,pPostingMerger(NULL)
        ,nNumTermCached(0)
        ,pDirectory(NULL)
        ,ppFieldInfos(NULL)
        ,nNumInfos(0)
        ,termCount(0)
        ,lastTerm(0)
        ,lastPOffset(0)
        ,beginOfVoc(0)
        ,nMergedTerms(0)
{
    memset(cachedTermInfos,0,NUM_CACHEDTERMINFO*sizeof(MergeTermInfo*));
}
FieldMerger::FieldMerger(Directory* pDirectory)
        :buffer(NULL)
        ,bufsize(0)
        ,pMergeQueue(NULL)
        ,pPostingMerger(NULL)
        ,nNumTermCached(0)
        ,ppFieldInfos(NULL)
        ,nNumInfos(0)
        ,termCount(0)
        ,lastTerm(0)
        ,lastPOffset(0)
        ,beginOfVoc(0)
        ,nMergedTerms(0)
{
    memset(cachedTermInfos,0,NUM_CACHEDTERMINFO*sizeof(MergeTermInfo*));
}
FieldMerger::~FieldMerger(void)
{
    buffer = NULL;
    bufsize = 0;
    vector<MergeFieldEntry*>::iterator iter = fieldEntries.begin();
    while (iter != fieldEntries.end())
    {
        delete (*iter);
        iter++;
    }
    fieldEntries.clear();

    if (pMergeQueue)
    {
        delete pMergeQueue;
        pMergeQueue = NULL;
    }
    if (pPostingMerger)
    {
        delete pPostingMerger;
        pPostingMerger = NULL;
    }

    for (int32_t i = 0;i<NUM_CACHEDTERMINFO;i++)
    {
        if (cachedTermInfos[i])
        {
            delete cachedTermInfos[i];
            cachedTermInfos[i] = NULL;
        }
    }

    if (ppFieldInfos)
    {
        delete[] ppFieldInfos;
        ppFieldInfos = NULL;
        nNumInfos = 0;
    }

}
void FieldMerger::addField(BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{
    fieldEntries.push_back(new MergeFieldEntry(pBarrelInfo,pFieldInfo));
    if (pPostingMerger == NULL)
        pPostingMerger = new PostingMerger();

}

fileoffset_t FieldMerger::merge(OutputDescriptor* pOutputDescriptor)
{
    if (initQueue() == false)///initialize merge queue
        //return 0; 
        //When collection is empty in a barrel, it still needs to write some information.
        return endMerge(pOutputDescriptor);


    pPostingMerger->setOutputDescriptor(pOutputDescriptor);

    nMergedTerms = 0;
    int64_t mergedTerms = 0;
    int32_t nMatch = 0;
    FieldMergeInfo** match = new FieldMergeInfo*[pMergeQueue->size()];
    Term* pTerm = NULL;
    FieldMergeInfo* pTop = NULL;
    fileoffset_t postingoffset = 0;

    while (pMergeQueue->size() > 0)
    {
        nMatch = 0;
        //Pop the first
        match[nMatch++] = pMergeQueue->pop();
        pTerm = match[0]->pCurTerm;
        //Get the current top of the queue
        pTop = pMergeQueue->top();
        while (pTop != NULL && (pTerm->compare(pTop->pCurTerm)==0) )
        {
            //A match has been found so add the matching FieldMergeInfo to the match array
            match[nMatch++] = pMergeQueue->pop();
            //Get the next FieldMergeInfo
            pTop = pMergeQueue->top();
        }

        postingoffset = mergeTerms(match,nMatch);

        if (postingoffset > 0)
        {
            ///store merged terms to term info cache
            if (cachedTermInfos[nNumTermCached] == NULL)
            {
                ///if the document frequency is required to be counted based on the total document, then use tdf
                //cachedTermInfos[nNumTermCached] = new MergeTermInfo(pTerm->clone(),new TermInfo(pPostingMerger->getPostingDescriptor().tdf,postingoffset));
                cachedTermInfos[nNumTermCached] = new MergeTermInfo(pTerm->clone(),new TermInfo(pPostingMerger->getPostingDescriptor().df,postingoffset));

            }
            else
            {
                cachedTermInfos[nNumTermCached]->pTerm->setValue(pTerm->getValue());

                ///if the document frequency is required to be counted based on the total document, then use tdf
                //cachedTermInfos[nNumTermCached]->pTermInfo->set(pPostingMerger->getPostingDescriptor().tdf,postingoffset);
                cachedTermInfos[nNumTermCached]->pTermInfo->set(pPostingMerger->getPostingDescriptor().df,postingoffset);
            }
            nNumTermCached++;
            if (nNumTermCached >= NUM_CACHEDTERMINFO)///cache is exhausted
            {
                flushTermInfo(pOutputDescriptor,cachedTermInfos,nNumTermCached);
                nNumTermCached = 0;
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
                pMergeQueue->put(pTop);
            }
            else
            {
                //No terms anymore
                delete pTop;
            }
        }
    }
    if (nNumTermCached > 0)///flush cache
    {
        flushTermInfo(pOutputDescriptor,cachedTermInfos,nNumTermCached);
        nNumTermCached = 0;
    }
    delete[] match;
    nMergedTerms = mergedTerms;
    return endMerge(pOutputDescriptor);///merge end here
}

void FieldMerger::setBuffer(char* buf,size_t bufSize)
{
    buffer = buf;
    bufsize = bufSize;
}

bool FieldMerger::initQueue()
{
    if (fieldEntries.size() <= 0)
        return false;
    pMergeQueue = new FieldMergeQueue(fieldEntries.size());
    TermReader* pTermReader = NULL;
    FieldMergeInfo* pMI = NULL;
    MergeFieldEntry* pEntry;

    size_t nSubBufSize = 0;
    size_t nCurBufferSize = 0;
    size_t nTotalUsed = 0;
    size_t nSubBufUsed = 0;

    ///dispatch buffer
    if ( (bufsize > 0) && (fieldEntries.size() > 0))
    {
        nSubBufSize = bufsize/fieldEntries.size();
        nCurBufferSize = nSubBufSize;
    }

    int32_t order = 0;
    nNumInfos = fieldEntries.size();
    if (nNumInfos > 0)
    {
        ppFieldInfos = new MergeFieldEntry*[fieldEntries.size()];
        memset(ppFieldInfos,0,nNumInfos * sizeof(MergeFieldEntry*));
    }
    vector<MergeFieldEntry*>::iterator iter = fieldEntries.begin();
    while (iter != fieldEntries.end())
    {
        pEntry = (*iter);
        ppFieldInfos[order] = pEntry;
        if (pEntry->pBarrelInfo->getWriter()) ///in-memory index barrel
        {
            pTermReader = pEntry->pBarrelInfo->getWriter()->getCollectionIndexer(pEntry->pFieldInfo->getColID())->getFieldIndexer(pEntry->pFieldInfo->getName())->termReader();
        }
        else
        {
            ///on-disk index barrel
            pTermReader = new DiskTermReader();
            ///open on-disk index barrel
            pTermReader->open(pDirectory,pEntry->pBarrelInfo->getName().c_str(),pEntry->pFieldInfo);
        }
        pMI = new FieldMergeInfo(order,pEntry->pBarrelInfo,pTermReader);
        if (nSubBufSize > 0)
        {
            nSubBufUsed = pMI->pIterator->setBuffer(buffer + nTotalUsed,nCurBufferSize);
            nTotalUsed += nSubBufUsed;
        }
        if (pMI->next())	///get first term
        {
            pMergeQueue->put(pMI);
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
    nNumInfos = order;

    ///buffer for posting merging
    if ( (bufsize - nTotalUsed) > POSTINGMERGE_BUFFERSIZE)
        pPostingMerger->setBuffer(buffer + nTotalUsed,bufsize - nTotalUsed);

    return (pMergeQueue->size() > 0);
}

void FieldMerger::flushTermInfo(OutputDescriptor* pOutputDescriptor,MergeTermInfo** ppTermInfo,int32_t numTermInfos)
{
    IndexOutput* pVocOutput = pOutputDescriptor->getVocOutput();

    if (termCount == 0)
        beginOfVoc = pVocOutput->getFilePointer();
    termid_t tid;
    fileoffset_t poffset;
    for (int32_t i = 0;i < numTermInfos;i++)
    {
        tid = ppTermInfo[i]->getTerm()->getValue();
        //pVocOutput->writeInt(tid - lastTerm);
        pVocOutput->writeInt(tid);
        pVocOutput->writeInt(ppTermInfo[i]->getTermInfo()->docFreq());
        poffset = ppTermInfo[i]->getTermInfo()->docPointer();
        pVocOutput->writeLong(poffset);
        //lastTerm = tid;
        //lastPOffset = poffset;
        termCount++;
    }

}

fileoffset_t FieldMerger::endMerge(OutputDescriptor* pOutputDescriptor)
{
    IndexOutput* pVocOutput = pOutputDescriptor->getVocOutput();
    fileoffset_t voffset = pVocOutput->getFilePointer();
    ///begin write vocabulary descriptor
    pVocOutput->writeLong(voffset - beginOfVoc);
    pVocOutput->writeLong(termCount);
    ///end write vocabulary descriptor
    return voffset;
}

