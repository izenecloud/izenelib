#include <ir/index_manager/index/EPostingReader.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/FixedBlockSkipListReader.h>
#include <ir/index_manager/index/SkipListReader.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

#include <math.h>
#include <algorithm>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

//////////////////////////////////////////////////////////////////////////
///BlockPostingReader

BlockPostingReader::BlockPostingReader(InputDescriptor* pInputDescriptor, const TermInfo& termInfo,IndexType type)
        :pInputDescriptor_(pInputDescriptor)
        ,pSkipListReader_(0)
        ,pListingCache_(0)
        ,pDocFilter_(0)
        ,urgentBuffer_(0)
        ,compressedPos_(0)
{
    reset(termInfo);
    if(type == WORD_LEVEL)
    {
        curr_pos_buffer_size_ = INIT_POS_CHUNK_SIZE;
        compressedPos_  = (uint32_t*)malloc(curr_pos_buffer_size_*sizeof(uint32_t));
    }
}

BlockPostingReader::~BlockPostingReader()
{
    if(pSkipListReader_) delete pSkipListReader_;
    if(urgentBuffer_) delete[] urgentBuffer_;
    if(compressedPos_) free(compressedPos_);
}

void BlockPostingReader::reset(const TermInfo& termInfo)
{
    curr_block_id_ = termInfo.skipLevel_; ///we reuse "skiplevel" to store start block id
    start_block_id_ = termInfo.skipLevel_;
    total_block_num_ = termInfo.docPostingLen_/BLOCK_SIZE;
    last_block_id_ = curr_block_id_ + total_block_num_ - 1;

    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    dlength_ = termInfo.docPostingLen_;
    df_ = termInfo.docFreq_;
    ctf_ = termInfo.ctf_;
    poffset_ = termInfo.positionPointer_;
    plength_ = termInfo.positionPostingLen_;
    last_doc_id_ = termInfo.lastDocID_;
    num_docs_left_ = df_;
    num_docs_decoded_ = 0;
    last_pos_decoded_ = 0;
    num_pos_decoded_ = 0;
    num_pos_chunk_skipped_ = 0;

    prev_block_last_doc_id_ = 0;

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    if((termInfo.docFreq_ >= 4096)&&(termInfo.skipPointer_ != -1))
    {
        pDPInput->seek(termInfo.skipPointer_);
        pSkipListReader_ = new FixedBlockSkipListReader(pDPInput, start_block_id_);
    }

    pDPInput->seek(postingOffset_);

    IndexInput* pPPInput = pInputDescriptor_->getPPostingInput();
    if (pPPInput)
    {
        pPPInput->reset();
        pPPInput->seek(termInfo.positionPointer_);
    }

    if(pListingCache_)
    {
        pListingCache_->applyBlocks(curr_block_id_, last_block_id_);
    }
}

void BlockPostingReader::advanceToNextBlock() 
{
    if(pListingCache_)
    {
        pListingCache_->freeBlock(curr_block_id_);
        ++curr_block_id_;

        IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
        uint32_t* blockBuffer = pListingCache_->getBlock(pDPInput, curr_block_id_);
        if(!blockBuffer)
        {
            if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
            blockBuffer = urgentBuffer_;
        }
        blockDecoder_.init(curr_block_id_, blockBuffer);
    }
    else
    {
        if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
        blockDecoder_.init(curr_block_id_, urgentBuffer_);
    }
}

void BlockPostingReader::skipToBlock(size_t targetBlock) 
{
    if(pListingCache_)
    {   
        for(; curr_block_id_ <= targetBlock; ++curr_block_id_)
        {
            pListingCache_->freeBlock(curr_block_id_); 
        }

        IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
        uint32_t* blockBuffer = pListingCache_->getBlock(pDPInput, curr_block_id_);
        if(!blockBuffer)
        {
            if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
            blockBuffer = urgentBuffer_;
        }
        blockDecoder_.init(curr_block_id_, blockBuffer);
    }
    else
    {
        if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
        blockDecoder_.init(curr_block_id_, urgentBuffer_);
    }
}

