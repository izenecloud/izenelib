#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/SkipListWriter.h>
#include <ir/index_manager/index/SkipListReader.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

#include <math.h>


using namespace std;

using namespace izenelib::ir::indexmanager;

Posting::Posting(int skipInterval, int maxSkipLevel)
    :skipInterval_(skipInterval)
    ,maxSkipLevel_(maxSkipLevel)
    ,pDocFilter_(0)
{}

Posting::~Posting()
{}

//////////////////////////////////////////////////////////////////////////
///InMemoryPosting

InMemoryPosting::InMemoryPosting(MemCache* pCache, int skipInterval, int maxSkipLevel)
        :Posting(skipInterval, maxSkipLevel)
        ,pMemCache_(pCache)
        ,nDF_(0)
        ,nLastDocID_(BAD_DOCID)
        ,nLastLoc_(BAD_DOCID)
        ,nCurTermFreq_(0)
        ,nCTF_(0)
        ,pDS_(NULL)
        ,pSkipListWriter_(0)
        ,pSkipListReader_(0)
        ,dirty_(false)
{
    pDocFreqList_ = new VariantDataPool(pCache);
    pLocList_  = new VariantDataPool(pCache);
}

InMemoryPosting::InMemoryPosting(int skipInterval, int maxSkipLevel)
        :Posting(skipInterval, maxSkipLevel)
        ,pMemCache_(NULL)
        ,nDF_(0)
        ,nLastDocID_(BAD_DOCID)
        ,nLastLoc_(BAD_DOCID)
        ,nCurTermFreq_(0)
        ,nCTF_(0)
        ,pDS_(NULL)
        ,pDocFreqList_(NULL)
        ,pLocList_(NULL)
        ,pSkipListWriter_(0)
        ,pSkipListReader_(0)
        ,dirty_(false)
{
}

InMemoryPosting::~InMemoryPosting()
{
    if (pDocFreqList_)
    {
        delete pDocFreqList_;
        pDocFreqList_ = NULL;
    }
    if (pLocList_)
    {
        delete pLocList_;
        pLocList_ = NULL;
    }
    pMemCache_ = NULL;

    if (pDS_)
    {
        delete pDS_;
        pDS_ = NULL;
    }

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;
    }
    if(pSkipListWriter_)
    {
        delete pSkipListWriter_;
        pSkipListWriter_ = 0;
    }
}

int32_t InMemoryPosting::getSkipLevel()
{
    if( pSkipListWriter_ && pSkipListWriter_->getNumLevels() > 0)
        return pSkipListWriter_->getNumLevels();
    else
        return 0;
}

count_t InMemoryPosting::getDPostingLen()
{
    flushLastDoc(true);
    return pDocFreqList_->getLength();
}

count_t InMemoryPosting::getPPostingLen()
{
    flushLastDoc(true);
    return pLocList_->getLength();
}

void InMemoryPosting::write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo)
{
    ///flush last document
    flushLastDoc(true);

    termInfo.docFreq_ = nDF_;

    termInfo.ctf_ = nCTF_;

    termInfo.lastDocID_ = nLastDocID_;

    if(skipInterval_ && nDF_ > 0 && nDF_ % skipInterval_ == 0)
    {
        if(pSkipListWriter_)
        {
            pSkipListWriter_->addSkipPoint(nLastDocID_,pDocFreqList_->getLength(),pLocList_->getLength());
        }
    }

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();

    if( pSkipListWriter_ && pSkipListWriter_->getNumLevels() > 0) ///nDF_ > SkipInterval
    {
        termInfo.skipLevel_ = pSkipListWriter_->getNumLevels();
        termInfo.skipPointer_ = pDOutput->getFilePointer();
        pSkipListWriter_->write(pDOutput);	///write skip list data
    }
    else
    {
        termInfo.skipPointer_ = -1;
        termInfo.skipLevel_ = 0;
    }

    ///save the offset of posting descriptor
    termInfo.docPointer_ = pDOutput->getFilePointer();

    ///write doc posting data
    pDocFreqList_->write(pDOutput);
	
    termInfo.docPostingLen_ = pDOutput->getFilePointer() - termInfo.docPointer_;

    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();

    termInfo.positionPointer_ = pPOutput->getFilePointer();

    ///write position posting data
    pLocList_->write(pPOutput);

    termInfo.positionPostingLen_ = pPOutput->getFilePointer() - termInfo.positionPointer_;
}

