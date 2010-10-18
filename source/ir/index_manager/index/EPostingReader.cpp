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
        ,uncompressed_pos_buffer_for_skipto_(0)
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
    if(uncompressed_pos_buffer_for_skipto_) free(uncompressed_pos_buffer_for_skipto_);
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

    prev_block_last_doc_id_ = 0;

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
    ++curr_block_id_;
}

void BlockPostingReader::skipToBlock(size_t targetBlock) 
{
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
    if(lastDocID == (docid_t)-1) return -1;

    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
    if(lastDocID > last_doc_id_)
    {
        size_t currBlock = pSkipListReader_->getBlockId();
        skipToBlock(currBlock - 1);
        last_doc_id_ = pSkipListReader_->getDoc();
        num_docs_decoded_ = pSkipListReader_->getNumSkipped();
        if(pPPostingInput)
        {
            pPPostingInput->seek(poffset_ + pSkipListReader_->getPOffset());
        }
    }

    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;
    bool computePos = (pPPostingInput == NULL)? false:true;
    while (blockDecoder_.curr_chunk() < blockDecoder_.num_chunks()) 
    {
        // Check the last doc id of the chunk against the current doc id
        // we're looking for and skip the chunk if possible.
        if (target <= blockDecoder_.chunk_last_doc_id(blockDecoder_.curr_chunk())) 
        {
            // Check if we previously decoded this chunk and decode if necessary.
            if (chunk.decoded() == false) 
            {
                // Create a new chunk and add it to the block.
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk.decodeDocIds();
                chunk.decodeFrequencies(computePos);
            }
            return chunk.move_to(target,nCurrentPosting,computePos);
        }

        blockDecoder_.advance_curr_chunk();
        blockDecoder_.chunk_decoder_.set_decoded(false);
        if(pPPostingInput)
       	{
       	    int size = pPPostingInput->readVInt();
       	    ensure_pos_buffer(size>>2);
       	    pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        }
        num_docs_left_ -= chunk.num_docs();
        
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
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk.set_doc_freq_buffer(pDoc,pFreq);
                chunk.decodeDocIds();
                chunk.decodeFrequencies(false);
                if(pDocFilter_) chunk.post_process(pDocFilter_);

                // Can update the number of documents left to process after processing the complete chunk.
                num_docs_left_ -= chunk.num_docs();
                left -= chunk.num_docs();
                decodedDoc += chunk.num_docs();
                pDoc += chunk.num_docs();
                pFreq += chunk.num_docs();
            }
            else
            {
                ///decodeTo has happened in this chunk, so this chunk has already been decoded
                for(int i = chunk.curr_document_offset(); i < chunk.num_docs(); ++i)
                {
                    *pDoc++ = chunk.doc_id(i);
                    *pFreq++ = chunk.frequencies(i);
                }

                // Can update the number of documents left to process after processing the complete chunk.
                int num_doc = chunk.num_docs() - chunk.curr_document_offset();
                num_docs_left_ -= num_doc;
                left -= num_doc;
                decodedDoc += num_doc;
            }
            blockDecoder_.advance_curr_chunk();
            chunk.set_decoded(false);
        } 
        else 
        {
            prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
            advanceToNextBlock();
        }        
    }

    num_docs_decoded_ += decodedDoc;

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
                chunk.reset(blockDecoder_.curr_block_data(), std::min(CHUNK_SIZE, (int)num_docs_left_));
                chunk.set_doc_freq_buffer(pDoc,pFreq);
                chunk.decodeDocIds();
                chunk.decodeFrequencies();

                size_of_positions = chunk.size_of_positions();
                if((posBufLength - decompressed_pos) < size_of_positions) growPosBuffer(pPPosting, posBufLength);

                chunk.set_pos_buffer(pPPosting + decompressed_pos);

                int size = pPPostingInput->readVInt();
                ensure_pos_buffer(size>>2);
                pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
                chunk.decodePositions(compressedPos_);
                if(pDocFilter_) chunk.post_process(pDocFilter_);

                num_docs_left_ -= chunk.num_docs();
                left -= chunk.num_docs();
                decodedDoc += chunk.num_docs();
                if(chunk.has_deleted_doc()) size_of_positions = chunk.size_of_positions();
                decompressed_pos += size_of_positions;

                pDoc += chunk.num_docs();
                pFreq += chunk.num_docs();
            }
            else
            {
                ///decodeTo has happened in this chunk, so this chunk has already been decoded
                for(int i = chunk.curr_document_offset(); i < chunk.num_docs(); ++i)
                {
                    *pDoc++ = chunk.doc_id(i);
                    *pFreq++ = chunk.frequencies(i);
                }

                size_of_positions = chunk.size_of_positions(true);
                if((posBufLength - decompressed_pos) < size_of_positions) growPosBuffer(pPPosting, posBufLength);

                chunk.set_pos_buffer(pPPosting + decompressed_pos);

                int size = pPPostingInput->readVInt();
                ensure_pos_buffer(size>>2);
                pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
                chunk.decodePositions(compressedPos_);
                memmove (pPPosting, pPPosting + chunk.curr_position_offset(), (size_of_positions - chunk.curr_position_offset())*sizeof(uint32_t));

                // Can update the number of documents left to process after processing the complete chunk.
                int num_doc = chunk.num_docs() - chunk.curr_document_offset();
                num_docs_left_ -= num_doc;
                left -= num_doc;
                decodedDoc += num_doc;
                decompressed_pos += (size_of_positions - chunk.curr_position_offset());
            }

            blockDecoder_.advance_curr_chunk();
            chunk.set_decoded(false);
        } 
        else 
        {
            prev_block_last_doc_id_ = blockDecoder_.chunk_last_doc_id(blockDecoder_.num_chunks() - 1);
            advanceToNextBlock();
        }        
    }

    num_docs_decoded_ += decodedDoc;
    posLength = decompressed_pos;
    return decodedDoc;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* pPosting,int32_t length)
{
    if(!pPosting) return true;

    IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();

    ChunkDecoder& chunk = blockDecoder_.chunk_decoder_;
    assert(chunk.decoded());
    assert(chunk.curr_document_offset() < chunk.num_docs());

    if(chunk.pos_decoded())
    {
        uint32_t* decoded_pos = uncompressed_pos_buffer_for_skipto_ + chunk.curr_position_offset();

        for(int i = 0; i < length; ++i)
            *pPosting++ = decoded_pos[i];
    }
    else
    {
        int size_of_positions = chunk.size_of_positions(true);
        if(!uncompressed_pos_buffer_for_skipto_)
        {
            uncompressed_pos_buffer_size_ = size_of_positions << 1;
            uncompressed_pos_buffer_for_skipto_ = (uint32_t*)malloc(uncompressed_pos_buffer_size_*sizeof(uint32_t));
        }
        else
        {
            if(uncompressed_pos_buffer_size_ < size_of_positions)
            {
                uncompressed_pos_buffer_size_ = size_of_positions << 1;
                uncompressed_pos_buffer_for_skipto_ = (uint32_t*)realloc(uncompressed_pos_buffer_for_skipto_, uncompressed_pos_buffer_size_*sizeof(uint32_t));
            }
        }
	
        chunk.set_pos_buffer(uncompressed_pos_buffer_for_skipto_);

        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunk.decodePositions(compressedPos_);

        uint32_t* decoded_pos = uncompressed_pos_buffer_for_skipto_ + chunk.curr_position_offset();

        for(int i = 0; i < length; ++i)
            *pPosting++ = decoded_pos[i];
    }

    chunk.set_curr_document_offset(chunk.curr_document_offset() + 1);
    chunk.updatePositionOffset();
    
    return true;
}

