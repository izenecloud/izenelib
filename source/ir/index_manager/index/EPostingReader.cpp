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
    curr_block_id_ = -1;
    start_block_id_ = termInfo.skipLevel_; ///we reuse "skiplevel" to store start block id
    total_block_num_ = termInfo.docPostingLen_/BLOCK_SIZE;
    last_block_id_ = start_block_id_ + total_block_num_ - 1;
    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    dlength_ = termInfo.docPostingLen_;
    df_ = termInfo.docFreq_;
    ctf_ = termInfo.ctf_;
    poffset_ = termInfo.positionPointer_;
    plength_ = termInfo.positionPostingLen_;
    last_doc_id_ = 0;
    num_docs_decoded_ = 0;

    prev_block_last_doc_id_ = 0;

    prev_block_id_ = -1;
    prev_chunk_ = 0;

    skipPosCount_ = 0;

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    pDPInput->seek(termInfo.skipPointer_);
    pSkipListReader_ = new FixedBlockSkipListReader(pDPInput, start_block_id_);

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
    if(-1 == curr_block_id_) 
        curr_block_id_ = start_block_id_;
    else
    {
        ++curr_block_id_;
        if(curr_block_id_ > last_block_id_) return;
        prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
    }

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    if(pListingCache_)
    {
        pListingCache_->freeBlock(curr_block_id_);

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
        pDPInput->seek(postingOffset_ + (curr_block_id_ - start_block_id_)* BLOCK_SIZE);
        pDPInput->read((char *)urgentBuffer_, BLOCK_SIZE);
        blockDecoder_.init(curr_block_id_, urgentBuffer_);
    }
    blockDecoder_.chunk_decoder_.set_prev_decoded_doc_id(prev_block_last_doc_id_);

}

void BlockPostingReader::skipToBlock(int targetBlock) 
{
    if(targetBlock <= curr_block_id_) return;
    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();

    if(pListingCache_)
    {
        for(; curr_block_id_ <= targetBlock; ++curr_block_id_)
        {
            pListingCache_->freeBlock(curr_block_id_); 
        }

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
        curr_block_id_ = targetBlock;
        pDPInput->seek(postingOffset_ + (curr_block_id_ - start_block_id_) * BLOCK_SIZE);
        pDPInput->read((char *)urgentBuffer_, BLOCK_SIZE);
        blockDecoder_.init(curr_block_id_, urgentBuffer_);
    }
}