void InMemoryPosting::reset()
{
    pDocFreqList_->reset();
    pLocList_->reset();

    nCTF_ = 0;
    nLastDocID_ = BAD_DOCID;
    nLastLoc_ = 0;
    nDF_ = 0;
    nCurTermFreq_ = 0;

    if (pDS_)
    {
        delete pDS_;
        pDS_ = NULL;
    }
    if(pSkipListWriter_)
        pSkipListWriter_ ->reset();
}

Posting* InMemoryPosting::clone()
{
    InMemoryPosting* pClone = new InMemoryPosting(skipInterval_, maxSkipLevel_);
    if (pDocFreqList_)
    {
        pClone->pDocFreqList_ = new VariantDataPool(*pDocFreqList_);
    }
    if (pLocList_)
    {
        pClone->pLocList_ = new VariantDataPool(*pLocList_);
    }
    pClone->nCTF_ = nCTF_;
    pClone->nDF_ = nDF_;
    pClone->nCurTermFreq_ = nCurTermFreq_;
    pClone->nLastDocID_ = nLastDocID_;
    pClone->nLastLoc_ = nLastLoc_;
    return pClone;
}


//////////////////////////////////////////////////////////////////////////
///InMemoryPosting
void InMemoryPosting::add(docid_t docid, loc_t location)
{
    if (docid == nLastDocID_)
    {
        ///see it before,only position is needed
        pLocList_->addVData32(location - nLastLoc_);
        nCurTermFreq_++;
        nLastLoc_ = location;
    }
    else///first see it
    {
        if (nCurTermFreq_ > 0)///write previous document's term freq
        {
            pDocFreqList_->addVData32(nCurTermFreq_);
        }
        else if (nLastDocID_ == BAD_DOCID)///first see it
        {
            nLastDocID_ = 0;
        }

        if(skipInterval_ && nDF_ > 0 && nDF_ % skipInterval_ == 0)
        {
            if(!pSkipListWriter_)
                pSkipListWriter_ = new SkipListWriter(skipInterval_,maxSkipLevel_,pMemCache_);

            pSkipListWriter_->addSkipPoint(nLastDocID_,pDocFreqList_->getLength(),pLocList_->getLength());
        }

        pDocFreqList_->addVData32(docid - nLastDocID_);
        pLocList_->addVData32(location);

        nCTF_ += nCurTermFreq_;
        nCurTermFreq_ = 1;

        nLastDocID_ = docid;
        nLastLoc_ = location;

        nDF_++;
    }
}

void InMemoryPosting::flushLastDoc(bool bTruncTail)
{
    if(!pMemCache_)
        return;

    if (nCurTermFreq_ > 0)
    {
        pDocFreqList_->addVData32(nCurTermFreq_);
        if (bTruncTail)
        {
            pDocFreqList_->truncTailChunk();///update real size
            pLocList_->truncTailChunk();///update real size
        }
        nCTF_ += nCurTermFreq_;
        nCurTermFreq_ = 0;
    }
    else if (bTruncTail)
    {
        pDocFreqList_->truncTailChunk();///update real size
        pLocList_->truncTailChunk();///update real size
    }
}

#define ISCHUNKOVER_D()\
        if(pDChunk > pDChunkEnd)\
        {\
            pDS_->decodingDChunk = pDS_->decodingDChunk->next;\
            if(!pDS_->decodingDChunk)\
                break;\
            pDS_->decodingDChunkPos = 0;\
            pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);\
            pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);\
        }

int32_t InMemoryPosting::decodeNext(uint32_t* pPosting,int32_t length)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }
    ///flush last document
    flushLastDoc(false);
    if (!pDS_)
    {
        pDS_ = new InMemoryPosting::DecodeState;
        pDS_->decodingDChunk = pDocFreqList_->pHeadChunk_;
        pDS_->decodingDChunkPos = 0;
        pDS_->lastDecodedDocID = 0;
        pDS_->decodedDocCount = 0;
        pDS_->decodingPChunk = pLocList_->pHeadChunk_;
        pDS_->decodingPChunkPos = 0;
        pDS_->lastDecodedPos = 0;
        pDS_->decodedPosCount = 0;
    }

    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length >> 1);

    int32_t left = nDF_ - pDS_->decodedDocCount;
    if (left <= 0)
        return -1;
    if (length > left*2)
        length = left*2;
    left = (length>>1);

    uint8_t* pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);

    int32_t count = 0;
    docid_t did = pDS_->lastDecodedDocID;
    while (count < left)
    {
        ISCHUNKOVER_D();
        did += VariantDataPool::decodeVData32(pDChunk);

        if(!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;
			
            ISCHUNKOVER_D();
		
            *pFreq++ = VariantDataPool::decodeVData32(pDChunk);					
        }
        else
        {
            ///this doc is deleted
            ISCHUNKOVER_D();
            VariantDataPool::decodeVData32(pDChunk);
        }

        count++;
    }
    ///update state
    pDS_->decodedDocCount += count;
    pDS_->lastDecodedDocID = did;
    pDS_->decodingDChunkPos = (int32_t)(pDChunk - pDS_->decodingDChunk->data);

    return (int32_t)(pDoc - pPosting);
}