bool BlockPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting)
{
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
    num_docs_left_ = df_;
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
            num_docs_left_ -= (pSkipListReader_->getNumSkipped() - num_docs_decoded_);
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
	chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
	skipPosCount_ = 0;
	chunkDecoder_.set_doc_freq_buffer(pPosting,pPosting+(length>>1));
	chunkDecoder_.decodeDocIds();
	chunkDecoder_.decodeFrequencies(computePos);
	decodedCount = chunkDecoder_.num_docs();
	if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);
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

    if(chunkDecoder_.decoded() && chunkDecoder_.use_internal_buffer())
    {
        ///process skip to within one chunk
        int start = chunkDecoder_.curr_document_offset() + 1;
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
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size);

        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies(false);
        if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);
        num_docs_left_ -= chunkDecoder_.num_docs();;
        left -= chunkDecoder_.num_docs();
        decodedDoc += chunkDecoder_.num_docs();
        pDoc += chunkDecoder_.num_docs();
        pFreq += chunkDecoder_.num_docs();
    }

    num_docs_decoded_ += decodedDoc;

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

    if(chunkDecoder_.decoded())
    {
    assert(false);
        ///process skip to within one chunk
        chunkDecoder_.set_curr_document_offset(chunkDecoder_.curr_document_offset() + 1);
        int start = chunkDecoder_.curr_document_offset();
        int end = chunkDecoder_.num_docs();
        for(int i = start; i < end; ++i)
        {
            *pDoc++ = chunkDecoder_.doc_id(i);
            *pFreq++ = chunkDecoder_.frequencies(i);
        }

        left -= (end - start);
        if(chunkDecoder_.pos_decoded() == false)
        {
            int size_of_positions = chunkDecoder_.size_of_positions();
            if(posBufLength < size_of_positions) growPosBuffer(pPPosting, posBufLength);
            chunkDecoder_.set_pos_buffer(pPPosting);
	
            int size = pPPostingInput->readVInt();
            ensure_pos_buffer(size>>2);
            pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
            chunkDecoder_.decodePositions(compressedPos_);
            chunkDecoder_.updatePositionOffset();
            decompressed_pos = size_of_positions - chunkDecoder_.curr_position_offset();
            memmove (pPPosting, chunkDecoder_.current_positions(), decompressed_pos);
        }
    }

    int decodedDoc = 0;

    int doc_size = 0;
    int tf_size = 0;

    while(left > 0)
    {
        doc_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_,doc_size);
        tf_size = pDPostingInput->readVInt();
        pDPostingInput->readBytes((uint8_t*)compressedBuffer_ + doc_size, tf_size);
        chunkDecoder_.reset(compressedBuffer_, std::min(CHUNK_SIZE, (int)num_docs_left_));
        chunkDecoder_.set_doc_freq_buffer(pDoc,pFreq);
        chunkDecoder_.decodeDocIds();
        chunkDecoder_.decodeFrequencies();

        int size_of_positions = chunkDecoder_.size_of_positions();
        if((posBufLength - decompressed_pos) < size_of_positions) growPosBuffer(pPPosting, posBufLength);
        chunkDecoder_.set_pos_buffer(pPPosting + decompressed_pos);

        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunkDecoder_.decodePositions(compressedPos_);
        if(pDocFilter_) chunkDecoder_.post_process(pDocFilter_);

        num_docs_left_ -= chunkDecoder_.num_docs();;
        left -= chunkDecoder_.num_docs();
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
    skipPosCount_ += length;
    return true;
}

bool ChunkPostingReader::decodeNextPositions(uint32_t* &pPosting, int32_t& posBufLength, uint32_t* pFreqs,int32_t nFreqs, int32_t& nCurrentPPosting)
{
    assert(chunkDecoder_.decoded() == true);
    assert(chunkDecoder_.curr_document_offset() < chunkDecoder_.num_docs());
    if(! chunkDecoder_.pos_decoded())
    {
        int size_of_positions = chunkDecoder_.size_of_positions();
        if(posBufLength < size_of_positions) growPosBuffer(pPosting, posBufLength);
    
        chunkDecoder_.set_pos_buffer(pPosting);
        IndexInput* pPPostingInput = pInputDescriptor_->getPPostingInput();
        int size = pPPostingInput->readVInt();
        ensure_pos_buffer(size>>2);
        pPPostingInput->readBytes((uint8_t*)compressedPos_,size);
        chunkDecoder_.decodePositions(compressedPos_);
    }
    nCurrentPPosting = skipPosCount_ + chunkDecoder_.curr_position_offset();
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


