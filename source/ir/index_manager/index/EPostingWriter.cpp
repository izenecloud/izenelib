#include <ir/index_manager/index/EPostingWriter.h>
#include <ir/index_manager/index/SkipListWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

///////////////////////////////////////////////////////////////////////////////////
//BlockPostingWriter
///////////////////////////////////////////////////////////////////////////////////

BlockPostingWriter::BlockPostingWriter(MemCache* pCache)
        :pMemCache_(pCache)
        ,current_nocomp_block_pointer_(0)
        ,position_buffer_pointer_(0)
        ,nDF_(0)
        ,nCurTermFreq_(0)
        ,nCTF_(0)
	,nLastDocID_(BAD_DOCID)
{
    pBlockDataPool_ = new BlockDataPool(pCache);
    pPosDataPool_ = new ChunkDataPool(pCache) ;
    pSkipListWriter_ = new FixedBlockSkipListWriter(pCache);
    curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
    positions_  = new uint32_t[curr_position_buffer_size_];
}

BlockPostingWriter::~BlockPostingWriter()
{
    pMemCache_ = NULL;
    delete pBlockDataPool_;
    delete pPosDataPool_;
    delete pSkipListWriter_;
    delete [] positions_;
}

bool BlockPostingWriter::isEmpty()
{
    return nDF_ == 0;
}

void BlockPostingWriter::write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo)
{
    flush();

    termInfo.docFreq_ = nDF_;
    termInfo.ctf_ = nCTF_;
    termInfo.lastDocID_ = nLastDocID_;

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();

    termInfo.skipLevel_ = 1;
    termInfo.skipPointer_ = pDOutput->getFilePointer();
    pSkipListWriter_->write(pDOutput);	///write skip list data

    ///save the offset of posting descriptor
    termInfo.docPointer_ = pDOutput->getFilePointer();

    ///write doc posting data
    pBlockDataPool_->write(pDOutput);

    termInfo.docPostingLen_ = pDOutput->getFilePointer() - termInfo.docPointer_;

    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();

    termInfo.positionPointer_ = pPOutput->getFilePointer();

    ///write position posting data
    pPosDataPool_->write(pPOutput);

    termInfo.positionPostingLen_ = pPOutput->getFilePointer() - termInfo.positionPointer_;
}

void BlockPostingWriter::reset()
{
    pBlockDataPool_->reset();
    pPosDataPool_->reset();
    pSkipListWriter_->reset();
    current_nocomp_block_pointer_ = 0;
    position_buffer_pointer_ = 0;

    nCTF_ = 0;
    nCurTermFreq_ = 0;
    nDF_ = 0;
    nLastDocID_ = BAD_DOCID;
}

void BlockPostingWriter::add(docid_t docid, loc_t location)
{
    if(docid == nLastDocID_)
    {
	///see it before,only position is needed
	if(position_buffer_pointer_ == curr_position_buffer_size_ - 1)
	{
	    uint32_t new_position_buffer_size = curr_position_buffer_size_<<1;
	    uint32_t* new_positions = new uint32_t[new_position_buffer_size];
           memcpy(new_positions,positions_,curr_position_buffer_size_ * sizeof(uint32_t));
           delete[] positions_;
           positions_ = new_positions;
           curr_position_buffer_size_ = new_position_buffer_size;
	}
       positions_[position_buffer_pointer_++] = location;
       nCurTermFreq_++;
    }
    else
    {
        /// new doc id
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_] = nCurTermFreq_;        

        if(current_nocomp_block_pointer_ == (ChunkEncoder::kChunkSize - 1))
        {
            chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_);
            pPosDataPool_->addPOSChunk(chunk_);
            if(!pBlockDataPool_->addChunk(chunk_))
            {
                pBlockDataPool_->addBlock();
				
                pSkipListWriter_->addSkipPoint(nLastDocID_,pBlockDataPool_->num_doc_of_curr_block(),
                                                           pBlockDataPool_->getLength(),pPosDataPool_->getLength());
                pBlockDataPool_->addChunk(chunk_);
            }

            current_nocomp_block_pointer_ = 0;
            position_buffer_pointer_ = 0;
        }

        nLastDocID_ = docid;

        doc_ids_[current_nocomp_block_pointer_++] = docid;
        positions_[position_buffer_pointer_++] = location;

        nCTF_ += nCurTermFreq_;
        nCurTermFreq_ = 1;

        nLastDocID_ = docid;

        nDF_++;
    }
}

void BlockPostingWriter::flush()
{
    if(current_nocomp_block_pointer_ > 0)
    {
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_] = nCurTermFreq_;        
    
        chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_ + 1);
        pPosDataPool_->addPOSChunk(chunk_);

        if(!pBlockDataPool_->addChunk(chunk_))
        {
            pBlockDataPool_->addBlock();
            pSkipListWriter_->addSkipPoint(nLastDocID_,pBlockDataPool_->num_doc_of_curr_block(),
                                                        pBlockDataPool_->getLength(),pPosDataPool_->getLength());
            pBlockDataPool_->addChunk(chunk_);			
        }

        pSkipListWriter_->addSkipPoint(nLastDocID_,current_nocomp_block_pointer_+1, 
                                                pBlockDataPool_->getLength(),pPosDataPool_->getLength());

    }
}