void InMemoryPosting::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }

    uint8_t* pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS_->lastDecodedPos;
    int32_t  nDecoded = 0;
    while (nDecoded < length)
    {
        if (pPChunk > pPChunkEnd)
        {
            pDS_->decodingPChunk = pDS_->decodingPChunk->next;
            if (!pDS_->decodingPChunk)
                break;
            pDS_->decodingPChunkPos = 0;
            pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
            pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);
        }

        loc += VariantDataPool::decodeVData32(pPChunk);
        if (pPos)
        {
            *pPos = loc;
            pPos++;
        }
        nDecoded++;
    }
    pDS_->decodedPosCount += nDecoded;
    pDS_->lastDecodedPos = loc;
}

void InMemoryPosting::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }

    uint8_t* pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS_->lastDecodedPos;
    uint32_t  nTotalDecoded = 0;
    uint32_t  nCurDecoded;
    for (int32_t nF = 0; nF < nFreqs;nF++)
    {
        nCurDecoded = 0;
        while (nCurDecoded < pFreqs[nF])
        {
            if (pPChunk > pPChunkEnd)
            {
                pDS_->decodingPChunk = pDS_->decodingPChunk->next;
                if (!pDS_->decodingPChunk)
                    break;
                pDS_->decodingPChunkPos = 0;
                pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
                pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);
            }

            loc += VariantDataPool::decodeVData32(pPChunk);
            if (pPos)
            {
                *pPos = loc;
                pPos++;
            }
            nCurDecoded++;
        }
        pDS_->lastDecodedPos = loc = 0;
        nTotalDecoded += nCurDecoded;
    }

    pDS_->decodedPosCount += nTotalDecoded;
    pDS_->lastDecodedPos = loc;
}

SkipListReader* InMemoryPosting::getSkipListReader()
{
    if(!pSkipListReader_)
        if(pSkipListWriter_)
        pSkipListReader_ = pSkipListWriter_->getSkipListReader();
    return pSkipListReader_;
}

void InMemoryPosting::resetPosition()
{
    pDS_->lastDecodedPos = 0;
}

docid_t InMemoryPosting::decodeTo(docid_t docID)
{
    if(dirty_)
    {
        SF1V5_THROW(ERROR_FILEIO,"Index dirty.");
    }

    ///skipping for in-memory posting is not that necessary
    ///just pass one by one

    ///flush last document
    flushLastDoc(false);
    if (!pDS_)
    {
        pDS_ = new InMemoryPosting::DecodeState;
        pDS_->decodingDChunk = pDocFreqList_->pHeadChunk_;
        pDS_->decodingDChunkPos = 0;
        pDS_->lastDecodedDocID = 0;
        pDS_->decodedDocCount = 0;
        pDS_->decodingPChunk = pLocList_->pHeadChunk_;
        pDS_->decodingPChunkPos = 0;
        pDS_->lastDecodedPos = 0;
        pDS_->decodedPosCount = 0;
    }


    int32_t left = nDF_ - pDS_->decodedDocCount;

    uint8_t* pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);

    int32_t count = 0;
    docid_t did = pDS_->lastDecodedDocID;
    count_t nSkipPCount = 0;
    while (count < left)
    {
        if(did >= docID)
            break;
        ISCHUNKOVER_D();
        did += VariantDataPool::decodeVData32(pDChunk);

        ISCHUNKOVER_D();

        nSkipPCount += VariantDataPool::decodeVData32(pDChunk);

        count++;
    }
    ///update state
    pDS_->decodedDocCount += count;
    pDS_->lastDecodedDocID = did;
    pDS_->decodingDChunkPos = (int32_t)(pDChunk - pDS_->decodingDChunk->data);

    uint8_t* pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

    loc_t loc = pDS_->lastDecodedPos;
    count_t  nDecoded = 0;
    while (nDecoded < nSkipPCount)
    {
        if (pPChunk > pPChunkEnd)
        {
            pDS_->decodingPChunk = pDS_->decodingPChunk->next;
            if (!pDS_->decodingPChunk)
                break;
            pDS_->decodingPChunkPos = 0;
            pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
            pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);
        }

        loc += VariantDataPool::decodeVData32(pPChunk);
        nDecoded++;
    }
    pDS_->decodedPosCount += nDecoded;
    pDS_->lastDecodedPos = loc;

    return ( did >= docID )? did : -1;
}

