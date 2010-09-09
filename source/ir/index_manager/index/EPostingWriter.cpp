#include <ir/index_manager/index/EPostingWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

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
    pPosDataPool_ = new PosDataPool(pCache) ;
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
            pBlockDataPool_->addChunk(chunk_);
            pPosDataPool_->addChunk(chunk_);
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
    
        chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_);
        pBlockDataPool_->addChunk(chunk_);
        pPosDataPool_->addChunk(chunk_);
    }
}

}

NS_IZENELIB_IR_END

