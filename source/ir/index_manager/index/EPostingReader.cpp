#include <ir/index_manager/index/EPostingReader.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/FixedBlockSkipListReader.h>
#include <ir/index_manager/store/IndexOutput.h>
#include <ir/index_manager/store/IndexInput.h>

#include <math.h>


using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

//////////////////////////////////////////////////////////////////////////
///BlockPostingReader

BlockPostingReader::BlockPostingReader(InputDescriptor* pInputDescriptor, const TermInfo& termInfo)
        :pInputDescriptor_(pInputDescriptor)
        ,pSkipListReader_(0)
        ,pListingCache_(0)
        ,urgentBuffer_(0)
{
    reset(termInfo);
}

BlockPostingReader::~BlockPostingReader()
{
    if(pSkipListReader_) delete pSkipListReader_;
    if(urgentBuffer_) delete[] urgentBuffer_;
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
    num_pos_skipped_ = 0;

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
        blockDecoder_.init(curr_block_id_, 0, blockBuffer);
    }
    else
    {
        if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
        blockDecoder_.init(curr_block_id_, 0, urgentBuffer_);
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
        blockDecoder_.init(curr_block_id_, 0, blockBuffer);
    }
    else
    {
        if(!urgentBuffer_) urgentBuffer_ = (uint32_t*)new char[BLOCK_SIZE];
        blockDecoder_.init(curr_block_id_, 0, urgentBuffer_);
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
                //chunk->reset(blockDecoder_.curr_block_data(), std::min(ChunkDecoder::kChunkSize, num_docs_left_));
                //chunk->DecodeDocIds();

                chunk->update_prev_decoded_doc_id(prev_block_last_doc_id_);

                // We need offset from previous chunk if this is not the first chunk in the list.
                if (curr_chunk_num > blockDecoder_.starting_chunk()) 
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
                curr_doc_id += chunk->doc_id(k);  // Necessary for d-gap coding.

                // Found the docID we're looking for.
                if (curr_doc_id >= target) 
                {
                    chunk->set_curr_document_offset(k);  // Offset for the frequency.
                    chunk->set_prev_decoded_doc_id(curr_doc_id);  // Comment out if you want to decode all d-gaps in one swoop.
                    return curr_doc_id;
                }
            }
        }

        // Moving on to the next chunk.
        // We could not find the docID we were looking for, so we need to set the current chunk in the block
        // so the pointer to the next chunk is correctly offset in the block data.
        blockDecoder_.advance_curr_chunk();
        blockDecoder_.curr_chunk_decoder()->set_decoded(false);

        // Can update the number of documents left to process after processing the complete chunk.
        //list_data->update_num_docs_left();
        
    } 
    else 
    {
        prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
        advanceToNextBlock();
    }
    return 0;
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
        num_pos_skipped_ = 0;
    }
}

int32_t BlockPostingReader::decodeNext(uint32_t* pPosting,int32_t length)
{

    return 0;
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

}

NS_IZENELIB_IR_END