//////////////////////////////////////////////////////////////////////////
///OnDiskPosting
OnDiskPosting::OnDiskPosting(int skipInterval, int maxSkipLevel, 
                                                           InputDescriptor* pInputDescriptor,const TermInfo& termInfo)
        :Posting(skipInterval, maxSkipLevel)
        ,pInputDescriptor_(pInputDescriptor)
        ,nBufSize_(0)
        ,pSkipListReader_(0)
{
    reset(termInfo);
}

OnDiskPosting::OnDiskPosting(int skipInterval, int maxSkipLevel)
        :Posting(skipInterval, maxSkipLevel)
        ,pSkipListReader_(0)

{
    reset();
}

OnDiskPosting::~OnDiskPosting()
{
    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;
    }
}

Posting* OnDiskPosting::clone()
{
    return NULL;
}

void OnDiskPosting::reset(const TermInfo& termInfo)
{
    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    postingDesc_.length = termInfo.docPostingLen_;	///<PostingLength(VInt64)>
    postingDesc_.df = termInfo.docFreq_;			///<DF(VInt32)>
    postingDesc_.ctf = termInfo.ctf_;				///<CTF(VInt64)>
    postingDesc_.poffset = termInfo.positionPointer_;	///PositionPointer(VInt64)
    postingDesc_.plength = termInfo.positionPostingLen_;

    chunkDesc_.length = termInfo.docPostingLen_;	///<ChunkLength(VInt64)>
    chunkDesc_.lastdocid = termInfo.lastDocID_;		///<LastDocID(VInt32)>

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    if(termInfo.skipLevel_ > 0)
    {
        if(termInfo.skipPointer_ != -1)
        {
            pDPInput->seek(termInfo.skipPointer_);
            pSkipListReader_ = new SkipListReader(pDPInput, skipInterval_, termInfo.skipLevel_);
        }
    }

    pDPInput->seek(postingOffset_);

    IndexInput* pPPInput = pInputDescriptor_->getPPostingInput();
    if (pPPInput)
    {
        pPPInput->reset();
        pPPInput->seek(termInfo.positionPointer_);
        nPPostingLength_ = termInfo.positionPostingLen_;
    }
    else
    {
        nPPostingLength_ = 0;
    }

    ds_.decodedDocCount = 0;
    ds_.lastDecodedDocID = 0;
    ds_.decodedPosCount= 0;
    ds_.lastDecodedPos = 0;
    ds_.lastDecodedDocTF = 0;
    ds_.skipPosCount_ = 0;
}

docid_t OnDiskPosting::decodeTo(docid_t target)
{
    if((count_t)(ds_.decodedDocCount) >= postingDesc_.df)		
        return -1;
    if(pSkipListReader_)
    {
        docid_t lastDocID = pSkipListReader_->skipTo(target);
        if(lastDocID > ds_.lastDecodedDocID)
        {
            seekTo(pSkipListReader_);
        }
    }

    docid_t nDocID = ds_.lastDecodedDocID;
    count_t nFreq = 0;
    count_t nSkipPCount = 0;
    count_t nDF = postingDesc_.df;
    count_t nDecodedCount = ds_.decodedDocCount;

    IndexInput* pDPostingInput = getInputDescriptor()->getDPostingInput();
    while ( nDecodedCount < nDF )
    {
        nDocID += pDPostingInput->readVInt();
        nDecodedCount++;
        if(nDocID >= target)
        {
            nFreq = pDPostingInput->readVInt();
            break;
        }
        else 
            nSkipPCount += pDPostingInput->readVInt();
    };
    ///update state
    ds_.lastDecodedDocID = nDocID;
    ds_.lastDecodedDocTF = nFreq;
    ds_.decodedDocCount = nDecodedCount;
    ds_.skipPosCount_ += nSkipPCount;
    return ( nDocID >= target )? nDocID : -1;
}

