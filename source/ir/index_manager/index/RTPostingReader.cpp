#include <ir/index_manager/index/RTPostingReader.h>
#include <ir/index_manager/index/RTPostingWriter.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/SkipListWriter.h>
#include <ir/index_manager/index/SkipListReader.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/utility/Bitset.h>

#include <math.h>


using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

MemPostingReader::MemPostingReader(
        boost::shared_ptr<RTPostingWriter> pPostingWriter,
        const TermInfo& termInfo,
        IndexLevel type)
    : pPostingWriter_(pPostingWriter)
    , pDocFreqList_(pPostingWriter->pDocFreqList_)
    , termInfo_(termInfo)
    , pDS_(NULL)
    , pSkipListReader_(0)
    , pDocFilter_(0)
{
    if (type == WORDLEVEL)
    {
        assert(pPostingWriter->indexLevel_ == WORDLEVEL);
        pLocList_ = pPostingWriter->pLocList_;
    }
}

MemPostingReader::~MemPostingReader()
{
    if (pDS_)
    {
        delete pDS_;
        pDS_ = NULL;
    }

    if (pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;
    }
}

count_t MemPostingReader::docFreq() const
{
    return termInfo_.docFreq_;
};

int64_t MemPostingReader::getCTF() const
{
    return termInfo_.ctf_;
};

int32_t MemPostingReader::getMaxDocFreq() const
{
    return termInfo_.maxTF_;
};

docid_t MemPostingReader::lastDocID()
{
    return termInfo_.lastDocID_;
}

int32_t MemPostingReader::getSkipLevel()
{
    return termInfo_.skipLevel_;
}

count_t MemPostingReader::getDPostingLen()
{
    pPostingWriter_->flushLastDoc(true);
    return pPostingWriter_->pDocFreqList_->getLength();
}

count_t MemPostingReader::getPPostingLen()
{
    pPostingWriter_->flushLastDoc(true);
    if (pPostingWriter_->pLocList_)
        return pPostingWriter_->pLocList_->getLength();
    else
        return 0;
}

#define ISCHUNKOVER_D()\
    if (pDChunk > pDChunkEnd)\
    {\
        pDS_->decodingDChunk = pDS_->decodingDChunk->next;\
        if (!pDS_->decodingDChunk)\
            break;\
        pDS_->decodingDChunkPos = 0;\
        pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);\
        pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);\
    }

#define ISCHUNKOVER_P()\
    if (pPChunk > pPChunkEnd)\
    {\
        pDS_->decodingPChunk = pDS_->decodingPChunk->next;\
        if (!pDS_->decodingPChunk)\
            break;\
        pDS_->decodingPChunkPos = 0;\
        pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);\
        pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);\
    }

int32_t MemPostingReader::DecodeNext(
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs
)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    if (!pDS_)
    {
        pDS_ = new MemPostingReader::DecodeState;
        pDS_->decodingDChunk = pDocFreqList_->pHeadChunk_;
        pDS_->decodingDChunkPos = 0;
        pDS_->lastDecodedDocID = 0;
        pDS_->decodedDocCount = 0;
        pDS_->decodingPChunk = NULL;
        if (pLocList_)
            pDS_->decodingPChunk = pLocList_->pHeadChunk_;
        pDS_->decodingPChunkPos = 0;
        pDS_->lastDecodedPos = 0;
        pDS_->decodedPosCount = 0;
    }

    if (! pDS_->decodingDChunk)
    {
        return 0;
    }

    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length >> 1);

    int32_t left = termInfo_.docFreq_ - pDS_->decodedDocCount;
    if (left <= 0)
        return -1;

    uint8_t* pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);

    docid_t did = pDS_->lastDecodedDocID;
    int32_t docCount = 0; // docs decoded from pDChunk
    int32_t copiedCount = 0; // docs copied into pDoc
    for (; copiedCount < nMaxDocs && docCount < left; ++docCount)
    {
        ISCHUNKOVER_D();
        did += VariantDataPool::decodeVData32(pDChunk);

        if (!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;

            ISCHUNKOVER_D();

            *pFreq++ = VariantDataPool::decodeVData32(pDChunk);
            ++copiedCount;
        }
        else
        {
            ///this doc is deleted
            ISCHUNKOVER_D();
            VariantDataPool::decodeVData32(pDChunk);
        }
    }

    ///update state
    pDS_->decodedDocCount += docCount;
    pDS_->lastDecodedDocID = did;
    pDS_->decodingDChunkPos = (int32_t)(pDChunk - pDS_->decodingDChunk->data);

    return copiedCount;
}