docid_t BlockPostingReader::decodeTo(docid_t target)
{
    if(pSkipListReader_)
    {
        docid_t lastDocID = pSkipListReader_->skipTo(target);
        if(lastDocID > last_doc_id_)
        {
            seekTo(pSkipListReader_);
        }
        size_t currBlock = pSkipListReader_->getBlockId();
        skipToBlock(currBlock);
    }

    int curr_chunk_num = blockDecoder_.curr_chunk();
    if (curr_chunk_num < blockDecoder_.num_chunks()) 
    {
        // Check the last doc id of the chunk against the current doc id
        // we're looking for and skip the chunk if possible.
        if (target <= blockDecoder_.chunk_last_doc_id(curr_chunk_num)) 
        {
            ChunkDecoder* chunk = blockDecoder_.curr_chunk_decoder();

            // Check if we previously decoded this chunk and decode if necessary.
            if (blockDecoder_.curr_chunk_decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk->reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk->update_prev_decoded_doc_id(prev_block_last_doc_id_);
                chunk->decodeDocIds();
                chunk->decodeFrequencies();

                // We need offset from previous chunk if this is not the first chunk in the list.
                if (curr_chunk_num > 0) 
                {
                    uint32_t doc_id_offset = blockDecoder_.chunk_last_doc_id(curr_chunk_num - 1);
                    chunk->update_prev_decoded_doc_id(doc_id_offset);
                }
            }

            uint32_t curr_doc_id = chunk->prev_decoded_doc_id();

            // The current document offset was the last docID processed, so we increment by 1 in order to not process it again. But not for the first chunk processed.
            int curr_document_offset = chunk->curr_document_offset() == -1 ? 0 : chunk->curr_document_offset() + 1;
            for (int k = curr_document_offset; k < chunk->num_docs(); ++k) 
            {
                // Found the docID we're looking for.
                if (curr_doc_id >= target) 
                {
                    chunk->set_curr_document_offset(k);
                    chunk->set_prev_decoded_doc_id(curr_doc_id);
                    return curr_doc_id;
                }
            }
        }

        blockDecoder_.advance_curr_chunk();
        blockDecoder_.curr_chunk_decoder()->set_decoded(false);

        // Can update the number of documents left to process after processing the complete chunk.
        num_docs_left_ -= CHUNK_SIZE;
        
    } 
    else 
    {
        prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
        advanceToNextBlock();
    }
    return BAD_DOCID;
}

void BlockPostingReader::seekTo(FixedBlockSkipListReader* pSkipListReader)
{
    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();
    pDPostingInput->seek(postingOffset_ + pSkipListReader->getOffset());
    last_doc_id_ = pSkipListReader->getDoc();
    num_docs_decoded_ = pSkipListReader->getNumSkipped();
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
    if(pPPostingInput)
    {
        pPPostingInput->seek(poffset_ + pSkipListReader->getPOffset());
        last_pos_decoded_ = 0;///reset position
    }
}

int32_t BlockPostingReader::decodeNext(uint32_t* pPosting,int32_t length)
{
    int32_t left = df_ - num_docs_decoded_;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length>>1);

    if (length > left*2)
        length = left*2;
    left = (length>>1);

    int decodedDoc = 0;
    while(left > 0)
    {
        int curr_chunk_num = blockDecoder_.curr_chunk();
        if (curr_chunk_num < blockDecoder_.num_chunks()) 
        {
            ChunkDecoder* chunk = blockDecoder_.curr_chunk_decoder();

            // Check if we previously decoded this chunk and decode if necessary.
            if (blockDecoder_.curr_chunk_decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk->reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk->set_doc_freq_buffer(pDoc,pFreq);
                chunk->update_prev_decoded_doc_id(prev_block_last_doc_id_);
                chunk->decodeDocIds();
                chunk->decodeFrequencies();
                if(!pDocFilter_) chunk->post_process(pDocFilter_);
            }

            blockDecoder_.advance_curr_chunk();
            blockDecoder_.curr_chunk_decoder()->set_decoded(false);

            // Can update the number of documents left to process after processing the complete chunk.
            num_docs_left_ -= chunk->num_docs();;
            left -= chunk->num_docs();
            decodedDoc += chunk->num_docs();
        } 
        else 
        {
            prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
            advanceToNextBlock();
        }        
    }

    doc_freq_decode_buffer_ = pPosting;

    return decodedDoc;
}

int32_t BlockPostingReader::decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& pLength)
{
    int32_t left = df_ - num_docs_decoded_;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length>>1);
    if (length > left*2)
        length = left*2;
    left = (length>>1);

    IndexInput*	pPPostingInput = pInputDescriptor_->getPPostingInput();

    int decodedDoc = 0;
    int decompressed_pos = 0;
    int size_of_positions = 0;

    while(left > 0)
    {
        int curr_chunk_num = blockDecoder_.curr_chunk();
        if (curr_chunk_num < blockDecoder_.num_chunks()) 
        {
            ChunkDecoder* chunk = blockDecoder_.curr_chunk_decoder();

            // Check if we previously decoded this chunk and decode if necessary.
            if (blockDecoder_.curr_chunk_decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk->reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk->set_doc_freq_buffer(pDoc,pFreq);
                chunk->update_prev_decoded_doc_id(prev_block_last_doc_id_);
                chunk->decodeDocIds();
                chunk->decodeFrequencies();

                size_of_positions = chunk->size_of_positions();
                if((pLength - decompressed_pos) < size_of_positions) growPosBuffer(pPPosting, pLength);
                chunk->set_pos_buffer(pPPosting + decompressed_pos);

                int size = pPPostingInput->readVInt();
                ensure_pos_buffer(size);
                pPPostingInput->readBytes((uint8_t*)compressedPos_,size*sizeof(uint32_t));
                chunk->decodePositions(compressedPos_);
                if(!pDocFilter_) chunk->post_process(pDocFilter_);
            }

            blockDecoder_.advance_curr_chunk();
            blockDecoder_.curr_chunk_decoder()->set_decoded(false);

            // Can update the number of documents left to process after processing the complete chunk.
            num_docs_left_ -= chunk->num_docs();;
            left -= chunk->num_docs();
            decodedDoc += chunk->num_docs();
            if(chunk->has_deleted_doc()) size_of_positions = chunk->size_of_positions();
            decompressed_pos += size_of_positions;
        } 
        else 
        {
            prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
            advanceToNextBlock();
        }        
    }

    doc_freq_decode_buffer_ = pPosting;

    return decodedDoc;

}

