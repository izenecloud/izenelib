#include <ir/index_manager/index/PostingMerger.h>
#include <ir/index_manager/index/RTPostingWriter.h>
#include <ir/index_manager/index/EPostingWriter.h>
#include <ir/index_manager/store/FSIndexOutput.h>
#include <ir/index_manager/store/FSIndexInput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

PostingMerger::PostingMerger(int skipInterval, int maxSkipLevel, CompressionType compressType, bool optimize, MemCache* pMemCache)
        :skipInterval_(skipInterval)
        ,maxSkipLevel_(maxSkipLevel)
        ,compressType_(compressType)
        ,pOutputDescriptor_(NULL)
        ,pTmpPostingOutput_(NULL)
        ,pTmpPostingInput_(NULL)
        ,nPPostingLength_(0)
        ,nSkipIntervalBetweenBarrels_(0)
        ,bFirstPosting_(true)
        ,pSkipListMerger_(NULL)
        ,pMemCache_(NULL)
        ,compressedPos_(NULL)
        ,block_buffer_(NULL)
        ,current_block_id_(0)
        ,optimize_(optimize)
        ,ownMemCache_(false)
{
    if(optimize_||(compressType_ != BYTEALIGN))
        pMemCache_ = pMemCache;
    else
    {
        pMemCache_ = new MemCache(POSTINGMERGE_BUFFERSIZE*512);
        ownMemCache_ = true;
    }

    init();
    reset();
}

PostingMerger::~PostingMerger()
{
    if(pMemCache_&&ownMemCache_)
        delete pMemCache_;
    if(pSkipListMerger_)
        delete pSkipListMerger_;
    if(pTmpPostingOutput_)
    {
        delete pTmpPostingInput_;
        delete pTmpPostingOutput_;
        pOutputDescriptor_->getDirectory()->deleteFile(tmpPostingName_);
    }
    delete [] compressedPos_;
    delete [] positions_;
    if(block_buffer_) delete [] block_buffer_;
    delete pPosDataPool_;
    delete pDocFreqDataPool_;
    delete pFixedSkipListWriter_;
    delete pSkipListWriter_;
	
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

    if(pSkipListWriter_)
    {
        pSkipListWriter_->reset();
    }

    pFixedSkipListWriter_->reset();
    pDocFreqDataPool_->reset();
    pPosDataPool_->reset();
    chunk_.reset();

    doc_ids_offset_ = 0;
    position_buffer_pointer_ = 0;
    blockEncoder_.reset();
    //current_block_id_ = 0;
}

void PostingMerger::init()
{
    if(skipInterval_ > 0)
        pSkipListMerger_ = new SkipListMerger(skipInterval_,maxSkipLevel_,pMemCache_);

    compressed_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
    compressedPos_ = new uint32_t[compressed_position_buffer_size_];
    curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE << 1;
    positions_ = new uint32_t[curr_position_buffer_size_];
    pFixedSkipListWriter_ = new FixedBlockSkipListWriter(pMemCache_);
    pSkipListWriter_ = new SkipListWriter(skipInterval_,maxSkipLevel_,pMemCache_);
    pPosDataPool_ = new ChunkDataPool(pMemCache_) ;
    pDocFreqDataPool_ = new ChunkDataPool(pMemCache_);
}

void PostingMerger::setOutputDescriptor(OutputDescriptor* pOutputDescriptor)
{
    pOutputDescriptor_ = pOutputDescriptor;
    tmpPostingName_ = pOutputDescriptor_->getBarrelName() + ".merge";
    pTmpPostingOutput_ = pOutputDescriptor_->getDirectory()->createOutput(tmpPostingName_);
}

void PostingMerger::mergeWith(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter)
{
    if(optimize_)
    {
        optimize(pOnDiskPosting, pFilter);
    }
    else
    {
        if(pFilter &&  pFilter->hasSmallThan((size_t)pOnDiskPosting->chunkDesc_.lastdocid))
            mergeWith_GC(pOnDiskPosting,pFilter);
        else
            mergeWith(pOnDiskPosting);
    }
}