docid_t BlockPostingReader::decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting)
{
    docid_t lastDocID = pSkipListReader_->skipTo(target);
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;
    if(lastDocID > last_doc_id_)
    {
        int currBlock = pSkipListReader_->getBlockId();
        if(currBlock > last_block_id_) return BAD_DOCID;
        skipToBlock(currBlock);
        last_doc_id_ = pSkipListReader_->getDoc();
        num_docs_decoded_ = pSkipListReader_->getNumSkipped();
        chunk.set_prev_decoded_doc_id(last_doc_id_);
        if(pPPostingInput)
        {
            pPPostingInput->seek(poffset_ + pSkipListReader_->getPOffset());
        }
    }
    else if(last_doc_id_ == 0)
    {
        skipToBlock(start_block_id_);
    }

    bool computePos = (pPPostingInput == NULL)? false:true;
    while (blockDecoder_.curr_chunk() < blockDecoder_.num_chunks()) 
    {
        if (target <= blockDecoder_.chunk_last_doc_id(blockDecoder_.curr_chunk()))
        {
            if (chunk.decoded() == false) 
            {
                int start_chunk = 0;
                if(prev_block_id_ == curr_block_id_)
                {
                    start_chunk = prev_chunk_ + 1;
                }
                int decoded_num_docs_in_this_block = 0;
                for(int i = start_chunk; i < blockDecoder_.curr_chunk(); ++i)
                    decoded_num_docs_in_this_block += CHUNK_SIZE;
                num_docs_decoded_ += decoded_num_docs_in_this_block;
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
                chunk.set_doc_freq_buffer(pPosting,pPosting+(length>>1));
                chunk.decodeDocIds();
                chunk.decodeFrequencies(computePos);

                num_docs_decoded_ += chunk.num_docs();

                ///num_docs_decoded_ should record the practical overall doc ids
                ///after post_process, the deleted doc ids will be removed from the decoding buffer
                if(pDocFilter_) chunk.post_process(pDocFilter_);
                decodedCount = chunk.num_docs();
            }
            skipPosCount_ = 0;
            prev_block_id_ = curr_block_id_;
            prev_chunk_ = blockDecoder_.curr_chunk();
            return chunk.move_to(target,nCurrentPosting,computePos);
        }
        if(pPPostingInput)
       	{
       	    ///skip positions
       	    if(!blockDecoder_.curr_chunk_pos_loaded(blockDecoder_.curr_chunk()))
       	    {
       	        int size = pPPostingInput->readVInt();
       	        ensure_pos_buffer(size>>2);
       	        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
       	        blockDecoder_.set_curr_chunk_pos_loaded(blockDecoder_.curr_chunk());
       	    }
        }
        blockDecoder_.advance_curr_chunk();
        blockDecoder_.chunk_decoder_.set_decoded(false);
    } 

    return BAD_DOCID;
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

    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;

    while(left > 0)
    {
        int curr_chunk_num = blockDecoder_.curr_chunk();
        if (curr_chunk_num < blockDecoder_.num_chunks()) 
        {
            // Check if we previously decoded this chunk and decode if necessary.
            if (chunk.decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
                chunk.set_doc_freq_buffer(pDoc,pFreq);
                chunk.decodeDocIds();
                chunk.decodeFrequencies(false);

                left -= chunk.num_docs();
                num_docs_decoded_ += chunk.num_docs();
                ///num_docs_decoded_ should record the practical overall doc ids
                ///after post_process, the deleted doc ids will be removed from the decoding buffer
                if(pDocFilter_) chunk.post_process(pDocFilter_);

                decodedDoc += chunk.num_docs();
                pDoc += chunk.num_docs();
                pFreq += chunk.num_docs();
            }
            blockDecoder_.advance_curr_chunk();
            chunk.set_decoded(false);
        } 
        else 
        {
            advanceToNextBlock();
        }        
    }

    prev_block_id_ = blockDecoder_.curr_block_num_;
    prev_chunk_ = blockDecoder_.curr_chunk();
    return decodedDoc;
}