///////////////////////////////////////////////////////////////////////////////////
//ChunkPostingWriter
///////////////////////////////////////////////////////////////////////////////////

ChunkPostingWriter::ChunkPostingWriter(MemCache* pCache,int skipInterval, int maxSkipLevel)
        :pMemCache_(pCache)
        ,pSkipListWriter_(0)
        ,skipInterval_(skipInterval)
        ,maxSkipLevel_(maxSkipLevel)
        ,current_nocomp_block_pointer_(0)
        ,position_buffer_pointer_(0)
        ,nDF_(0)
        ,nCurTermFreq_(0)
        ,nCTF_(0)
	,nLastDocID_(BAD_DOCID)
{
    assert(ChunkEncoder::kChunkSize == skipInterval);
    pDocFreqDataPool_ = new ChunkDataPool(pCache);
    pPosDataPool_ = new ChunkDataPool(pCache) ;
    if(skipInterval_)  pSkipListWriter_ = new SkipListWriter(skipInterval,maxSkipLevel,pCache);
    curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
    positions_  = new uint32_t[curr_position_buffer_size_];
}

ChunkPostingWriter::~ChunkPostingWriter()
{
    pMemCache_ = NULL;
    delete pDocFreqDataPool_;
    delete pPosDataPool_;
    if(pSkipListWriter_) delete pSkipListWriter_;
    delete [] positions_;
}

bool ChunkPostingWriter::isEmpty()
{
    return nDF_ == 0;
}

void ChunkPostingWriter::write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo)
{
    flush();
    termInfo.docFreq_ = nDF_;
    termInfo.ctf_ = nCTF_;
    termInfo.lastDocID_ = nLastDocID_;
    if(skipInterval_ && nDF_ > 0 && nDF_ % skipInterval_ == 0)
    {
        if(pSkipListWriter_)
        {
            pSkipListWriter_->addSkipPoint(nLastDocID_,pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
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
    pDocFreqDataPool_->write(pDOutput);

    termInfo.docPostingLen_ = pDOutput->getFilePointer() - termInfo.docPointer_;

    IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();

    termInfo.positionPointer_ = pPOutput->getFilePointer();

    ///write position posting data
    pPosDataPool_->write(pPOutput);

    termInfo.positionPostingLen_ = pPOutput->getFilePointer() - termInfo.positionPointer_;
}

void ChunkPostingWriter::reset()
{
    pDocFreqDataPool_->reset();
    pPosDataPool_->reset();
    pSkipListWriter_->reset();
    current_nocomp_block_pointer_ = 0;
    position_buffer_pointer_ = 0;

    nCTF_ = 0;
    nCurTermFreq_ = 0;
    nDF_ = 0;
    nLastDocID_ = BAD_DOCID;
}

void ChunkPostingWriter::add(docid_t docid, loc_t location)
{
    if(docid == nLastDocID_)
    {
	///see it before,only position is needed
	if(position_buffer_pointer_ == curr_position_buffer_size_ - 1)
	{
	    uint32_t new_position_buffer_size = curr_position_buffer_size_<<1;
	    uint32_t* new_positions = new uint32_t[new_position_buffer_size];
           memcpy(new_positions,positions_,curr_position_buffer_size_ * sizeof(uint32_t));
           delete[] positions_;
           positions_ = new_positions;
           curr_position_buffer_size_ = new_position_buffer_size;
	}
       positions_[position_buffer_pointer_++] = location;
       nCurTermFreq_++;
    }
    else
    {
        /// new doc id
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_] = nCurTermFreq_;        

        if(skipInterval_ && nDF_ > 0 && nDF_ % skipInterval_ == 0)
        {
            chunk_.encode(doc_ids_, frequencies_, positions_, ChunkEncoder::kChunkSize);
            pDocFreqDataPool_->addDFChunk(chunk_);
            pPosDataPool_->addPOSChunk(chunk_);
            pSkipListWriter_->addSkipPoint(nLastDocID_,pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
            current_nocomp_block_pointer_ = 0;
            position_buffer_pointer_ = 0;
        }

        doc_ids_[current_nocomp_block_pointer_++] = docid;
        positions_[position_buffer_pointer_++] = location;

        nCTF_ += nCurTermFreq_;
        nCurTermFreq_ = 1;

        nLastDocID_ = docid;

        nDF_++;
    }
}

void ChunkPostingWriter::flush()
{
    if(current_nocomp_block_pointer_ > 0)
    {
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_] = nCurTermFreq_;        
    
        chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_+1);
        pDocFreqDataPool_->addDFChunk(chunk_);
        pPosDataPool_->addPOSChunk(chunk_);
    }
}

}

NS_IZENELIB_IR_END

