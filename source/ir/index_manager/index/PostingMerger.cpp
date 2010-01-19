#include <ir/index_manager/index/PostingMerger.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

using namespace izenelib::ir::indexmanager;

PostingMerger::PostingMerger()
        :buffer_(NULL)
        ,bufsize_(0)
        ,bOwnBuffer_(false)
        ,pOutputDescriptor_(NULL)
        ,nPPostingLength_(0)
        ,bFirstPosting_(true)
        ,pSkipListMerger_(NULL)
        ,pMemCache_(NULL)
{
    reset();
}

PostingMerger::PostingMerger(OutputDescriptor* pOutputDescriptor)
        :buffer_(NULL)
        ,bufsize_(0)
        ,bOwnBuffer_(false)
        ,pOutputDescriptor_(pOutputDescriptor)
        ,nPPostingLength_(0)
        ,bFirstPosting_(true)
        ,pSkipListMerger_(NULL)
        ,pMemCache_(NULL)
{
    reset();
}

PostingMerger::~PostingMerger()
{
    if (buffer_ && bOwnBuffer_)
    {
        delete buffer_;
        buffer_ = NULL;
    }
    bufsize_ = 0;
    if(pMemCache_)
        delete pMemCache_;
    if(pSkipListMerger_)
        delete pSkipListMerger_;
}

inline void PostingMerger::reset()
{
    ///reset posting descriptor
    postingDesc_.length = 0;
    postingDesc_.ctf = 0;
    postingDesc_.df = 0;
    postingDesc_.poffset = -1;
    chunkDesc_.lastdocid = 0;
    chunkDesc_.length = 0;
    if(pSkipListMerger_)
        pSkipListMerger_->reset();
}

void PostingMerger::setBuffer(char* buf,size_t bufSize)
{
    buffer_ = buf;
    bufsize_ = bufSize;
}
void PostingMerger::createBuffer()
{
    buffer_ = new char[POSTINGMERGE_BUFFERSIZE];
    bufsize_ = POSTINGMERGE_BUFFERSIZE;
    bOwnBuffer_ = true;
    pMemCache_ = new MemCache(POSTINGMERGE_BUFFERSIZE);
}

void PostingMerger::mergeWith(InMemoryPosting* pInMemoryPosting)
{
    ///flush last doc
    pInMemoryPosting->flushLastDoc(true);

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting_)///first posting
    {
        reset();
        nPPostingLength_ = 0;
        bFirstPosting_ = false;
        ///save position posting offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }
    ///write chunk data, update the first doc id
    VariantDataChunk* pDChunk = pInMemoryPosting->pDocFreqList_->pHeadChunk_;
    if (pDChunk)
    {
        uint8_t* bp = &(pDChunk->data[0]);
        docid_t firstDocID = VariantDataPool::decodeVData32(bp) - chunkDesc_.lastdocid;
		
        pDOutput->writeVInt(firstDocID);///write first doc id
        int32_t writeSize = pDChunk->size - (bp - &(pDChunk->data[0]));	//write the rest data of first chunk
        if (writeSize > 0)
            pDOutput->write((const char*)bp,writeSize);
        pDChunk = pDChunk->next;
    }
    ///write rest data
    while (pDChunk)
    {
        pDOutput->write((const char*)pDChunk->data,pDChunk->size);
        pDChunk = pDChunk->next;
    }

    chunkDesc_.length += (pDOutput->getFilePointer() - oldDOff);

    ///write position posting
    VariantDataChunk* pPChunk = pInMemoryPosting->pLocList_->pHeadChunk_;
    while (pPChunk)
    {
        pPOutput->write((const char*)pPChunk->data,pPChunk->size);
        pPChunk = pPChunk->next;
    }

    ///merge skiplist
    SkipListReader* pSkipReader = pInMemoryPosting->getSkipListReader();
    fileoffset_t baseDOffset,basePOffset;
    baseDOffset = postingDesc_.length;
    basePOffset = postingDesc_.poffset;

    if(pSkipReader)
    {
        if(!pSkipListMerger_)
        {
            if(!pMemCache_)
                createBuffer();
            pSkipListMerger_ = new SkipListMerger(pInMemoryPosting->skipInterval_,pInMemoryPosting->maxSkipLevel_,pMemCache_);
        }
        pSkipListMerger_->setBasePoint(0,baseDOffset,basePOffset);
        pSkipListMerger_->addToMerge(pSkipReader,pInMemoryPosting->lastDocID());
    }

    ///update descriptors
    postingDesc_.ctf += pInMemoryPosting->nCTF_;
    postingDesc_.df += pInMemoryPosting->nDF_;
    postingDesc_.length = chunkDesc_.length;

    chunkDesc_.lastdocid = pInMemoryPosting->nLastDocID_;
}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting)
{
    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting_)///first posting
    {
        reset();
        nPPostingLength_ = 0;
        bFirstPosting_ = false;

        ///save position offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }

    fileoffset_t baseDOffset,basePOffset;

    docid_t firstDocID = pDInput->readVInt() - chunkDesc_.lastdocid;

    pDOutput->writeVInt(firstDocID);///write first doc id
    int64_t writeSize = pOnDiskPosting->postingDesc_.length - pDOutput->getVIntLength(firstDocID + chunkDesc_.lastdocid);
    if (writeSize > 0)
        pDOutput->write(pDInput,writeSize);

    chunkDesc_.length += (pDOutput->getFilePointer() - oldDOff);

    ///write position posting
    pPOutput->write(pPInput,pOnDiskPosting->nPPostingLength_);

    ///merge skiplist
    SkipListReader* pSkipReader = pOnDiskPosting->getSkipListReader();

    baseDOffset = postingDesc_.length;
    basePOffset = postingDesc_.poffset;

    if(pSkipReader)
    {
        if(!pSkipListMerger_)
        {
            if(!pMemCache_)
                createBuffer();
            pSkipListMerger_ = new SkipListMerger(pOnDiskPosting->skipInterval_,pOnDiskPosting->maxSkipLevel_,pMemCache_);
        }
        pSkipListMerger_->setBasePoint(0,baseDOffset,basePOffset);
        pSkipListMerger_->addToMerge(pSkipReader,pOnDiskPosting->chunkDesc_.lastdocid);
    }

    ///update descriptors
    postingDesc_.ctf += pOnDiskPosting->postingDesc_.ctf;
    postingDesc_.df += pOnDiskPosting->postingDesc_.df;
    postingDesc_.length = chunkDesc_.length; ///currently,it's only one chunk
    chunkDesc_.lastdocid = pOnDiskPosting->chunkDesc_.lastdocid;

}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting,BitVector* pFilter)
{
    if(pFilter &&  pFilter->hasSmallThan((size_t)pOnDiskPosting->chunkDesc_.lastdocid))
        mergeWith_GC(pOnDiskPosting,pFilter);
    else
        mergeWith(pOnDiskPosting);
}

