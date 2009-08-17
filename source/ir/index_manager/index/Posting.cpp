#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

#include <math.h>


using namespace std;

using namespace izenelib::ir::indexmanager;

int32_t InMemoryPosting::UPTIGHT_ALLOC_CHUNKSIZE = 8;
int32_t InMemoryPosting::UPTIGHT_ALLOC_MEMSIZE = 40000;

Posting::Posting()
{}

Posting::~Posting()
{}


//////////////////////////////////////////////////////////////////////////
///CompressedPostingList
bool CompressedPostingList::addPosting(uint32_t posting32)
{
    if (pTailChunk == NULL)
        return false;
    int32_t left = pTailChunk->size - nPosInCurChunk;

    if (left < 7)///at least 7 free space
    {
        nTotalUnused += left;///Unused size
        pTailChunk->size = nPosInCurChunk;///the real size
        return false;
    }

    uint32_t ui = posting32;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk->data[nPosInCurChunk++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk->data[nPosInCurChunk++] = (uint8_t)ui;
    return true;
}

bool CompressedPostingList::addPosting(uint64_t posting64)
{
    if (pTailChunk == NULL)
        return false;
    int32_t left = pTailChunk->size - nPosInCurChunk;
    if (left < 11)///at least 11 free space
    {
        nTotalUnused += left;///Unused size
        pTailChunk->size = nPosInCurChunk;///the real size
        return false;
    }

    uint64_t ui = posting64;
    while ((ui & ~0x7F) != 0)
    {
        pTailChunk->data[nPosInCurChunk++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    pTailChunk->data[nPosInCurChunk++] = ((uint8_t)ui);

    return true;
}
int32_t CompressedPostingList::decodePosting32(uint8_t*& posting)
{
    uint8_t b = *posting++;
    int32_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *posting++;
        i |= (b & 0x7FL) << shift;
    }
    return i;
}

int64_t CompressedPostingList::decodePosting64(uint8_t*& posting)
{
    uint8_t b = *posting++;
    int64_t i = b & 0x7F;
    for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
    {
        b = *posting++;
        i |= (b & 0x7FLL) << shift;
    }
    return i;
}

void CompressedPostingList::encodePosting32(uint8_t*& posting,int32_t val)
{
    uint32_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *posting++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *posting++ = (uint8_t)ui;
}
void CompressedPostingList::encodePosting64(uint8_t*& posting,int64_t val)
{
    uint64_t ui = val;
    while ((ui & ~0x7F) != 0)
    {
        *posting++ = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
    }
    *posting++ = ((uint8_t)ui);
}

void CompressedPostingList::truncTailChunk()
{
    nTotalUnused += pTailChunk->size - nPosInCurChunk;
    pTailChunk->size = nPosInCurChunk;
}

void CompressedPostingList::addChunk(PostingChunk* pChunk)
{
    if (pTailChunk)
        pTailChunk->next = pChunk;
    pTailChunk = pChunk;
    if (!pHeadChunk)
        pHeadChunk = pTailChunk;
    nTotalSize += pChunk->size;

    nPosInCurChunk = 0;
}
int32_t CompressedPostingList::getRealSize()
{
    return nTotalSize - nTotalUnused;
}

void CompressedPostingList::reset()
{
    pHeadChunk = pTailChunk = NULL;
    nTotalSize = nPosInCurChunk = nTotalUnused = 0;
}

//////////////////////////////////////////////////////////////////////////
///InMemoryPosting
InMemoryPosting::AllocStrategy InMemoryPosting::ALLOCSTRATEGY;

InMemoryPosting::InMemoryPosting(MemCache* pCache)
        :pMemCache(pCache)
        ,nDF(0)
        ,nTDF(0)
        ,nLastDocID(BAD_DOCID)
        ,nYetAnotherLastDocID(BAD_DOCID)
        ,nLastLoc(BAD_DOCID)
        ,nCurTermFreq(0)
        ,nCTF(0)
        ,pDS(NULL)
{
    pDocFreqList = new CompressedPostingList();
    pLocList  = new CompressedPostingList();

    int32_t newSize = getNextChunkSize(pDocFreqList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
    pDocFreqList->addChunk(newChunk(newSize));
    newSize = getNextChunkSize(pLocList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
    pLocList->addChunk(newChunk(newSize));
}

InMemoryPosting::InMemoryPosting()
        :pMemCache(NULL)
        ,nDF(0)
        ,nTDF(0)
        ,nLastDocID(BAD_DOCID)
        ,nYetAnotherLastDocID(BAD_DOCID)
        ,nLastLoc(BAD_DOCID)
        ,nCurTermFreq(0)
        ,nCTF(0)
        ,pDS(NULL)
        ,pDocFreqList(NULL)
        ,pLocList(NULL)
{
}

InMemoryPosting::~InMemoryPosting()
{
    if (pDocFreqList)
    {
        delete pDocFreqList;
        pDocFreqList = NULL;
    }
    if (pLocList)
    {
        delete pLocList;
        pLocList = NULL;
    }
    pMemCache = NULL;

    if (pDS)
    {
        delete pDS;
        pDS = NULL;
    }
}

PostingChunk* InMemoryPosting::newChunk(int32_t chunkSize)
{
    uint8_t* begin = pMemCache->getMem(chunkSize);
    ///allocate memory failed,decrease chunk size
    if (!begin)
    {
        ///into UPTIGHT state
        begin = pMemCache->getMem(UPTIGHT_ALLOC_CHUNKSIZE);
        ///allocation failed again, grow memory cache.
        if (!begin)
        {
            begin  = pMemCache->grow(UPTIGHT_ALLOC_MEMSIZE)->getMem(UPTIGHT_ALLOC_CHUNKSIZE);
            if (!begin)
            {
                SF1V5_THROW(ERROR_OUTOFMEM,"InMemoryPosting:newChunk() : Allocate memory failed.");
            }
        }
        chunkSize = UPTIGHT_ALLOC_CHUNKSIZE;
    }

    PostingChunk* pChunk = (PostingChunk*)begin;
    pChunk->size = (int32_t)(POW_TABLE[chunkSize] - sizeof(PostingChunk*) - sizeof(int32_t));
    pChunk->next = NULL;
    return pChunk;
}

fileoffset_t InMemoryPosting::write(OutputDescriptor* pOutputDescriptor)
{
    ///flush last document
    flushLastDoc(true);

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();

    ///write chunk data
    writeDPosting(pDOutput);

    ///save the offset of posting descriptor
    fileoffset_t poffset = pDOutput->getFilePointer();

    IndexOutput* ptiOutput = pOutputDescriptor->getPPostingOutput();
    ///write position posting data
    fileoffset_t positionPointer = writePPosting(ptiOutput);
    ///write descriptors
    writeDescriptor(pDOutput,positionPointer);

    return poffset;
}

void InMemoryPosting::reset()
{
    pDocFreqList->reset();
    pLocList->reset();

    nCTF = 0;
    nLastDocID = BAD_DOCID;
    nYetAnotherLastDocID = BAD_DOCID;
    nLastLoc = 0;
    nDF = 0;
    nTDF = 0;
    nCurTermFreq = 0;

    if (pDS)
    {
        delete pDS;
        pDS = NULL;
    }
}

Posting* InMemoryPosting::clone()
{
    InMemoryPosting* pClone = new InMemoryPosting();
    if (pDocFreqList)
    {
        pClone->pDocFreqList = new CompressedPostingList(*pDocFreqList);
    }
    if (pLocList)
    {
        pClone->pLocList = new CompressedPostingList(*pLocList);
    }
    pClone->nCTF = nCTF;
    pClone->nDF = nDF;
    pClone->nTDF = nTDF;
    pClone->nCurTermFreq = nCurTermFreq;
    pClone->nLastDocID = nLastDocID;
    pClone->nYetAnotherLastDocID = nYetAnotherLastDocID;
    pClone->nLastLoc = nLastLoc;
    return pClone;
}


//////////////////////////////////////////////////////////////////////////
///InMemoryPosting
int32_t InMemoryPosting::getNextChunkSize(int32_t nCurSize,AllocStrategy& as)
{
    int32_t newSize = 0;
    if (as.strategy == STRATEGY_ALLOC_EXPLIMIT)
    {
        newSize = min(as.l,max(as.n,(int32_t)(nCurSize*(as.k - 1) + 0.5)) );
    }
    else if (as.strategy == STRATEGY_ALLOC_EXP)
    {
        newSize = max(as.n,(int32_t)(nCurSize*(as.k - 1) + 0.5));
    }
    else if (as.strategy == STRATEGY_ALLOC_CONST)
        newSize = as.n;
    return (int32_t)Utilities::LOG2_UP(newSize);
}

void InMemoryPosting::updateDF(docid_t docid)
{
    if (docid != nYetAnotherLastDocID)
    {
        nTDF++;
        nYetAnotherLastDocID = docid;
    }
}

void InMemoryPosting::addLocation(docid_t docid, freq_t doclength, loc_t location, loc_t sublocation)
{
    if (docid == nLastDocID)
    {
        ///see it before,only position is needed
        if (!pLocList->addPosting(location - nLastLoc))
        {
            ///chunk is exhausted
            int32_t newSize = getNextChunkSize(pLocList->nTotalSize, InMemoryPosting::ALLOCSTRATEGY);
            pLocList->addChunk(newChunk(newSize));
            pLocList->addPosting(location - nLastLoc);///d-gap encoding
        }
        if (!pLocList->addPosting(sublocation))
        {
            ///chunk is exhausted
            int32_t newSize = getNextChunkSize(pLocList->nTotalSize, InMemoryPosting::ALLOCSTRATEGY);
            pLocList->addChunk(newChunk(newSize));
            pLocList->addPosting(sublocation);///d-gap encoding
        }
        nCurTermFreq++;
        nLastLoc = location+sublocation;
    }
    else///first see it
    {
        if (nCurTermFreq > 0)///write previous document's term freq
        {
            if (!pDocFreqList->addPosting(nCurTermFreq))
            {
                ///chunk is exhausted
                int32_t newSize = getNextChunkSize(pDocFreqList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
                pDocFreqList->addChunk(newChunk(newSize));
                pDocFreqList->addPosting(nCurTermFreq);
            }
            /// doc length info, which is required by Ranking
            if (!pDocFreqList->addPosting(doclength))
            {
                ///chunk is exhausted
                int32_t newSize = getNextChunkSize(pDocFreqList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
                pDocFreqList->addChunk(newChunk(newSize));
                pDocFreqList->addPosting(nCurTermFreq);
            }
        }
        else if (nLastDocID == BAD_DOCID)///first see it
        {
            nLastDocID = 0;
        }
        if (!pDocFreqList->addPosting(docid - nLastDocID))
        {
            ///chunk is exhausted
            int32_t newSize = getNextChunkSize(pDocFreqList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
            pDocFreqList->addChunk(newChunk(newSize));
            pDocFreqList->addPosting(docid - nLastDocID);
        }
        if (!pLocList->addPosting(location))
        {
            ///chunk is exhausted
            int32_t newSize = getNextChunkSize(pLocList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
            pLocList->addChunk(newChunk(newSize));
            pLocList->addPosting(location);
        }

        if (!pLocList->addPosting(sublocation))
        {
            ///chunk is exhausted
            int32_t newSize = getNextChunkSize(pLocList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
            pLocList->addChunk(newChunk(newSize));
            pLocList->addPosting(sublocation);
        }

        nCTF += nCurTermFreq;
        nCurTermFreq = 1;

        nLastDocID = docid;

        nLastLoc = location+sublocation;

        nDF++;
    }
}

void InMemoryPosting::writeDPosting(IndexOutput* pDOutput)
{
    ///write chunk data
    PostingChunk* pChunk = pDocFreqList->pHeadChunk;
    while (pChunk)
    {
        pDOutput->write((const char*)pChunk->data,pChunk->size);
        pChunk = pChunk->next;
    }
}

fileoffset_t InMemoryPosting::writePPosting(IndexOutput* pPOutput)
{
    ///write position posting data
    PostingChunk* pChunk = pLocList->pHeadChunk;
    int size = 0;
    while (pChunk)
    {
        pPOutput->write((const char*)pChunk->data,pChunk->size);
        size+=pChunk->size;
        pChunk = pChunk->next;
    }
    fileoffset_t poffset = pPOutput->getFilePointer();
    pPOutput->writeVLong(pLocList->getRealSize());///<ChunkLength(VInt64)>
    return poffset;
}

void InMemoryPosting::writeDescriptor(IndexOutput* pDOutput,fileoffset_t poffset)
{
    ///begin write posting descriptor
    pDOutput->writeVLong(pDocFreqList->getRealSize());///<PostingLength(VInt64)>
    pDOutput->writeVInt(nDF); 						///<DF(VInt32)>
    pDOutput->writeVInt(nTDF);						///<TDF(VInt32)>
    pDOutput->writeVLong(nCTF);						///<CTF(VInt64)>
    pDOutput->writeVLong(poffset);						///<PositionPointer(VInt64)>
    ///end write posting descriptor

    pDOutput->writeVInt(1); 							///<ChunkCount(VInt32)>
    ///begin write chunk descriptor
    pDOutput->writeVLong(pDocFreqList->getRealSize());///<ChunkLength(VInt64)>
    pDOutput->writeVInt(nLastDocID);					///<LastDocID(VInt32)>
    ///end write posting descriptor
}

void InMemoryPosting::flushLastDoc(bool bTruncTail)
{
    if (nCurTermFreq > 0)
    {
        if (!pDocFreqList->addPosting(nCurTermFreq))
        {
            int32_t newSize = getNextChunkSize(pDocFreqList->nTotalSize,InMemoryPosting::ALLOCSTRATEGY);
            pDocFreqList->addChunk(newChunk(newSize));
            pDocFreqList->addPosting(nCurTermFreq);
        }
        if (bTruncTail)
        {
            pDocFreqList->truncTailChunk();///update real size
            pLocList->truncTailChunk();///update real size
        }
        nCTF += nCurTermFreq;
        nCurTermFreq = 0;
    }
    else if (bTruncTail)
    {
        pDocFreqList->truncTailChunk();///update real size
        pLocList->truncTailChunk();///update real size
    }
}



int32_t InMemoryPosting::decodeNext(uint32_t* pPosting,int32_t length)
{

#define ISCHUNKOVER_D()\
        if(pDChunk > pDChunkEnd)\
        {\
            pDS->decodingDChunk = pDS->decodingDChunk->next;\
            if(!pDS->decodingDChunk)\
                break;\
            pDS->decodingDChunkPos = 0;\
            pDChunk = &(pDS->decodingDChunk->data[pDS->decodingDChunkPos]);\
            pDChunkEnd = &(pDS->decodingDChunk->data[pDS->decodingDChunk->size-1]);\
        }

    ///flush last document
    //flushLastDoc(false);
    if (!pDS)
    {
        pDS = new InMemoryPosting::DecodeState;
        pDS->decodingDChunk = pDocFreqList->pHeadChunk;
        pDS->decodingDChunkPos = 0;
        pDS->lastDecodedDocID = 0;
        pDS->decodedDocCount = 0;
        pDS->decodingPChunk = pLocList->pHeadChunk;
        pDS->decodingPChunkPos = 0;
        pDS->lastDecodedPos = 0;
        pDS->decodedPosCount = 0;
    }

    uint32_t* pDoc = pPosting;
    //uint32_t* pFreq = pPosting + (length >> 1);
    uint32_t* pFreq = pPosting + (length/3);
    uint32_t* pDocLen = pPosting + (length*2/3);

    int32_t left = nDF - pDS->decodedDocCount;
    if (left <= 0)
        return -1;
    if (length > left*2)
        length = left*2;
    left = (length >> 1);

    uint8_t* pDChunk = &(pDS->decodingDChunk->data[pDS->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS->decodingDChunk->data[pDS->decodingDChunk->size-1]);

    int32_t count = 0;
    docid_t did = pDS->lastDecodedDocID;
    while (count < left)
    {
        ISCHUNKOVER_D();
        did += CompressedPostingList::decodePosting32(pDChunk);

        *pDoc++ = did;

        ISCHUNKOVER_D();

        *pFreq++ = CompressedPostingList::decodePosting32(pDChunk);

        *pDocLen++ = CompressedPostingList::decodePosting32(pDChunk);

        count++;
    }
    ///update state
    pDS->decodedDocCount += count;
    pDS->lastDecodedDocID = did;
    pDS->decodingDChunkPos = (int32_t)(pDChunk - pDS->decodingDChunk->data);

    return (int32_t)(pDoc - pPosting);
}

void InMemoryPosting::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    uint8_t* pPChunk = &(pDS->decodingPChunk->data[pDS->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS->decodingPChunk->data[pDS->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS->lastDecodedPos;
    int32_t  nDecoded = 0;
    while (nDecoded < length)
    {
        if (pPChunk > pPChunkEnd)
        {
            pDS->decodingPChunk = pDS->decodingPChunk->next;
            if (!pDS->decodingPChunk)
                break;
            pDS->decodingPChunkPos = 0;
            pPChunk = &(pDS->decodingPChunk->data[pDS->decodingPChunkPos]);
            pPChunkEnd = &(pDS->decodingPChunk->data[pDS->decodingPChunk->size-1]);
        }

        loc += CompressedPostingList::decodePosting32(pPChunk);
        if (pPos)
        {
            *pPos = loc;
            pPos++;
        }
        nDecoded++;
    }
    pDS->decodedPosCount += nDecoded;
    pDS->lastDecodedPos = loc;
}

void InMemoryPosting::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    uint8_t* pPChunk = &(pDS->decodingPChunk->data[pDS->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS->decodingPChunk->data[pDS->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS->lastDecodedPos;
    uint32_t  nTotalDecoded = 0;
    uint32_t  nCurDecoded;
    for (int32_t nF = 0; nF < nFreqs;nF++)
    {
        nCurDecoded = 0;
        while (nCurDecoded < pFreqs[nF])
        {
            if (pPChunk > pPChunkEnd)
            {
                pDS->decodingPChunk = pDS->decodingPChunk->next;
                if (!pDS->decodingPChunk)
                    break;
                pDS->decodingPChunkPos = 0;
                pPChunk = &(pDS->decodingPChunk->data[pDS->decodingPChunkPos]);
                pPChunkEnd = &(pDS->decodingPChunk->data[pDS->decodingPChunk->size-1]);
            }

            loc += CompressedPostingList::decodePosting32(pPChunk);
            if (pPos)
            {
                *pPos = loc;
                pPos++;
            }
            nCurDecoded++;
        }
        pDS->lastDecodedPos = loc = 0;
        nTotalDecoded += nCurDecoded;
    }

    pDS->decodedPosCount += nTotalDecoded;
    pDS->lastDecodedPos = loc;
}

void InMemoryPosting::resetPosition()
{
    pDS->lastDecodedPos = 0;
}
//////////////////////////////////////////////////////////////////////////
///OnDiskPosting
OnDiskPosting::OnDiskPosting(InputDescriptor* pInputDescriptor,fileoffset_t poffset)
        :pInputDescriptor(pInputDescriptor)
        ,nBufSize(0)
{
    postingOffset = poffset;
    reset(postingOffset);
}
OnDiskPosting::OnDiskPosting()
{
    nBufSize = 0;
    postingOffset = 0;
    postingDesc.length = 0;
    postingDesc.df = 0;
    postingDesc.tdf = 0;
    postingDesc.ctf = 0;
    postingDesc.poffset = 0;

    chunkDesc.length = 0;
    chunkDesc.lastdocid = 0;

    nPPostingLength = 0;

    ds.decodedDocCount = 0;
    ds.lastDecodedDocID = 0;
    ds.decodedPosCount= 0;
    ds.lastDecodedPos = 0;
}
OnDiskPosting::~OnDiskPosting()
{
}

Posting* OnDiskPosting::clone()
{
    return NULL;
}

void OnDiskPosting::reset(fileoffset_t newOffset)
{
    postingOffset = newOffset;
    IndexInput* pDPInput = pInputDescriptor->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();
    pDPInput->seekInternal(newOffset);///not seek(), because seek() may trigger a large data read event.

    ///read descriptor of posting list <PostingDescriptor>
    uint8_t buf[512];
    uint8_t* u = buf;
    pDPInput->readInternal((char*)buf,512,false);
    postingDesc.length = CompressedPostingList::decodePosting64(u);	///<PostingLength(VInt64)>
    postingDesc.df = CompressedPostingList::decodePosting32(u);		///<DF(VInt32)>
    postingDesc.tdf = CompressedPostingList::decodePosting32(u);		///<TDF(VInt32)>
    postingDesc.ctf = CompressedPostingList::decodePosting64(u);		///<CTF(VInt64)>
    postingDesc.poffset = CompressedPostingList::decodePosting64(u);	///PositionPointer(VInt64)

    CompressedPostingList::decodePosting32(u);///<ChunkCount(VInt32)>
    ///read first chunk descriptor of posting list <ChunkDescriptor>
    chunkDesc.length = CompressedPostingList::decodePosting64(u);	///<ChunkLength(VInt64)>
    chunkDesc.lastdocid = CompressedPostingList::decodePosting32(u);	///<LastDocID(VInt32)>

    IndexInput* pPPInput = pInputDescriptor->getPPostingInput();
    if (pPPInput)
    {
        pPPInput->reset();
        pPPInput->seekInternal(postingDesc.poffset);///not seek(), because seek() may trigger a large data read event.
        pPPInput->readInternal((char*)buf,8,false);
        u = buf;
        nPPostingLength = CompressedPostingList::decodePosting64(u); ///<ChunkLength(VInt64)>
        pPPInput->seek(postingDesc.poffset - nPPostingLength);///seek to the begin of position posting data
    }
    else
    {
        nPPostingLength = 0;
    }

    pDPInput->seek(newOffset - postingDesc.length);	///seek to the begin of posting data

    ds.decodedDocCount = 0;
    ds.lastDecodedDocID = 0;
    ds.decodedPosCount= 0;
    ds.lastDecodedPos = 0;
}

int32_t OnDiskPosting::decodeNext(uint32_t* pPosting,int32_t length)
{
    int32_t left = postingDesc.df - ds.decodedDocCount;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    //uint32_t* pFreq = pPosting + (length>>1);
    uint32_t* pFreq = pPosting + (length/3);
    uint32_t* pDocLen = pPosting + (length*2/3);

    if (length > left*2)
        length = left*2;
    left = (length>>1);

    IndexInput* pDPostingInput = getInputDescriptor()->getDPostingInput();

    int32_t count = 0;
    docid_t did = ds.lastDecodedDocID;
    while (count < left)
    {
        did += pDPostingInput->readVInt();

        *pDoc++ = did;
        *pFreq++ = pDPostingInput->readVInt();
        *pDocLen++ = pDPostingInput->readVInt();

        count++;
    }

    ///update state
    ds.decodedDocCount += count;
    ds.lastDecodedDocID = did;

    return (int32_t)(pDoc - pPosting);
}
void OnDiskPosting::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    if (length <= 0)
        return;
    IndexInput* pPPostingInput = getInputDescriptor()->getPPostingInput();

    int32_t nDecoded = 0;
    loc_t loc = ds.lastDecodedPos;
    uint32_t* pPos = pPosting;
    while (nDecoded < length)
    {
        loc += pPPostingInput->readVInt();
        if (pPos)
        {
            *pPos = loc;
            pPos++;
        }
        nDecoded++;
    }

    ds.decodedPosCount += nDecoded;
    ds.lastDecodedPos = loc;
}
void OnDiskPosting::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    IndexInput*	pPPostingInput = getInputDescriptor()->getPPostingInput();

    uint32_t nTotalDecoded = 0;
    uint32_t nCurDecoded = 0;
    loc_t loc = ds.lastDecodedPos;
    uint32_t* pPos = pPosting;
    for (int32_t nF = 0;nF < nFreqs;nF++)
    {
        nCurDecoded = 0;
        while (nCurDecoded < pFreqs[nF])
        {
            loc += pPPostingInput->readVInt();
            if (pPos)
            {
                *pPos = loc;
                pPos++;
            }
            nCurDecoded++;
        }
        nTotalDecoded += nCurDecoded;
        loc = 0;
    }

    ds.decodedPosCount += nTotalDecoded;
    ds.lastDecodedPos = loc;
}
void OnDiskPosting::resetPosition()
{
    ds.lastDecodedPos = 0;
}

void OnDiskPosting::reset()
{
    reset(postingOffset);
}

size_t OnDiskPosting::setBuffer(int32_t* buffer,size_t nBufSize)
{
    size_t nBufUsed = nBufSize*sizeof(int32_t);
    if (nBufUsed <= 2*INDEXINPUT_BUFFSIZE)
    {
        this->nBufSize = 0;
        return 0;
    }

    IndexInput* pDInput = getInputDescriptor()->getDPostingInput();
    IndexInput* pPInput = getInputDescriptor()->getPPostingInput();
    if ((int64_t)nBufUsed > (postingDesc.length + nPPostingLength))
    {
        nBufUsed = (size_t)(postingDesc.length + nPPostingLength);
        pDInput->setBuffer((char*)buffer,(size_t)postingDesc.length);
        if (pPInput)
            pPInput->setBuffer((char*)buffer + postingDesc.length,(size_t)nPPostingLength);
    }
    else
    {
        size_t nDSize = nBufUsed/2;
        if ((int64_t)nDSize > postingDesc.length)
            nDSize = (size_t)postingDesc.length;
        pDInput->setBuffer((char*)buffer,nDSize);
        if (pPInput)
            pPInput->setBuffer((char*)buffer + nDSize,nBufUsed - nDSize);
    }
    nBufSize = nBufUsed;
    return (nBufUsed + sizeof(int32_t) - 1)/sizeof(int32_t);
}

