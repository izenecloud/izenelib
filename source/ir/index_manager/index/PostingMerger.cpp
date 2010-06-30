#include <ir/index_manager/index/PostingMerger.h>
#include <ir/index_manager/store/FSIndexOutput.h>
#include <ir/index_manager/store/FSIndexInput.h>

using namespace izenelib::ir::indexmanager;

PostingMerger::PostingMerger()
        :pOutputDescriptor_(NULL)
        ,pTmpPostingOutput_(NULL)
        ,pTmpPostingInput_(NULL)
        ,nPPostingLength_(0)
        ,nSkipIntervalBetweenBarrels_(0)
        ,bFirstPosting_(true)
        ,pSkipListMerger_(NULL)
        ,pMemCache_(NULL)
{
    init();
    reset();
}

PostingMerger::~PostingMerger()
{
    if(pMemCache_)
        delete pMemCache_;
    if(pSkipListMerger_)
        delete pSkipListMerger_;
    if(pTmpPostingOutput_)
    {
        delete pTmpPostingInput_;
        delete pTmpPostingOutput_;
        pOutputDescriptor_->getDirectory()->deleteFile(tmpPostingName_);
    }
}

void PostingMerger::reset()
{
    ///reset posting descriptor
    postingDesc_.length = 0;
    postingDesc_.plength = 0;
    postingDesc_.ctf = 0;
    postingDesc_.df = 0;
    postingDesc_.poffset = -1;
    chunkDesc_.lastdocid = 0;
    chunkDesc_.length = 0;
    termInfo_.reset();
    pMemCache_->flushMem();
    nSkipIntervalBetweenBarrels_ = 0;
    if(pSkipListMerger_)
    {
        pSkipListMerger_->reset();
    }
    if(pTmpPostingOutput_)
    {
        reinterpret_cast<FSIndexOutput*>(pTmpPostingOutput_)->trunc();
    }
}

void PostingMerger::init()
{
    pMemCache_ = new MemCache(POSTINGMERGE_BUFFERSIZE*512);
    if(Posting::skipInterval_ > 0)
        pSkipListMerger_ = new SkipListMerger(Posting::skipInterval_,Posting::maxSkipLevel_,pMemCache_);
}

void PostingMerger::setOutputDescriptor(OutputDescriptor* pOutputDescriptor)
{
    pOutputDescriptor_ = pOutputDescriptor;
    tmpPostingName_ = pOutputDescriptor_->getBarrelName() + ".merge";
    pTmpPostingOutput_ = pOutputDescriptor_->getDirectory()->createOutput(tmpPostingName_);
}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting,BitVector* pFilter)
{
    if(pFilter &&  pFilter->hasSmallThan((size_t)pOnDiskPosting->chunkDesc_.lastdocid))
        mergeWith_GC(pOnDiskPosting,pFilter);
    else
        mergeWith(pOnDiskPosting);
}