bool BlockPostingReader::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    return true;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    return true;
}

void BlockPostingReader::resetPosition()
{
    last_pos_decoded_ = 0;
}

void BlockPostingReader::reset()
{
    postingOffset_ = 0;

    dlength_ = 0;
    df_ = 0;
    ctf_ = 0;
    poffset_ = 0;
    plength_ = 0;
    last_doc_id_ = 0;
}




//////////////////////////////////////////////////////////////////////////
///ChunkPostingReader

ChunkPostingReader::ChunkPostingReader(int skipInterval, int maxSkipLevel, InputDescriptor* pInputDescriptor, const TermInfo& termInfo,IndexType type)
        :skipInterval_(skipInterval)
        ,maxSkipLevel_(maxSkipLevel)
        ,pInputDescriptor_(pInputDescriptor)
        ,pSkipListReader_(0)
        ,pDocFilter_(0)
        ,compressedPos_(0)
{
    reset(termInfo);
    if(type == WORD_LEVEL)
    {
        curr_pos_buffer_size_ = INIT_POS_CHUNK_SIZE;
        compressedPos_  = (uint32_t*)malloc(curr_pos_buffer_size_*sizeof(uint32_t));
    }
}

ChunkPostingReader::~ChunkPostingReader()
{
    if(pSkipListReader_) delete pSkipListReader_;
    if(compressedPos_) free(compressedPos_);
}

void ChunkPostingReader::reset(const TermInfo& termInfo)
{
    curr_block_id_ = termInfo.skipLevel_; ///we reuse "skiplevel" to store start block id
    start_block_id_ = termInfo.skipLevel_;
    total_block_num_ = termInfo.docPostingLen_/BLOCK_SIZE;
    last_block_id_ = curr_block_id_ + total_block_num_ - 1;

    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    dlength_ = termInfo.docPostingLen_;
    df_ = termInfo.docFreq_;
    ctf_ = termInfo.ctf_;
    poffset_ = termInfo.positionPointer_;
    plength_ = termInfo.positionPostingLen_;
    last_doc_id_ = termInfo.lastDocID_;
    num_docs_left_ = df_;
    num_docs_decoded_ = 0;
    last_pos_decoded_ = 0;
    num_pos_decoded_ = 0;
    num_pos_chunk_skipped_ = 0;

    prev_block_last_doc_id_ = 0;

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    if((termInfo.docFreq_ >= 4096)&&(termInfo.skipPointer_ != -1))
    {
        pDPInput->seek(termInfo.skipPointer_);
        pSkipListReader_ = new SkipListReader(pDPInput, skipInterval_, maxSkipLevel_);
    }

    pDPInput->seek(postingOffset_);

    IndexInput* pPPInput = pInputDescriptor_->getPPostingInput();
    if (pPPInput)
    {
        pPPInput->reset();
        pPPInput->seek(termInfo.positionPointer_);
    }

}

docid_t ChunkPostingReader::decodeTo(docid_t target)
{
    docid_t lastDocID = 0;
    if(pSkipListReader_)
    {
        lastDocID = pSkipListReader_->skipTo(target);
        if(lastDocID > last_doc_id_)
        {
            seekTo(pSkipListReader_);
        }
    }

    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();

    if(! chunkDecoder_.decoded())
    {
	int doc_size = pDPostingInput->readVInt();
	pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size*sizeof(uint32_t));
	int tf_size = pDPostingInput->readVInt();
	pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size*sizeof(uint32_t));
	chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
	chunkDecoder_.set_doc_freq_buffer(NULL,NULL);
	chunkDecoder_.update_prev_decoded_doc_id(lastDocID);
	chunkDecoder_.decodeDocIds();
	chunkDecoder_.decodeFrequencies();

	if(!pDocFilter_) chunkDecoder_.post_process(pDocFilter_);
    }

    docid_t nDocID = chunkDecoder_.move_to(target);

    return ( nDocID >= target )? nDocID : -1;
}