void PostingMerger::mergeWith_GC(OnDiskPosting* pOnDiskPosting,BitVector* pFilter)
{
    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    docid_t nDocID = 0;
    docid_t nDocIDPrev = 0;
    docid_t nLastDocID = 0;
    freq_t	nTF = 0;
    count_t nCTF = 0;
    count_t nDF = 0;
    count_t nPCount = 0;
    count_t nODDF = pOnDiskPosting->postingDesc_.df;
    if(nODDF <= 0)
        return;

    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting_)///first posting
    {
        reset();
        nPPostingLength_ = 0;
        bFirstPosting_ = false;

        ///save position offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }

    ///merge skiplist
    if(!pSkipListMerger_)
    {
        if(!pMemCache_)
            createBuffer();
        pSkipListMerger_ = new SkipListMerger(pOnDiskPosting->skipInterval_,pOnDiskPosting->maxSkipLevel_,pMemCache_);
    }

    while (nODDF > 0)
    {
        nDocID += pDInput->readVInt();
        nTF = pDInput->readVInt();
        if(!pFilter->test((size_t)nDocID))///the document has not been deleted
        {
            pDOutput->writeVInt(nDocID - nDocIDPrev);
            pDOutput->writeVInt(nTF);

            nDocIDPrev = nDocID;
            nPCount += nTF;
            nDF++;
            nLastDocID = nDocID;
            if(pSkipListMerger_)
                pSkipListMerger_->addSkipPoint(nLastDocID,pDOutput->getFilePointer(),pPOutput->getFilePointer());
        }
        else ///this document has been deleted
        {					
            nCTF += nPCount;
            ///write positions of documents
            while (nPCount > 0)
            {							
                pPOutput->writeVInt(pPInput->readVInt());
                nPCount--;
            }
            ///skip positions of deleted documents
            while (nTF > 0)
            {		
                pPInput->readVInt();
                nTF--;							
            }
        }
        nODDF--;
    }	
    if(nPCount > 0)
    {
        nCTF += nPCount;
        while (nPCount > 0)
        {							
            pPOutput->writeVInt(pPInput->readVInt());
            nPCount--;
        }
    }			

    chunkDesc_.length += (pDOutput->getFilePointer() - oldDOff);

    ///update descriptors
    postingDesc_.ctf += nCTF;
    postingDesc_.df += nDF;
    postingDesc_.length = chunkDesc_.length; ///currently,it's only one chunk 			
    chunkDesc_.lastdocid = nLastDocID;
}

fileoffset_t PostingMerger::endMerge()
{
    bFirstPosting_ = true;
    if (postingDesc_.df <= 0)
        return -1;

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    fileoffset_t postingoffset = pDOutput->getFilePointer();

    ///write position posting descriptor
    nPPostingLength_ = pPOutput->getFilePointer() - postingDesc_.poffset;
    postingDesc_.poffset = pPOutput->getFilePointer();
    pPOutput->writeVLong(nPPostingLength_);	////<ChunkLength(VInt64)>

    ///begin write posting descriptor
    pDOutput->writeVLong(postingDesc_.length);	///<PostingLength(VInt64)>
    pDOutput->writeVInt(postingDesc_.df);		///<DF(VInt32)>
    pDOutput->writeVLong(postingDesc_.ctf);	///<CTF(VInt64)>
    pDOutput->writeVLong(postingDesc_.poffset);///<PositionPointer(VInt64)>
    ///end write posting descriptor

    pDOutput->writeVInt(1);						///<ChunkCount(VInt32)>
    ///begin write chunk descriptor
    pDOutput->writeVLong(chunkDesc_.length);	///<ChunkLength(VInt64)>

    if( pSkipListMerger_ && pSkipListMerger_->getNumLevels() > 0) ///nDF_ > SkipInterval
    {
        pDOutput->writeVInt(chunkDesc_.lastdocid);///<LastDocID(VInt32)>
        pDOutput->writeVInt(pSkipListMerger_->getNumLevels()); ///skiplevel (VInt32)
        pSkipListMerger_->write(pDOutput);	///write skip list data
    }
    else
    {
        pDOutput->writeVInt(chunkDesc_.lastdocid);///<LastDocID(VInt32)>
        pDOutput->writeVInt(0);  /// skiplevel = 0
    }
    
    ///end write posting descriptor

    return postingoffset;
}