void OnDiskPosting::seekTo(SkipListReader* pSkipListReader)
{
    IndexInput* pDPostingInput = getInputDescriptor()->getDPostingInput();
    pDPostingInput->seek(postingOffset_ + pSkipListReader->getOffset());
    ds_.lastDecodedDocID = pSkipListReader->getDoc();
    ds_.decodedDocCount = pSkipListReader->getNumSkipped();
    IndexInput* pPPostingInput = getInputDescriptor()->getPPostingInput();
    if(pPPostingInput)
    {
        pPPostingInput->seek(postingDesc_.poffset + pSkipListReader->getPOffset());
        ds_.lastDecodedPos = 0;///reset position
        ds_.skipPosCount_ = 0;
    }
}

int32_t OnDiskPosting::decodeNext(uint32_t* pPosting,int32_t length)
{
    int32_t left = postingDesc_.df - ds_.decodedDocCount;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length>>1);

    if (length > left*2)
        length = left*2;
    left = (length>>1);

    IndexInput* pDPostingInput = getInputDescriptor()->getDPostingInput();

    int32_t count = 0;
    docid_t did = ds_.lastDecodedDocID;
    count_t nSkipPCount = 0;

    while (count < left)
    {
        did += pDPostingInput->readVInt();
        if(!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;
            *pFreq++ = pDPostingInput->readVInt();
        }
        else
        {
            ///this doc is deleted
            nSkipPCount += pDPostingInput->readVInt();
        }				

        count++;
    }

    ///update state
    ds_.decodedDocCount += count;
    ds_.lastDecodedDocID = did;
    ds_.skipPosCount_ += nSkipPCount;

    return (int32_t)(pDoc - pPosting);
}
void OnDiskPosting::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    if (length <= 0)
        return;
    if(!pPosting)
    {
        ds_.skipPosCount_ += length;///just record the skip number
        return;
    }
	
    IndexInput* pPPostingInput = getInputDescriptor()->getPPostingInput();

    count_t nSkipPCount = ds_.skipPosCount_;
    while(nSkipPCount > 0) ///skip previous positions
    {
        pPPostingInput->readVInt();
        nSkipPCount--;
    }
    ds_.skipPosCount_ = 0;

    int32_t nDecoded = 0;
    loc_t loc = ds_.lastDecodedPos;

    uint32_t* pPos = pPosting;
    while (nDecoded < length)
    {
        loc += pPPostingInput->readVInt();
        *pPos++ = loc;
        nDecoded++;
    }
    ds_.decodedPosCount += nDecoded;
    ds_.lastDecodedPos = loc;
}
void OnDiskPosting::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    if(!pPosting)
    {
        for (int nF = 0;nF < nFreqs;nF++)///just record the skip number
            ds_.skipPosCount_ += pFreqs[nF];
        return;
    }

    IndexInput*	pPPostingInput = getInputDescriptor()->getPPostingInput();

    count_t nSkipPCount = ds_.skipPosCount_;
    while(nSkipPCount > 0) ///skip previous positions
    {
        pPPostingInput->readVInt();
        nSkipPCount--;
    }
    ds_.skipPosCount_ = 0;

    uint32_t nTotalDecoded = 0;
    uint32_t nCurDecoded = 0;
    loc_t loc = ds_.lastDecodedPos;
    uint32_t* pPos = pPosting;
    for (int32_t nF = 0;nF < nFreqs;nF++)
    {
        nCurDecoded = 0;
        while (nCurDecoded < pFreqs[nF])
        {
            loc += pPPostingInput->readVInt();
            *pPos++ = loc;
            nCurDecoded++;
        }
        nTotalDecoded += nCurDecoded;
        loc = 0;
    }

    ds_.decodedPosCount += nTotalDecoded;
    ds_.lastDecodedPos = loc;
}
void OnDiskPosting::resetPosition()
{
    ds_.lastDecodedPos = 0;
}

void OnDiskPosting::reset()
{
    nBufSize_ = 0;
    postingOffset_ = 0;
    postingDesc_.length = 0;
    postingDesc_.df = 0;
    postingDesc_.ctf = 0;
    postingDesc_.poffset = 0;

    chunkDesc_.length = 0;
    chunkDesc_.lastdocid = 0;

    nPPostingLength_ = 0;

    ds_.decodedDocCount = 0;
    ds_.lastDecodedDocID = 0;
    ds_.decodedPosCount= 0;
    ds_.lastDecodedPos = 0;
    ds_.skipPosCount_ = 0;

}