int32_t MemPostingReader::DecodeNext(
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs,
    uint32_t* &pPPosting,
    int32_t& posBufLength,
    int32_t& posLength)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    if (!pDS_)
    {
        pDS_ = new MemPostingReader::DecodeState;
        pDS_->decodingDChunk = pDocFreqList_->pHeadChunk_;
        pDS_->decodingDChunkPos = 0;
        pDS_->lastDecodedDocID = 0;
        pDS_->decodedDocCount = 0;
        pDS_->decodingPChunk = NULL;
        if (pLocList_)
            pDS_->decodingPChunk = pLocList_->pHeadChunk_;
        pDS_->decodingPChunkPos = 0;
        pDS_->lastDecodedPos = 0;
        pDS_->decodedPosCount = 0;
    }

    if (! pDS_->decodingDChunk)
    {
        return 0;
    }

    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length >> 1);
    uint8_t* pPChunk = NULL;
    uint8_t* pPChunkEnd = NULL;
    if (pDS_->decodingPChunk)
    {
        pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
        pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);
    }

    int32_t left = termInfo_.docFreq_ - pDS_->decodedDocCount;
    if (left <= 0)
        return -1;

    uint8_t* pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);

    docid_t did = pDS_->lastDecodedDocID;
    int32_t nFreqs = 0;

    posLength = 0;
    int32_t docCount = 0; // docs decoded from pDChunk
    int32_t copiedCount = 0; // docs copied into pDoc
    for (; copiedCount < nMaxDocs && docCount < left; ++docCount)
    {
        ISCHUNKOVER_D();
        did += VariantDataPool::decodeVData32(pDChunk);

        if (!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;
            uint32_t nCurTF = 0;
            if (did == termInfo_.lastDocID_)
            {
                nCurTF = termInfo_.currTF_;
            }
            else
            {
                ISCHUNKOVER_D();
                nCurTF = VariantDataPool::decodeVData32(pDChunk);
            }
            *pFreq++ = nCurTF;
            ++copiedCount;

            nFreqs += nCurTF;
            if (posBufLength < nFreqs)
                GrowPosBuffer(pPPosting, posBufLength, nFreqs);

            loc_t loc = 0;
            if (pDS_->decodingPChunk)
            {
                for (uint32_t i = 0; i < nCurTF; ++i)
                {
                    ISCHUNKOVER_P();
                    loc += VariantDataPool::decodeVData32(pPChunk);
                    pPPosting[posLength++] = loc;
                }
            }
        }
        else
        {
            ///this doc is deleted
            ISCHUNKOVER_D();
            uint32_t nCurTF = VariantDataPool::decodeVData32(pDChunk);
            if (pDS_->decodingPChunk)
            {
                for (uint32_t i = 0; i < nCurTF; ++i)
                {
                    ISCHUNKOVER_P();
                    VariantDataPool::decodeVData32(pPChunk);
                }
            }
        }
    }

    ///update state
    pDS_->decodedDocCount += docCount;
    pDS_->lastDecodedDocID = did;
    pDS_->decodingDChunkPos = (int32_t)(pDChunk - pDS_->decodingDChunk->data);

    return copiedCount;
}