void PostingMerger::mergeWith(InMemoryPosting* pInMemoryPosting)
{
    ///flush last doc
    pInMemoryPosting->flushLastDoc(true);

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();

    if (bFirstPosting_)///first posting
    {
        reset();
        if(Posting::skipInterval_ == 0)
        {
            termInfo_.docPointer_ = pDOutput->getFilePointer();
            termInfo_.skipPointer_ = -1;
        }
        else
            termInfo_.skipPointer_ = pDOutput->getFilePointer();
        termInfo_.positionPointer_ = pPOutput->getFilePointer();
        nPPostingLength_ = 0;
        bFirstPosting_ = false;
        ///save position posting offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }

    IndexOutput* pDocIndexOutput = 0;

    if(Posting::skipInterval_ == 0)
    {
        pDocIndexOutput = pDOutput;
    }
    else
        pDocIndexOutput = pTmpPostingOutput_;

    fileoffset_t oldDOff = pDocIndexOutput->getFilePointer();

    ///write chunk data, update the first doc id
    VariantDataChunk* pDChunk = pInMemoryPosting->pDocFreqList_->pHeadChunk_;

    ///DPosting start for skip
    int32_t DOffsetStartForSkipping = 0;
	
    if (pDChunk)
    {
        uint8_t* bp = &(pDChunk->data[0]);
        docid_t firstDocID = VariantDataPool::decodeVData32(bp) - chunkDesc_.lastdocid;

        if(chunkDesc_.lastdocid == 0)
            DOffsetStartForSkipping = 0;
        else
            DOffsetStartForSkipping = postingDesc_.length - (pDocIndexOutput->getVIntLength(firstDocID + chunkDesc_.lastdocid)
																	   - pDocIndexOutput->getVIntLength(firstDocID));
		
        pDocIndexOutput->writeVInt(firstDocID);///write first doc id
        int32_t writeSize = pDChunk->size - (bp - &(pDChunk->data[0]));	//write the rest data of first chunk
        if (writeSize > 0)
            pDocIndexOutput->write((const char*)bp,writeSize);
        pDChunk = pDChunk->next;
    }
    ///write rest data
    while (pDChunk)
    {
        pDocIndexOutput->write((const char*)pDChunk->data,pDChunk->size);
        pDChunk = pDChunk->next;
    }

    chunkDesc_.length += (pDocIndexOutput->getFilePointer() - oldDOff);

    ///write position posting
    VariantDataChunk* pPChunk = pInMemoryPosting->pLocList_->pHeadChunk_;
    while (pPChunk)
    {
        pPOutput->write((const char*)pPChunk->data,pPChunk->size);
        pPChunk = pPChunk->next;
    }

    ///merge skiplist
    SkipListReader* pSkipReader = pInMemoryPosting->getSkipListReader();
    if(pSkipReader)
    {
        pSkipListMerger_->setBasePoint(0, DOffsetStartForSkipping, postingDesc_.plength);
        pSkipListMerger_->addToMerge(pSkipReader,pInMemoryPosting->lastDocID(), nSkipIntervalBetweenBarrels_);
        nSkipIntervalBetweenBarrels_ = pInMemoryPosting->docFreq() - pSkipReader->getNumSkipped();
    }
    else
        nSkipIntervalBetweenBarrels_ += pInMemoryPosting->docFreq();

    ///update descriptors
    postingDesc_.ctf += pInMemoryPosting->nCTF_;
    postingDesc_.df += pInMemoryPosting->nDF_;
    postingDesc_.length = chunkDesc_.length;
    postingDesc_.plength += pInMemoryPosting->getPPostingLen();
    chunkDesc_.lastdocid = pInMemoryPosting->nLastDocID_;
}

void PostingMerger::mergeWith(OnDiskPosting* pOnDiskPosting)
{
    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    if (bFirstPosting_)///first posting
    {
        reset();
        if(Posting::skipInterval_ == 0)
        {
            termInfo_.docPointer_ = pDOutput->getFilePointer();
            termInfo_.skipPointer_ = -1;
        }
        else
            termInfo_.skipPointer_ = pDOutput->getFilePointer();
        termInfo_.positionPointer_ = pPOutput->getFilePointer();
		
        nPPostingLength_ = 0;
        bFirstPosting_ = false;

        ///save position offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }

    IndexOutput* pDocIndexOutput = 0;

    if(Posting::skipInterval_ == 0)
    {
        pDocIndexOutput = pDOutput;
    }
    else
        pDocIndexOutput = pTmpPostingOutput_;

    fileoffset_t oldDOff = pDocIndexOutput->getFilePointer();
    docid_t firstDocID = pDInput->readVInt() - chunkDesc_.lastdocid;

    ///DPosting start for skip
    int32_t DOffsetStartForSkipping = 0;
    if(chunkDesc_.lastdocid == 0)
        DOffsetStartForSkipping = 0;
    else
        DOffsetStartForSkipping = postingDesc_.length - (pDocIndexOutput->getVIntLength(firstDocID + chunkDesc_.lastdocid)
                                                                   - pDocIndexOutput->getVIntLength(firstDocID));
	
    pDocIndexOutput->writeVInt(firstDocID);///write first doc id
    int64_t writeSize = pOnDiskPosting->postingDesc_.length - pDocIndexOutput->getVIntLength(firstDocID + chunkDesc_.lastdocid);
    if (writeSize > 0)
        pDocIndexOutput->write(pDInput,writeSize);
    chunkDesc_.length += (pDocIndexOutput->getFilePointer() - oldDOff);

    ///write position posting
    pPOutput->write(pPInput,pOnDiskPosting->nPPostingLength_);

    ///merge skiplist
    SkipListReader* pSkipReader = pOnDiskPosting->getSkipListReader();

    if(pSkipReader)
    {
        pSkipListMerger_->setBasePoint(0, DOffsetStartForSkipping, postingDesc_.plength);
        pSkipListMerger_->addToMerge(pSkipReader,pOnDiskPosting->chunkDesc_.lastdocid, nSkipIntervalBetweenBarrels_);
        nSkipIntervalBetweenBarrels_ = pOnDiskPosting->docFreq() - pSkipReader->getNumSkipped();
    }
    else
        nSkipIntervalBetweenBarrels_ += pOnDiskPosting->docFreq();

    ///update descriptors
    postingDesc_.ctf += pOnDiskPosting->postingDesc_.ctf;
    postingDesc_.df += pOnDiskPosting->postingDesc_.df;
    postingDesc_.length = chunkDesc_.length; ///currently,it's only one chunk
    postingDesc_.plength += pOnDiskPosting->nPPostingLength_;
    chunkDesc_.lastdocid = pOnDiskPosting->chunkDesc_.lastdocid;
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

    if (bFirstPosting_)///first posting
    {
        reset();
        if(Posting::skipInterval_ == 0)
        {
            termInfo_.docPointer_ = pDOutput->getFilePointer();
            termInfo_.skipPointer_ = -1;
        }
        else
            termInfo_.skipPointer_ = pDOutput->getFilePointer();
        termInfo_.positionPointer_ = pPOutput->getFilePointer();
		
        nPPostingLength_ = 0;
        bFirstPosting_ = false;

        ///save position offset
        postingDesc_.poffset = pPOutput->getFilePointer();
    }

    IndexOutput* pDocIndexOutput = 0;

    if(Posting::skipInterval_ == 0)
    {
        pDocIndexOutput = pDOutput;
    }
    else
        pDocIndexOutput = pTmpPostingOutput_;
	
    fileoffset_t oldDOff = pDocIndexOutput->getFilePointer();
    fileoffset_t oldPOff = pPOutput->getFilePointer();


    while (nODDF > 0)
    {
        nDocID += pDInput->readVInt();
        nTF = pDInput->readVInt();
        if(!pFilter->test((size_t)nDocID))///the document has not been deleted
        {
            pDocIndexOutput->writeVInt(nDocID - nDocIDPrev);
            pDocIndexOutput->writeVInt(nTF);

            nDocIDPrev = nDocID;
            nPCount += nTF;
            nDF++;
            nLastDocID = nDocID;
            if(pSkipListMerger_)
            {
                pSkipListMerger_->setBasePoint(0, postingDesc_.length, postingDesc_.plength);
                pSkipListMerger_->addSkipPoint(nLastDocID,pDocIndexOutput->getFilePointer(),pPOutput->getFilePointer()-postingDesc_.poffset);
            }
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

    chunkDesc_.length += (pDocIndexOutput->getFilePointer() - oldDOff);

    ///update descriptors
    postingDesc_.ctf += nCTF;
    postingDesc_.df += nDF;
    postingDesc_.length = chunkDesc_.length; ///currently,it's only one chunk 
    postingDesc_.plength += pPOutput->getFilePointer() - oldPOff;
    chunkDesc_.lastdocid = nLastDocID;
}

fileoffset_t PostingMerger::endMerge()
{
    bFirstPosting_ = true;
    if (postingDesc_.df <= 0)
        return -1;

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();

    termInfo_.docFreq_ = postingDesc_.df;
    termInfo_.ctf_ = postingDesc_.ctf;
    termInfo_.lastDocID_ = chunkDesc_.lastdocid;
    termInfo_.positionPostingLen_ = pPOutput->getFilePointer() - postingDesc_.poffset;

    if(Posting::skipInterval_ == 0)
    {
        termInfo_.skipLevel_ = 0;
        termInfo_.docPostingLen_ = postingDesc_.length;
    }
    else
    {
        pSkipListMerger_->write(pDOutput);
        termInfo_.skipLevel_ = pSkipListMerger_->getNumLevels();
        termInfo_.docPointer_ = pDOutput->getFilePointer();

        pTmpPostingOutput_->flush();

        if(!pTmpPostingInput_)
            pTmpPostingInput_ = pOutputDescriptor_->getDirectory()->openInput(tmpPostingName_);
        else
            reinterpret_cast<FSIndexInput*>(pTmpPostingInput_)->reopen();
        pDOutput->write(pTmpPostingInput_, pTmpPostingOutput_->length());

        termInfo_.docPostingLen_ = postingDesc_.length;
    }

    ///end write posting descriptor
    return termInfo_.docPointer_;
}