int32_t BlockPostingReader::decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength)
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
    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;

    while(left > 0)
    {
        int curr_chunk_num = blockDecoder_.curr_chunk();
        if (curr_chunk_num < blockDecoder_.num_chunks())
        {
            // Check if we previously decoded this chunk and decode if necessary.
            if (chunk.decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
                chunk.set_doc_freq_buffer(pDoc,pFreq);
                chunk.decodeDocIds();
                chunk.decodeFrequencies();

                size_of_positions = chunk.size_of_positions();
                if(posBufLength < decompressed_pos + size_of_positions)
                    growPosBuffer(pPPosting, posBufLength, decompressed_pos + size_of_positions);

                chunk.set_pos_buffer(pPPosting + decompressed_pos);

                blockDecoder_.set_curr_chunk_pos_loaded(blockDecoder_.curr_chunk());

                int size = pPPostingInput->readVInt();
                ensure_pos_buffer(size>>2);
                pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
                chunk.decodePositions(compressedPos_);

                left -= chunk.num_docs();
                num_docs_decoded_ += chunk.num_docs();
                ///num_docs_decoded_ should record the practical overall doc ids
                ///after post_process, the deleted doc ids will be removed from the decoding buffer
                if(pDocFilter_) chunk.post_process(pDocFilter_);

                decodedDoc += chunk.num_docs();
                if(chunk.has_deleted_doc()) size_of_positions = chunk.size_of_positions();
                decompressed_pos += size_of_positions;

                pDoc += chunk.num_docs();
                pFreq += chunk.num_docs();
            }

            blockDecoder_.advance_curr_chunk();
            chunk.set_decoded(false);
        } 
        else 
        {
            advanceToNextBlock();
        }        
    }

    posLength = decompressed_pos;

    prev_block_id_ = blockDecoder_.curr_block_num_;
    prev_chunk_ = blockDecoder_.curr_chunk();

    return decodedDoc;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    skipPosCount_ += length;
    return true;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, int32_t decodeLength, int32_t& nCurrentPPosting)
{
    if(!pPosting)
    {
        skipPosCount_ += decodeLength;
        return true;
    }
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();

    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;
    assert(chunk.decoded());
    assert(chunk.curr_document_offset() < chunk.num_docs());

    if(! chunk.pos_decoded())
    {
        int size_of_positions = chunk.size_of_positions();
        if(posBufLength < size_of_positions)
            growPosBuffer(pPosting, posBufLength, size_of_positions);
        chunk.set_pos_buffer(pPosting);

        if(!blockDecoder_.curr_chunk_pos_loaded(blockDecoder_.curr_chunk()))
        {
            int size = pPPostingInput->readVInt();
            ensure_pos_buffer(size>>2);
            pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
            blockDecoder_.set_curr_chunk_pos_loaded(blockDecoder_.curr_chunk());
        }
        chunk.decodePositions(compressedPos_);
    }

    nCurrentPPosting = skipPosCount_ + chunk.curr_position_offset();
    skipPosCount_ = 0;

    return true;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting)
{
    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();

    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;
    assert(chunk.decoded());
    assert(chunk.curr_document_offset() < chunk.num_docs());

    if(! chunk.pos_decoded())
    {
        int size_of_positions = chunk.size_of_positions();
        if(posBufLength < size_of_positions)
            growPosBuffer(pPosting, posBufLength, size_of_positions);
        chunk.set_pos_buffer(pPosting);

        if(!blockDecoder_.curr_chunk_pos_loaded(blockDecoder_.curr_chunk()))
        {
            int size = pPPostingInput->readVInt();
            ensure_pos_buffer(size>>2);
            pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
            blockDecoder_.set_curr_chunk_pos_loaded(blockDecoder_.curr_chunk());
        }
        chunk.decodePositions(compressedPos_);
    }

    nCurrentPPosting = skipPosCount_ + chunk.curr_position_offset();
    skipPosCount_ = 0;

    return true;
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

    prev_block_id_ = 0;
    prev_chunk_ = 0;

    skipPosCount_ = 0;
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
    postingOffset_ = termInfo.docPointer_;

    IndexInput* pDPInput = pInputDescriptor_->getDPostingInput();
    //IndexInput should be reset because the internal buffer should be clear when a new posting is needed to be read
    pDPInput->reset();

    dlength_ = termInfo.docPostingLen_;
    df_ = termInfo.docFreq_;
    ctf_ = termInfo.ctf_;
    poffset_ = termInfo.positionPointer_;
    plength_ = termInfo.positionPostingLen_;
    last_doc_id_ = 0;
    num_docs_decoded_ = 0;

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    if(termInfo.skipPointer_ != -1)
    {
        pDPInput->seek(termInfo.skipPointer_);
        pSkipListReader_ = new SkipListReader(pDPInput, skipInterval_, termInfo.skipLevel_);
    }

    pDPInput->seek(postingOffset_);

    IndexInput* pPPInput = pInputDescriptor_->getPPostingInput();
    if (pPPInput)
    {
        pPPInput->reset();
        pPPInput->seek(termInfo.positionPointer_);
    }
    chunkDecoder_.set_prev_decoded_doc_id(0);
    chunkDecoder_.reset(NULL, 0);
    skipPosCount_ = 0;
}

docid_t ChunkPostingReader::decodeTo(docid_t target, uint32_t* pPosting, int32_t length, int32_t& decodedCount, int32_t& nCurrentPosting)
{
    docid_t lastDocID = 0;

    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
    bool computePos = (pPPostingInput == NULL)? false:true;

    if(pSkipListReader_)
    {
        lastDocID = pSkipListReader_->skipTo(target);
        if(lastDocID > last_doc_id_)
        {
            IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();
			
            pDPostingInput->seek(postingOffset_ + pSkipListReader_->getOffset());
            last_doc_id_ = pSkipListReader_->getDoc();
            num_docs_decoded_ = pSkipListReader_->getNumSkipped();
            if(pPPostingInput)
            {
                pPPostingInput->seek(poffset_ + pSkipListReader_->getPOffset());
            }
            chunkDecoder_.set_prev_decoded_doc_id(last_doc_id_);
            chunkDecoder_.reset(NULL, 0);
        }
    }
    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();

    if(! chunkDecoder_.decoded())
    {
        int doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size);
        int tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
        skipPosCount_ = 0;
        chunkDecoder_.set_doc_freq_buffer(pPosting,pPosting+(length>>1));
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies(computePos);

        num_docs_decoded_ += chunkDecoder_.num_docs();
        ///num_docs_decoded_ should record the practical overall doc ids
        ///after post_process, the deleted doc ids will be removed from the decoding buffer
        if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);
        decodedCount = chunkDecoder_.num_docs();
    }

    return chunkDecoder_.move_to(target,nCurrentPosting,computePos);
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

    int decodedDoc = 0;

    IndexInput* pDPostingInput = pInputDescriptor_->getDPostingInput();

    int doc_size = 0;
    int tf_size = 0;

    while(left > 0)
    {
        doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size);

        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies(false);

        num_docs_decoded_ += chunkDecoder_.num_docs();
        left -= chunkDecoder_.num_docs();
        ///num_docs_decoded_ should record the practical overall doc ids
        ///after post_process, the deleted doc ids will be removed from the decoding buffer
        if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);

        decodedDoc += chunkDecoder_.num_docs();
        pDoc += chunkDecoder_.num_docs();
        pFreq += chunkDecoder_.num_docs();
    }


    return decodedDoc;
}