void ChunkPostingReader::seekTo(SkipListReader* pSkipListReader)
{
    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();
    pDPostingInput->seek(postingOffset_ + pSkipListReader->getOffset());
    last_doc_id_ = pSkipListReader->getDoc();
    num_docs_decoded_ = pSkipListReader->getNumSkipped();
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
    if(pPPostingInput)
    {
        pPPostingInput->seek(poffset_ + pSkipListReader->getPOffset());
        last_pos_decoded_ = 0;///reset position
    }
}

int32_t ChunkPostingReader::decodeNext(uint32_t* pPosting,int32_t length)
{
    int32_t left = df_ - num_docs_decoded_;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length>>1);

    if (length > left*2)
        length = left*2;
    left = (length>>1);

    if(chunkDecoder_.decoded() && chunkDecoder_.use_internal_buffer())
    {
        ///process skip to within one chunk
        int start = chunkDecoder_.curr_document_offset();
        int end = chunkDecoder_.num_docs();
        for(int i = start; i < end; ++i)
        {
            *pDoc++ = chunkDecoder_.doc_id(i);
            *pFreq++ = chunkDecoder_.frequencies(i);
        }
        left -= (end - start);
    }


    int decodedDoc = 0;

    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();

    int doc_size = 0;
    int tf_size = 0;

    while(left > 0)
    {
        doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size*sizeof(uint32_t));
        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size*sizeof(uint32_t));
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.update_prev_decoded_doc_id(prev_block_last_doc_id_);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies();
        if(!pDocFilter_) chunkDecoder_.post_process(pDocFilter_);
        num_docs_left_ -= chunkDecoder_.num_docs();;
        left -= chunkDecoder_.num_docs();
        decodedDoc += chunkDecoder_.num_docs();
    }

    return decodedDoc;
}

int32_t ChunkPostingReader::decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& pLength)
{
    int32_t left = df_ - num_docs_decoded_;
    if (left <= 0)
        return -1;
    uint32_t* pDoc = pPosting;
    uint32_t* pFreq = pPosting + (length>>1);

    if (length > left*2)
        length = left*2;
    left = (length>>1);

    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();

    int decompressed_pos = 0;

    if(chunkDecoder_.decoded() && chunkDecoder_.use_internal_buffer())
    {
        ///process skip to within one chunk
        int start = chunkDecoder_.curr_document_offset();
        int end = chunkDecoder_.num_docs();
        for(int i = start; i < end; ++i)
        {
            *pDoc++ = chunkDecoder_.doc_id(i);
            *pFreq++ = chunkDecoder_.frequencies(i);
        }
        left -= (end - start);

        int size_of_positions = chunkDecoder_.size_of_positions();
        if(pLength < size_of_positions) growPosBuffer(pPPosting, pLength);
        chunkDecoder_.set_pos_buffer(pPPosting);
	
        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size*sizeof(uint32_t));
        chunkDecoder_.decodePositions(compressedPos_);
        chunkDecoder_.updatePositionOffset();
        decompressed_pos = size_of_positions - chunkDecoder_.curr_position_offset();
        memmove (pPPosting, chunkDecoder_.current_positions(), decompressed_pos);
    }

    int decodedDoc = 0;

    int doc_size = 0;
    int tf_size = 0;

    while(left > 0)
    {
        doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size*sizeof(uint32_t));
        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size*sizeof(uint32_t));
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.update_prev_decoded_doc_id(prev_block_last_doc_id_);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies();

        int size_of_positions = chunkDecoder_.size_of_positions();
        if((pLength - decompressed_pos) < size_of_positions) growPosBuffer(pPPosting, pLength);
        chunkDecoder_.set_pos_buffer(pPPosting + decompressed_pos);

        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size*sizeof(uint32_t));
        chunkDecoder_.decodePositions(compressedPos_);
        if(!pDocFilter_) chunkDecoder_.post_process(pDocFilter_);

        num_docs_left_ -= chunkDecoder_.num_docs();;
        left -= chunkDecoder_.num_docs();
        decodedDoc += chunkDecoder_.num_docs();
        if(chunkDecoder_.has_deleted_doc()) size_of_positions = chunkDecoder_.size_of_positions();
        decompressed_pos += size_of_positions;
    }

    return decodedDoc;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    return true;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* pPosting,uint32_t* pFreqs,int32_t nFreqs)
{
    return true;
}

void ChunkPostingReader::resetPosition()
{
    last_pos_decoded_ = 0;
}

void ChunkPostingReader::reset()
{
    postingOffset_ = 0;

    dlength_ = 0;
    df_ = 0;
    ctf_ = 0;
    poffset_ = 0;
    plength_ = 0;
    last_doc_id_ = 0;
}


}

NS_IZENELIB_IR_END