bool MemPostingReader::DecodeNextPositions(
    uint32_t* pPosting,
    int32_t length)
{
    if (! pDS_->decodingPChunk)
    {
        return false;
    }

    uint8_t* pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS_->lastDecodedPos;
    int32_t  nDecoded = 0;
    while (nDecoded < length)
    {
        ISCHUNKOVER_P();
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
    return true;
}

bool MemPostingReader::DecodeNextPositions(
    uint32_t* &pPosting,
    int32_t& posBufLength,
    int32_t decodeLength,
    int32_t& nCurrentPPosting)
{
    if (! pDS_->decodingPChunk)
    {
        return false;
    }

    uint8_t* pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
    uint8_t* pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

    uint32_t* pPos = pPosting;
    loc_t loc = pDS_->lastDecodedPos;
    int32_t  nDecoded = 0;
    while (nDecoded < decodeLength)
    {
        ISCHUNKOVER_P();
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
    nCurrentPPosting = 0;
    return true;
}

bool MemPostingReader::DecodeNextPositions(
    uint32_t* &pPosting,
    int32_t& posBufLength,
    uint32_t* pFreqs,
    int32_t nFreqs,
    int32_t& nCurrentPPosting)
{
    if (! pDS_->decodingPChunk)
    {
        return false;
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
        if (!pDS_->decodingPChunk) break;
        while (nCurDecoded < pFreqs[nF])
        {
            ISCHUNKOVER_P();
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
    return true;
}

SkipListReader* MemPostingReader::getSkipListReader()
{
    if (!pSkipListReader_ && pPostingWriter_->pSkipListWriter_)
        pSkipListReader_ = pPostingWriter_->pSkipListWriter_->getSkipListReader();

    return pSkipListReader_;
}

void MemPostingReader::ResetPosition()
{
    if (pDS_) pDS_->lastDecodedPos = 0;
}

docid_t MemPostingReader::DecodeTo(
    docid_t target,
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs,
    int32_t& decodedCount,
    int32_t& nCurrentPosting)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    ///skipping for in-memory posting is not that necessary
    ///just pass one by one

    if (!pDS_)
    {
        pDS_ = new MemPostingReader::DecodeState;
        pDS_->decodingDChunk = pDocFreqList_->pHeadChunk_;
        pDS_->decodingDChunkPos = 0;
        pDS_->lastDecodedDocID = 0;
        pDS_->decodedDocCount = 0;
        pDS_->decodingPChunk = NULL;
        if (pLocList_)
            pDS_->decodingPChunk = pLocList_->pHeadChunk_;
        pDS_->decodingPChunkPos = 0;
        pDS_->lastDecodedPos = 0;
        pDS_->decodedPosCount = 0;
    }
    if (! pDS_->decodingDChunk)
    {
        return BAD_DOCID;
    }

    int32_t left = termInfo_.docFreq_ - pDS_->decodedDocCount;

    uint8_t* pDChunk = &(pDS_->decodingDChunk->data[pDS_->decodingDChunkPos]);
    uint8_t* pDChunkEnd = &(pDS_->decodingDChunk->data[pDS_->decodingDChunk->size-1]);

    int32_t count = 0;
    docid_t did = pDS_->lastDecodedDocID;
    count_t nSkipPCount = 0;
    uint32_t nCurTF = 0;
    for (; count < left && did < target; ++count)
    {
        ISCHUNKOVER_D();
        did += VariantDataPool::decodeVData32(pDChunk);

        if (did == termInfo_.lastDocID_)
        {
            nCurTF = termInfo_.currTF_;
        }
        else
        {
            ISCHUNKOVER_D();
            nCurTF = VariantDataPool::decodeVData32(pDChunk);
        }

        nSkipPCount += nCurTF;
    }
    ///update state
    pDS_->decodedDocCount += count;
    pDS_->lastDecodedDocID = did;
    pDS_->decodingDChunkPos = (int32_t)(pDChunk - pDS_->decodingDChunk->data);

    if (pDS_->decodingPChunk)
    {
        uint8_t* pPChunk = NULL;
        uint8_t* pPChunkEnd = NULL;
        pPChunk = &(pDS_->decodingPChunk->data[pDS_->decodingPChunkPos]);
        pPChunkEnd = &(pDS_->decodingPChunk->data[pDS_->decodingPChunk->size-1]);

        loc_t loc = pDS_->lastDecodedPos;
        count_t nDecoded = 0;
        for (; nDecoded < nSkipPCount; nDecoded++)
        {
            ISCHUNKOVER_P();
            loc += VariantDataPool::decodeVData32(pPChunk);
        }
        pDS_->decodedPosCount += nDecoded;
        pDS_->lastDecodedPos = loc;
    }

    pPosting[0] = ( did >= target )? did : -1;
    pPosting[length>>1] = nCurTF;
    decodedCount = 1;
    nCurrentPosting = 0;
    return pPosting[0];
}

//////////////////////////////////////////////////////////////////////////
///RTDiskPostingReader

RTDiskPostingReader::RTDiskPostingReader(
        int skipInterval,
        int maxSkipLevel,
        InputDescriptor* pInputDescriptor,
        const TermInfo& termInfo)
    : skipInterval_(skipInterval)
    , maxSkipLevel_(maxSkipLevel)
    , inputDescriptorPtr_(pInputDescriptor)
    , pDocFilter_(0)
{
    reset(termInfo);
}

RTDiskPostingReader::~RTDiskPostingReader()
{
}

void RTDiskPostingReader::reset(const TermInfo& termInfo)
{
    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = inputDescriptorPtr_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    postingDesc_.length = termInfo.docPostingLen_;	///<PostingLength(VInt64)>
    postingDesc_.df = termInfo.docFreq_;			///<DF(VInt32)>
    postingDesc_.ctf = termInfo.ctf_;				///<CTF(VInt64)>
    postingDesc_.maxTF = termInfo.maxTF_;
    postingDesc_.poffset = termInfo.positionPointer_;	///PositionPointer(VInt64)
    postingDesc_.plength = termInfo.positionPostingLen_;

    chunkDesc_.length = termInfo.docPostingLen_;	///<ChunkLength(VInt64)>
    chunkDesc_.lastdocid = termInfo.lastDocID_;		///<LastDocID(VInt32)>

    skipListReaderPtr_.reset();
    if (termInfo.skipLevel_ > 0)
    {
        if ((termInfo.docFreq_ >= 4096)&&(termInfo.skipPointer_ != -1))
        {
            pDPInput->seek(termInfo.skipPointer_);
            skipListReaderPtr_.reset(new SkipListReader(pDPInput, skipInterval_, termInfo.skipLevel_));
        }
    }

    pDPInput->seek(postingOffset_);

    IndexInput* pPPInput = inputDescriptorPtr_->getPPostingInput();
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

docid_t RTDiskPostingReader::DecodeTo(
    docid_t target,
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs,
    int32_t& decodedCount,
    int32_t& nCurrentPosting)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    if ((count_t)(ds_.decodedDocCount) >= postingDesc_.df)
        return -1;

    if (skipListReaderPtr_.get())
    {
        docid_t lastDocID = skipListReaderPtr_->skipTo(target);
        if (lastDocID > ds_.lastDecodedDocID)
        {
            SeekTo(skipListReaderPtr_.get());
        }
    }

    docid_t nDocID = ds_.lastDecodedDocID;
    count_t nFreq = 0;
    count_t nSkipPCount = 0;
    count_t nDF = postingDesc_.df;
    count_t nDecodedCount = ds_.decodedDocCount;

    IndexInput* pDPostingInput = inputDescriptorPtr_->getDPostingInput();
    while (nDecodedCount < nDF)
    {
        nDocID += pDPostingInput->readVInt();
        ++nDecodedCount;
        if (!pDocFilter_ || !pDocFilter_->test((size_t)nDocID))
        {
            if (nDocID >= target)
            {
                nFreq = pDPostingInput->readVInt();
                break;
            }
            else
                nSkipPCount += pDPostingInput->readVInt();
        }
        else
            nSkipPCount += pDPostingInput->readVInt();
    }

    ///update state
    ds_.lastDecodedDocID = nDocID;
    ds_.lastDecodedDocTF = nFreq;
    ds_.decodedDocCount = nDecodedCount;
    ds_.skipPosCount_ += nSkipPCount;

    if (pDocFilter_ && pDocFilter_->test((size_t)nDocID))
        return -1;

    pPosting[0] = ( nDocID >= target )? nDocID : -1;
    pPosting[length>>1] = ds_.lastDecodedDocTF;
    decodedCount = 1;
    nCurrentPosting = 0;
    return pPosting[0];
}

void RTDiskPostingReader::SeekTo(
    SkipListReader* pSkipListReader)
{
    IndexInput* pDPostingInput = inputDescriptorPtr_->getDPostingInput();
    pDPostingInput->seek(postingOffset_ + pSkipListReader->getOffset());
    ds_.lastDecodedDocID = pSkipListReader->getDoc();
    ds_.decodedDocCount = pSkipListReader->getNumSkipped();
    IndexInput* pPPostingInput = inputDescriptorPtr_->getPPostingInput();
    if (pPPostingInput)
    {
        pPPostingInput->seek(postingDesc_.poffset + pSkipListReader->getPOffset());
        ds_.lastDecodedPos = 0;///reset position
        ds_.skipPosCount_ = 0;
    }
}

int32_t RTDiskPostingReader::DecodeNext(
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    int32_t left = postingDesc_.df - ds_.decodedDocCount;
    if (left <= 0)
        return -1;

    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length >> 1);

    IndexInput* pDPostingInput = inputDescriptorPtr_->getDPostingInput();

    docid_t did = ds_.lastDecodedDocID;
    count_t nSkipPCount = 0;

    int32_t docCount = 0; // docs decoded from pDPostingInput
    int32_t copiedCount = 0; // docs copied into pDoc
    for (; copiedCount < nMaxDocs && docCount < left; ++docCount)
    {
        did += pDPostingInput->readVInt();
        if (!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;
            *pFreq++ = pDPostingInput->readVInt();
            ++copiedCount;
         }
        else
        {
            ///this doc is deleted
            nSkipPCount += pDPostingInput->readVInt();
        }
    }

    ///update state
    ds_.decodedDocCount += docCount;
    ds_.lastDecodedDocID = did;
    ds_.skipPosCount_ += nSkipPCount;

    return copiedCount;
}

int32_t RTDiskPostingReader::DecodeNext(
    uint32_t* pPosting,
    int32_t length,
    int32_t nMaxDocs,
    uint32_t* &pPPosting,
    int32_t& posBufLength,
    int32_t& posLength)
{
    assert(nMaxDocs > 0 && nMaxDocs <= length >> 1);

    int32_t left = postingDesc_.df - ds_.decodedDocCount;
    if (left <= 0)
        return -1;

    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length >> 1);

    skipPositions();

    IndexInput* pDPostingInput = inputDescriptorPtr_->getDPostingInput();
    IndexInput*	pPPostingInput = inputDescriptorPtr_->getPPostingInput();

    docid_t did = ds_.lastDecodedDocID;
    int32_t nFreqs = 0;

    posLength = 0;
    int32_t docCount = 0; // docs decoded from pDPostingInput
    int32_t copiedCount = 0; // docs copied into pDoc
    for (; copiedCount < nMaxDocs && docCount < left; ++docCount)
    {
        did += pDPostingInput->readVInt();
        if (!pDocFilter_ || !pDocFilter_->test((size_t)did))
        {
            *pDoc++ = did;
            uint32_t nCurTF = pDPostingInput->readVInt();
            *pFreq++ = nCurTF;
            ++copiedCount;

            nFreqs += nCurTF;
            if (posBufLength < nFreqs)
                GrowPosBuffer(pPPosting, posBufLength, nFreqs);

            loc_t loc = 0;
            if (pPPostingInput)
            {
                for (uint32_t i = 0; i < nCurTF; ++i)
                {
                    loc += pPPostingInput->readVInt();
                    pPPosting[posLength++] = loc;
                }
            }
        }
        else
        {
            ///this doc is deleted, skip positions
            uint32_t nCurTF = pDPostingInput->readVInt();
            if (pPPostingInput)
            {
                for (uint32_t i = 0; i < nCurTF; ++i)
                {
                    pPPostingInput->readVInt();
                }
            }
        }
    }

    ///update state
    ds_.decodedDocCount += docCount;
    ds_.lastDecodedDocID = did;

    return copiedCount;
}

bool RTDiskPostingReader::DecodeNextPositions(
    uint32_t* pPosting,
    int32_t length)
{
    if (length <= 0)
        return true;
    if (!pPosting)
    {
        ds_.skipPosCount_ += length;///just record the skip number
        return true;
    }

    IndexInput* pPPostingInput = inputDescriptorPtr_->getPPostingInput();

    skipPositions();

    int32_t nDecoded = 0;
    loc_t loc = ds_.lastDecodedPos;

    uint32_t* pPos = pPosting;
    if (pPPostingInput)
    {
        while (nDecoded < length)
        {
            loc += pPPostingInput->readVInt();
            *pPos++ = loc;
            nDecoded++;
        }
    }
    ds_.decodedPosCount += nDecoded;
    ds_.lastDecodedPos = loc;
    return true;
}

bool RTDiskPostingReader::DecodeNextPositions(
    uint32_t* &pPosting,
    int32_t& posBufLength,
    int32_t decodeLength,
    int32_t& nCurrentPPosting)
{
    if (decodeLength <= 0)
        return true;
    if (!pPosting)
    {
        ds_.skipPosCount_ += decodeLength;///just record the skip number
        return true;
    }

    IndexInput* pPPostingInput = inputDescriptorPtr_->getPPostingInput();

    skipPositions();

    int32_t nDecoded = 0;
    loc_t loc = ds_.lastDecodedPos;

    uint32_t* pPos = pPosting;
    if (pPPostingInput)
    {
        while (nDecoded < decodeLength)
        {
            loc += pPPostingInput->readVInt();
            *pPos++ = loc;
            nDecoded++;
        }
    }
    ds_.decodedPosCount += nDecoded;
    ds_.lastDecodedPos = loc;
    nCurrentPPosting = 0;
    return true;
}

bool RTDiskPostingReader::DecodeNextPositions(
    uint32_t* &pPosting,
    int32_t& posBufLength,
    uint32_t* pFreqs,
    int32_t nFreqs,
    int32_t& nCurrentPPosting)
{
    if (!pPosting)
    {
        for (int nF = 0;nF < nFreqs;nF++)///just record the skip number
            ds_.skipPosCount_ += pFreqs[nF];
        return true;
    }

    IndexInput*	pPPostingInput = inputDescriptorPtr_->getPPostingInput();

    skipPositions();

    uint32_t nTotalDecoded = 0;
    uint32_t nCurDecoded = 0;
    loc_t loc = ds_.lastDecodedPos;
    uint32_t* pPos = pPosting;
    if (pPPostingInput)
    {
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
    }

    ds_.decodedPosCount += nTotalDecoded;
    ds_.lastDecodedPos = loc;
    return true;
}

void RTDiskPostingReader::ResetPosition()
{
    ds_.lastDecodedPos = 0;
}

void RTDiskPostingReader::reset()
{
    postingOffset_ = 0;
    postingDesc_.length = 0;
    postingDesc_.df = 0;
    postingDesc_.ctf = 0;
    postingDesc_.maxTF = 0;
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

void RTDiskPostingReader::skipPositions()
{
    if (ds_.skipPosCount_ > 0)
    {
        IndexInput* pPPostingInput = inputDescriptorPtr_->getPPostingInput();
        if (pPPostingInput)
        {
            size_t nSkipPCount = ds_.skipPosCount_;
            while (nSkipPCount > 0) ///skip previous positions
            {
                pPPostingInput->readVInt();
                nSkipPCount--;
            }
            ds_.skipPosCount_ = 0;
        }
    }
}

}

NS_IZENELIB_IR_END