int32_t ChunkPostingReader::decodeNext(uint32_t* pPosting,int32_t length, uint32_t* &pPPosting, int32_t& posBufLength, int32_t& posLength)
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

    int decodedDoc = 0;

    int doc_size = 0;
    int tf_size = 0;

    while(left > 0)
    {
        doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size);
        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, static_cast<int>(leftDocsNum())));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies();

        int size_of_positions = chunkDecoder_.size_of_positions();
        if(posBufLength < decompressed_pos + size_of_positions)
            growPosBuffer(pPPosting, posBufLength, decompressed_pos + size_of_positions);
        chunkDecoder_.set_pos_buffer(pPPosting + decompressed_pos);

        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunkDecoder_.decodePositions(compressedPos_);

        num_docs_decoded_ += chunkDecoder_.num_docs();
        left -= chunkDecoder_.num_docs();
        ///num_docs_decoded_ should record the practical overall doc ids
        ///after post_process, the deleted doc ids will be removed from the decoding buffer
        if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);

        decodedDoc += chunkDecoder_.num_docs();
        if(chunkDecoder_.has_deleted_doc()) size_of_positions = chunkDecoder_.size_of_positions();
        decompressed_pos += size_of_positions;

	pDoc += chunkDecoder_.num_docs();
	pFreq += chunkDecoder_.num_docs();

    }


    num_docs_decoded_ += decodedDoc;
    posLength = decompressed_pos;
    chunkDecoder_.reset(NULL, 0);
    skipPosCount_ = 0;

    return decodedDoc;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    if(!pPosting)
    {
        skipPosCount_ += length;
        return true;
    }
    

    return true;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, int32_t decodeLength, int32_t& nCurrentPPosting)
{
    if(!pPosting)
    {
        skipPosCount_ += decodeLength;
        return true;
    }
    assert(chunkDecoder_.decoded() == true);
    assert(chunkDecoder_.curr_document_offset() < chunkDecoder_.num_docs());
    if(! chunkDecoder_.pos_decoded())
    {
        int size_of_positions = chunkDecoder_.size_of_positions();
        if(posBufLength < size_of_positions)
            growPosBuffer(pPosting, posBufLength, size_of_positions);
    
        chunkDecoder_.set_pos_buffer(pPosting);
        IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunkDecoder_.decodePositions(compressedPos_);
    }
    nCurrentPPosting = skipPosCount_ + chunkDecoder_.curr_position_offset();
    skipPosCount_ = 0;
    return true;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting)
{
    assert(chunkDecoder_.decoded() == true);
    assert(chunkDecoder_.curr_document_offset() < chunkDecoder_.num_docs());
    if(! chunkDecoder_.pos_decoded())
    {
        int size_of_positions = chunkDecoder_.size_of_positions();
        if(posBufLength < size_of_positions)
            growPosBuffer(pPosting, posBufLength, size_of_positions);
    
        chunkDecoder_.set_pos_buffer(pPosting);
        IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunkDecoder_.decodePositions(compressedPos_);
    }
    nCurrentPPosting = skipPosCount_ + chunkDecoder_.curr_position_offset();
    skipPosCount_ = 0;
    return true;
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
    skipPosCount_ = 0;
}


}

NS_IZENELIB_IR_END


