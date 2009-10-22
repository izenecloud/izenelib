#include <ir/index_manager/index/PostingMerger.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

using namespace izenelib::ir::indexmanager;

PostingMerger::PostingMerger()
        :buffer(NULL)
        ,bufsize(0)
        ,bOwnBuffer(false)
        ,pOutputDescriptor(NULL)
        ,nPPostingLength(0)
        ,bFirstPosting(true)
{
    reset();
}

PostingMerger::PostingMerger(OutputDescriptor* pOutputDescriptor)
        :buffer(NULL)
        ,bufsize(0)
        ,bOwnBuffer(false)
        ,pOutputDescriptor(pOutputDescriptor)
        ,nPPostingLength(0)
        ,bFirstPosting(true)
{
    reset();
}

PostingMerger::~PostingMerger()
{
    if (buffer && bOwnBuffer)
    {
        delete buffer;
        buffer = NULL;
    }
    bufsize = 0;
}

inline void PostingMerger::reset()
{
    ///reset posting descriptor
    postingDesc.length = 0;
    postingDesc.ctf = 0;
    postingDesc.df = 0;
    postingDesc.tdf = 0;
    postingDesc.poffset = -1;
    chunkDesc.lastdocid = 0;
    chunkDesc.length = 0;
}

void PostingMerger::setBuffer(char* buf,size_t bufSize)
{
    buffer = buf;
    bufsize = bufSize;
}
void PostingMerger::createBuffer()
{
    buffer = new char[POSTINGMERGE_BUFFERSIZE];
    bufsize = POSTINGMERGE_BUFFERSIZE;
    bOwnBuffer = true;
}

void PostingMerger::mergeWith(InMemoryPosting* pInMemoryPosting)
{
    ///flush last doc
    pInMemoryPosting->flushLastDoc(true);

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();
    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting)///first posting
    {
        reset();
        nPPostingLength = 0;
        bFirstPosting = false;
        ///save position posting offset
        postingDesc.poffset = pPOutput->getFilePointer();
    }
    ///write chunk data, update the first doc id
    PostingChunk* pDChunk = pInMemoryPosting->pDocFreqList->pHeadChunk;
    if (pDChunk)
    {
        uint8_t* bp = &(pDChunk->data[0]);
        docid_t firstDocID = CompressedPostingList::decodePosting32(bp) - chunkDesc.lastdocid;
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

    chunkDesc.length += (pDOutput->getFilePointer() - oldDOff);

    ///write position posting
    PostingChunk* pPChunk = pInMemoryPosting->pLocList->pHeadChunk;
    while (pPChunk)
    {
        pPOutput->write((const char*)pPChunk->data,pPChunk->size);
        pPChunk = pPChunk->next;
    }

    ///update descriptors
    postingDesc.ctf += pInMemoryPosting->nCTF;
    postingDesc.df += pInMemoryPosting->nDF;
    postingDesc.tdf += pInMemoryPosting->nTDF;
    postingDesc.length = chunkDesc.length;

    chunkDesc.lastdocid = pInMemoryPosting->nLastDocID;
}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting)
{
    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting)///first posting
    {
        reset();
        nPPostingLength = 0;
        bFirstPosting = false;

        ///save position offset
        postingDesc.poffset = pPOutput->getFilePointer();
    }

    docid_t firstDocID = pDInput->readVInt() - chunkDesc.lastdocid;
    pDOutput->writeVInt(firstDocID);///write first doc id
    int64_t writeSize = pOnDiskPosting->postingDesc.length - pDOutput->getVIntLength(firstDocID + chunkDesc.lastdocid);
    if (writeSize > 0)
        pDOutput->write(pDInput,writeSize);

    chunkDesc.length += (pDOutput->getFilePointer() - oldDOff);

    ///write position posting
    pPOutput->write(pPInput,pOnDiskPosting->nPPostingLength);

    ///update descriptors
    postingDesc.ctf += pOnDiskPosting->postingDesc.ctf;
    postingDesc.df += pOnDiskPosting->postingDesc.df;
    postingDesc.tdf += pOnDiskPosting->postingDesc.tdf;
    postingDesc.length = chunkDesc.length; ///currently,it's only one chunk
    chunkDesc.lastdocid = pOnDiskPosting->chunkDesc.lastdocid;

}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting,BitVector* pFilter)
{
    if(pFilter &&  pFilter->hasSmallThan((size_t)pOnDiskPosting->chunkDesc.lastdocid))
        mergeWith_GC(pOnDiskPosting,pFilter);
    else
        mergeWith(pOnDiskPosting);
}

void PostingMerger::mergeWith_GC(OnDiskPosting* pOnDiskPosting,BitVector* pFilter)
{
    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    docid_t nDocID = 0;
    docid_t nDocIDPrev = 0;
    docid_t nLastDocID = 0;
    freq_t	nTF = 0;
    freq_t nDocLength = 0;
    count_t nCTF = 0;
    count_t nDF = 0;
    count_t nPCount = 0;
    count_t nODDF = pOnDiskPosting->postingDesc.df;
    if(nODDF <= 0)
        return;

    fileoffset_t oldDOff = pDOutput->getFilePointer();

    if (bFirstPosting)///first posting
    {
        reset();
        nPPostingLength = 0;
        bFirstPosting = false;

        ///save position offset
        postingDesc.poffset = pPOutput->getFilePointer();
    }

    while (nODDF > 0)
    {
        nDocID += pDInput->readVInt();
        nTF = pDInput->readVInt();
        nDocLength = pDInput->readVInt();
        if(!pFilter->test((size_t)nDocID))///the document has not been deleted
        {
            pDOutput->writeVInt(nDocID - nDocIDPrev);
            pDOutput->writeVInt(nTF);
            pDOutput->writeVInt(nDocLength);

            nDocIDPrev = nDocID;
            nPCount += nTF;
            nDF++;
            nLastDocID = nDocID;
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

    chunkDesc.length += (pDOutput->getFilePointer() - oldDOff);

    ///update descriptors
    postingDesc.ctf += nCTF;
    postingDesc.df += nDF;
    postingDesc.length = chunkDesc.length; ///currently,it's only one chunk 			
    chunkDesc.lastdocid = nLastDocID;
}

fileoffset_t PostingMerger::endMerge()
{
    bFirstPosting = true;
    if (postingDesc.df <= 0)
        return -1;

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();
    fileoffset_t postingoffset = pDOutput->getFilePointer();

    ///write position posting descriptor
    nPPostingLength = pPOutput->getFilePointer() - postingDesc.poffset;
    postingDesc.poffset = pPOutput->getFilePointer();
    pPOutput->writeVLong(nPPostingLength);	////<ChunkLength(VInt64)>

    ///begin write posting descriptor
    pDOutput->writeVLong(postingDesc.length);	///<PostingLength(VInt64)>
    pDOutput->writeVInt(postingDesc.df);		///<DF(VInt32)>
    pDOutput->writeVInt(postingDesc.tdf);		///<TDF(VInt32)>
    pDOutput->writeVLong(postingDesc.ctf);	///<CTF(VInt64)>
    pDOutput->writeVLong(postingDesc.poffset);///<PositionPointer(VInt64)>
    ///end write posting descriptor

    pDOutput->writeVInt(1);						///<ChunkCount(VInt32)>
    ///begin write chunk descriptor
    pDOutput->writeVLong(chunkDesc.length);	///<ChunkLength(VInt64)>
    pDOutput->writeVInt(chunkDesc.lastdocid);	///<LastDocID(VInt32)>
    ///end write posting descriptor

    return postingoffset;
}