void PostingMerger::mergeWith(MemPostingReader* pInMemoryPosting)
{
    ///flush last doc
    pInMemoryPosting->pPostingWriter_->flushLastDoc(true);

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();

    if (bFirstPosting_)///first posting
    {
        reset();
        if(skipInterval_ == 0)
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

    if(skipInterval_ == 0)
    {
        pDocIndexOutput = pDOutput;
    }
    else
        pDocIndexOutput = pTmpPostingOutput_;

    fileoffset_t oldDOff = pDocIndexOutput->getFilePointer();

    ///write chunk data, update the first doc id
    VariantDataChunk* pDChunk = pInMemoryPosting->pPostingWriter_->pDocFreqList_->pHeadChunk_;

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
    VariantDataChunk* pPChunk = pInMemoryPosting->pPostingWriter_->pLocList_->pHeadChunk_;
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
    postingDesc_.ctf += pInMemoryPosting->getCTF();
    postingDesc_.df += pInMemoryPosting->docFreq();
    postingDesc_.length = chunkDesc_.length;
    postingDesc_.plength += pInMemoryPosting->getPPostingLen();
    chunkDesc_.lastdocid = pInMemoryPosting->lastDocID();
}

void PostingMerger::mergeWith(RTDiskPostingReader* pOnDiskPosting)
{
    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    if (bFirstPosting_)///first posting
    {
        reset();
        if(skipInterval_ == 0)
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

    if(skipInterval_ == 0)
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

void PostingMerger::mergeWith_GC(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter)
{
    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    count_t nODDF = pOnDiskPosting->postingDesc_.df;
    if(nODDF <= 0)
        return;

    if (bFirstPosting_)///first posting
    {
        reset();
        if(skipInterval_ == 0)
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

    docid_t nDocID = 0;
    docid_t nLastDocID = chunkDesc_.lastdocid;
    freq_t	nTF = 0;
    count_t nCTF = 0;
    count_t nDF = 0;
    count_t nPCount = 0;

    IndexOutput* pDocIndexOutput = 0;

    if(skipInterval_ == 0)
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
            pDocIndexOutput->writeVInt(nDocID - nLastDocID);
            pDocIndexOutput->writeVInt(nTF);

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

void PostingMerger::mergeWith(BlockPostingReader* pPosting,BitVector* pFilter)
{
    IndexInput* pPInput = pPosting->pInputDescriptor_->getPPostingInput();
    if (bFirstPosting_)///first posting
    {
        reset();
        nPPostingLength_ = 0;
        bFirstPosting_ = false;
        if(! block_buffer_) block_buffer_ = new uint8_t[BLOCK_SIZE];
    }

    int num_docs_left = pPosting->df_;
    int prev_block_last_doc_id = 0;
    int size_of_positions = 0;

    BlockDecoder& blockDecoder = pPosting->blockDecoder_;
    ChunkDecoder& chunk = blockDecoder.chunk_decoder_;

    pPosting->advanceToNextBlock();
    while(pPosting->curr_block_id_ <= pPosting->last_block_id_)
    {
        while (blockDecoder.curr_chunk() < blockDecoder.num_chunks()) 
        {
            // Check if we previously decoded this chunk and decode if necessary.
            if (blockDecoder.curr_chunk_decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                int num_doc = std::min(CHUNK_SIZE, num_docs_left);
                chunk.reset(blockDecoder.curr_block_data(), num_doc);
                chunk.set_doc_freq_buffer(internal_doc_ids_buffer_, internal_freqs_buffer_);
                chunk.decodeDocIds();
                chunk.decodeFrequencies();

                size_of_positions = chunk.size_of_positions();
                ensure_pos_buffer(size_of_positions);
                chunk.set_pos_buffer(positions_ + position_buffer_pointer_);
                int size = pPInput->readVInt();
                ensure_compressed_pos_buffer(size>>2);
                pPInput->readBytes((uint8_t*)compressedPos_,size);
                chunk.decodePositions(compressedPos_);

                if(pFilter) chunk.post_process(pFilter);

                num_docs_left -= chunk.num_docs();
                int copySize = std::min(chunk.num_docs(), CHUNK_SIZE - doc_ids_offset_);
                memcpy(doc_ids_ + doc_ids_offset_, chunk.doc_ids(), copySize);
                memcpy(frequencies_+ doc_ids_offset_, chunk.frequencies(), copySize);
                doc_ids_offset_ += copySize;
                position_buffer_pointer_ += size_of_positions;
                if (doc_ids_offset_ == CHUNK_SIZE) 
                {
                    chunk_.encode(doc_ids_, frequencies_, positions_, CHUNK_SIZE);
                    if(!blockEncoder_.addChunk(chunk_))
                    {
                        blockEncoder_.getBlockBytes(block_buffer_);
                        pTmpPostingOutput_->writeBytes(block_buffer_, BLOCK_SIZE);
                        ++current_block_id_;

                        pFixedSkipListWriter_->addSkipPoint(prev_block_last_doc_id,blockEncoder_.num_doc_ids(),pPosDataPool_->getLength());
                        blockEncoder_.reset();
                        blockEncoder_.addChunk(chunk_);
                    }
                    pPosDataPool_->addPOSChunk(chunk_);

                    doc_ids_offset_ = 0;
                    position_buffer_pointer_ = 0;

                    int left = chunk.num_docs() - copySize;
                    if(left > 0)
                    {
                        chunk.set_curr_document_offset(copySize);
                        chunk.updatePositionOffset();
                        memcpy(doc_ids_, chunk.doc_ids(), left*sizeof(uint32_t));
                        memcpy(frequencies_, chunk.frequencies(), left*sizeof(uint32_t));
                        memmove (positions_, positions_ + position_buffer_pointer_ + chunk.curr_position_offset(), 
                                                           (chunk.size_of_positions() - chunk.curr_position_offset())*sizeof(uint32_t));
                        position_buffer_pointer_ = chunk.size_of_positions() - chunk.curr_position_offset();
                        doc_ids_offset_ = left;
                        chunk.set_curr_document_offset(0);
                    }
                }
            }
            blockDecoder.advance_curr_chunk();
            chunk.set_decoded(false);
        } 
        prev_block_last_doc_id = blockDecoder.chunk_last_doc_id(blockDecoder.num_chunks() - 1);      
        pPosting->advanceToNextBlock();
    }


    ///update descriptors
    postingDesc_.ctf += pPosting->getCTF();
    postingDesc_.df += pPosting->docFreq();
}

void PostingMerger::mergeWith(ChunkPostingReader* pPosting,BitVector* pFilter)
{
    IndexInput*	pDInput = pPosting->pInputDescriptor_->getDPostingInput();
    IndexInput*	pPInput = pPosting->pInputDescriptor_->getPPostingInput();

    if (bFirstPosting_)///first posting
    {
        reset();
	
        nPPostingLength_ = 0;
        bFirstPosting_ = false;
    }

    int num_docs_left = pPosting->df_;
    int size_of_positions = 0;

    ChunkDecoder& chunk = pPosting->chunkDecoder_;
    int doc_size = 0;
    int tf_size = 0;

    while(num_docs_left)
    {
        doc_size = pDInput->readVInt();
        pDInput->readBytes((uint8_t*)compressedBuffer_,doc_size);
        tf_size = pDInput->readVInt();
        pDInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);

        int num_doc = std::min(CHUNK_SIZE, num_docs_left);
        chunk.reset(compressedBuffer_, num_doc);
        chunk.set_doc_freq_buffer(internal_doc_ids_buffer_, internal_freqs_buffer_);
        chunk.decodeDocIds();
        chunk.decodeFrequencies();

        size_of_positions = chunk.size_of_positions();
        ensure_pos_buffer(size_of_positions);
        chunk.set_pos_buffer(positions_ + position_buffer_pointer_);
        int size = pPInput->readVInt();
        ensure_compressed_pos_buffer(size>>2);
        pPInput->readBytes((uint8_t*)compressedPos_,size);
        chunk.decodePositions(compressedPos_);

        num_docs_left -= chunk.num_docs();

        if(pFilter) chunk.post_process(pFilter);

        int copySize = std::min(chunk.num_docs(), CHUNK_SIZE - doc_ids_offset_);
        memcpy(doc_ids_ + doc_ids_offset_, chunk.doc_ids(), copySize*sizeof(uint32_t));
        memcpy(frequencies_+ doc_ids_offset_, chunk.frequencies(), copySize*sizeof(uint32_t));
        doc_ids_offset_ += copySize;
        position_buffer_pointer_ += size_of_positions;
        if (doc_ids_offset_ == CHUNK_SIZE) 
        {
            chunk_.encode(doc_ids_, frequencies_, positions_, ChunkEncoder::kChunkSize);
            pDocFreqDataPool_->addDFChunk(chunk_);
            pPosDataPool_->addPOSChunk(chunk_);
            pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());

            doc_ids_offset_ = 0;
            position_buffer_pointer_ = 0;

            int left = chunk.num_docs() - copySize;
            if(left > 0)
            {
                chunk.set_curr_document_offset(copySize);
                chunk.updatePositionOffset();
                memcpy(doc_ids_, chunk.doc_ids(), left*sizeof(uint32_t));
                memcpy(frequencies_, chunk.frequencies(), left*sizeof(uint32_t));
                memmove (positions_, positions_ + position_buffer_pointer_ + chunk.curr_position_offset(), 
                                                    (chunk.size_of_positions() - chunk.curr_position_offset())*sizeof(uint32_t));
                position_buffer_pointer_ = chunk.size_of_positions() - chunk.curr_position_offset();
                doc_ids_offset_ = left;
                chunk.set_curr_document_offset(0);
            }
        }

    }

    ///update descriptors
    postingDesc_.ctf += pPosting->getCTF();
    postingDesc_.df += pPosting->docFreq();
}

void PostingMerger::optimize(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter)
{
    if(compressType_ == BLOCK) optimize_to_Block(pOnDiskPosting, pFilter);
    else optimize_to_Chunk(pOnDiskPosting, pFilter);
}

void PostingMerger::optimize_to_Block(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter)
{
    IndexInput*	pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput*	pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    count_t nODDF = pOnDiskPosting->postingDesc_.df;
    if(nODDF <= 0)
        return;

    if (bFirstPosting_)///first posting
    {
        reset();
        bFirstPosting_ = false;
    }

    docid_t nDocID = 0;
    freq_t	nTF = 0;
    count_t nCTF = 0;
    count_t nDF = 0;
    count_t nPCount = 0;

    while (nODDF > 0)
    {
        nDocID += pDInput->readVInt();
        nTF = pDInput->readVInt();

        if(!pFilter||!pFilter->test((size_t)nDocID)) ///the document has not been deleted
        {
            doc_ids_[doc_ids_offset_] = nDocID;
            frequencies_[doc_ids_offset_] = nTF;
            nPCount += nTF;
            nDF ++;
            nCTF += nTF;
            ensure_pos_buffer(nPCount);
            uint32_t pos = 0;
            while(nTF > 0)
            {
                pos += pPInput->readVInt();
                positions_[position_buffer_pointer_++] = pos;
                nTF --;
            }

            if (doc_ids_offset_ == ChunkEncoder::kChunkSize - 1) 
            {
                chunk_.encode(doc_ids_, frequencies_, positions_, ChunkEncoder::kChunkSize);
                if(!blockEncoder_.addChunk(chunk_))
                {
                    if(! block_buffer_) block_buffer_ = new uint8_t[BLOCK_SIZE];
                    blockEncoder_.getBlockBytes(block_buffer_);
                    pTmpPostingOutput_->writeBytes(block_buffer_, BLOCK_SIZE);
                    ++current_block_id_;
		
                    pFixedSkipListWriter_->addSkipPoint(blockEncoder_.last_doc_id_,blockEncoder_.num_doc_ids(),pPosDataPool_->getLength());
                    blockEncoder_.reset();
                    blockEncoder_.addChunk(chunk_);
                }
                pPosDataPool_->addPOSChunk(chunk_);
		
                doc_ids_offset_ = 0;
                position_buffer_pointer_ = 0;
                nPCount = 0;
            }
        }
        else ///this document has been deleted
        {
            while (nTF > 0)
            {		
                pPInput->readVInt();
                nTF--;							
            }
        }
    }	

    ///update descriptors
    postingDesc_.ctf += nCTF;
    postingDesc_.df += nDF;
    chunkDesc_.lastdocid = nDocID;
}

void PostingMerger::optimize_to_Chunk(RTDiskPostingReader* pOnDiskPosting,BitVector* pFilter)
{
    IndexInput* pDInput = pOnDiskPosting->getInputDescriptor()->getDPostingInput();
    IndexInput* pPInput = pOnDiskPosting->getInputDescriptor()->getPPostingInput();

    count_t nODDF = pOnDiskPosting->postingDesc_.df;
    if(nODDF <= 0)
        return;

    if (bFirstPosting_)///first posting
    {
        reset();
        bFirstPosting_ = false;
    }

    docid_t nDocID = 0;
    freq_t nTF = 0;
    count_t nCTF = 0;
    count_t nDF = 0;
    count_t nPCount = 0;

    while (nODDF > 0)
    {
        nDocID += pDInput->readVInt();
        nTF = pDInput->readVInt();
        if(!pFilter||!pFilter->test((size_t)nDocID))
        {
            doc_ids_[doc_ids_offset_] = nDocID;
            frequencies_[doc_ids_offset_] = nTF;
            nPCount += nTF;
            nDF ++;
            nCTF += nTF;
            ensure_pos_buffer(nPCount);
            uint32_t pos = 0;
            while(nTF > 0)
            {
                pos += pPInput->readVInt();
                positions_[position_buffer_pointer_++] = pos;
                nTF --;
            }

            if (doc_ids_offset_ == ChunkEncoder::kChunkSize - 1) 
            {
                chunk_.encode(doc_ids_, frequencies_, positions_, ChunkEncoder::kChunkSize);

                pDocFreqDataPool_->addDFChunk(chunk_);
                pPosDataPool_->addPOSChunk(chunk_);
                pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
	
                doc_ids_offset_ = 0;
                position_buffer_pointer_ = 0;
                nPCount = 0;
            }
        }
        else ///this document has been deleted
        {
            while (nTF > 0)
            {		
                pPInput->readVInt();
                nTF--;							
            }
        }
    }	

    ///update descriptors
    postingDesc_.ctf += nCTF;
    postingDesc_.df += nDF;
    chunkDesc_.lastdocid = nDocID;
}

fileoffset_t PostingMerger::endMerge()
{
    switch(compressType_)
    {
    case BYTEALIGN:
        return endMerge_ByteAlign();
    case BLOCK:
        return endMerge_Block();
    case CHUNK:
        return endMerge_Chunk();
    default:
        assert(false);
    }
}

fileoffset_t PostingMerger::endMerge_ByteAlign()
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

    if(skipInterval_ == 0)
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


fileoffset_t PostingMerger::endMerge_Block()
{
    bFirstPosting_ = true;
    if (postingDesc_.df <= 0)
        return -1;
    if(doc_ids_offset_> 0)
    {
        chunk_.encode(doc_ids_, frequencies_, positions_, doc_ids_offset_);
        if(!blockEncoder_.addChunk(chunk_))
        {
            blockEncoder_.getBlockBytes(block_buffer_);
            pTmpPostingOutput_->writeBytes(block_buffer_, BLOCK_SIZE);
            ++current_block_id_;
		
            pFixedSkipListWriter_->addSkipPoint(blockEncoder_.last_doc_id_, blockEncoder_.num_doc_ids(), pPosDataPool_->getLength());
            blockEncoder_.reset();
            blockEncoder_.addChunk(chunk_);
    	}
    	blockEncoder_.getBlockBytes(block_buffer_);
    	pTmpPostingOutput_->writeBytes(block_buffer_, BLOCK_SIZE);
    	++current_block_id_;
    	pPosDataPool_->addPOSChunk(chunk_);
    	pFixedSkipListWriter_->addSkipPoint(chunkDesc_.lastdocid, blockEncoder_.num_doc_ids(),pPosDataPool_->getLength());
    	blockEncoder_.reset();
    }


    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();

    termInfo_.docFreq_ = postingDesc_.df;
    termInfo_.ctf_ = postingDesc_.ctf;
    termInfo_.lastDocID_ = chunkDesc_.lastdocid;
    termInfo_.positionPostingLen_ = pPOutput->getFilePointer() - postingDesc_.poffset;

    termInfo_.skipPointer_ = pDOutput->getFilePointer();
    pFixedSkipListWriter_->write(pDOutput);	///write skip list data

    ///save the offset of posting descriptor
    termInfo_.docPointer_ = pDOutput->getFilePointer();

    pTmpPostingOutput_->flush();
	
    if(!pTmpPostingInput_)
        pTmpPostingInput_ = pOutputDescriptor_->getDirectory()->openInput(tmpPostingName_);
    else
        reinterpret_cast<FSIndexInput*>(pTmpPostingInput_)->reopen();
    pDOutput->write(pTmpPostingInput_, pTmpPostingOutput_->length());
	
    termInfo_.docPostingLen_ = postingDesc_.length;

    uint32_t num_blocks = termInfo_.docPostingLen_ / BLOCK_SIZE;
    ///we reuse "skiplevel " to store the start block id for this posting. 
    termInfo_.skipLevel_ = current_block_id_ - num_blocks; 

    termInfo_.positionPointer_ = pPOutput->getFilePointer();

    ///write position posting data
    pPosDataPool_->write(pPOutput);

    termInfo_.positionPostingLen_ = pPOutput->getFilePointer() - termInfo_.positionPointer_;

    ///end write posting descriptor
    return termInfo_.docPointer_;
}


fileoffset_t PostingMerger::endMerge_Chunk()
{
    bFirstPosting_ = true;

    if (postingDesc_.df <= 0)
        return -1;

    if(doc_ids_offset_> 0)
    {
        chunk_.encode(doc_ids_, frequencies_, positions_, doc_ids_offset_);
        pDocFreqDataPool_->addDFChunk(chunk_);
        pPosDataPool_->addPOSChunk(chunk_);
    }
    if(skipInterval_ && postingDesc_.df > 0 && postingDesc_.df % skipInterval_ == 0)
    {
        if(pSkipListWriter_)
        {
            pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
        }
    }

    IndexOutput* pDOutput = pOutputDescriptor_->getDPostingOutput();
    IndexOutput* pPOutput = pOutputDescriptor_->getPPostingOutput();

    if( pSkipListWriter_ && pSkipListWriter_->getNumLevels() > 0) ///nDF_ > SkipInterval
    {
        termInfo_.skipLevel_ = pSkipListWriter_->getNumLevels();
        termInfo_.skipPointer_ = pDOutput->getFilePointer();
        pSkipListWriter_->write(pDOutput);	///write skip list data
    }
    else
    {
        termInfo_.skipPointer_ = -1;
        termInfo_.skipLevel_ = 0;
    }

    pDocFreqDataPool_->truncTailChunk();
    pPosDataPool_->truncTailChunk();

    termInfo_.docFreq_ = postingDesc_.df;
    termInfo_.ctf_ = postingDesc_.ctf;
    termInfo_.lastDocID_ = chunkDesc_.lastdocid;

    ///save the offset of posting descriptor
    termInfo_.docPointer_ = pDOutput->getFilePointer();
    ///write doc posting data
    pDocFreqDataPool_->write(pDOutput);

    termInfo_.docPostingLen_ = pDOutput->getFilePointer() - termInfo_.docPointer_;

    termInfo_.positionPointer_ = pPOutput->getFilePointer();

    ///write position posting data
    pPosDataPool_->write(pPOutput);

    termInfo_.positionPostingLen_ = pPOutput->getFilePointer() - termInfo_.positionPointer_;

    ///end write posting descriptor
    return termInfo_.docPointer_;
}

}

NS_IZENELIB_IR_END

