#include <ir/index_manager/index/EPostingWriter.h>
#include <ir/index_manager/index/SkipListWriter.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

///////////////////////////////////////////////////////////////////////////////////
//BlockPostingWriter
///////////////////////////////////////////////////////////////////////////////////

BlockPostingWriter::BlockPostingWriter(
    boost::shared_ptr<MemCache> pCache,
    IndexLevel indexLevel
)
    :pMemCache_(pCache)
    ,current_nocomp_block_pointer_(0)
    ,position_buffer_pointer_(0)
    ,nDF_(0)
    ,nCurTermFreq_(0)
    ,nmaxTF_(0)
    ,nCTF_(0)
    ,nLastDocID_(BAD_DOCID)
    ,current_block_id_(0)
    ,indexLevel_(indexLevel)
{
    pBlockDataPool_ = new BlockDataPool(pCache);
    pPosDataPool_ = NULL;
    if(indexLevel == WORDLEVEL)
        pPosDataPool_ = new ChunkDataPool(pCache);
    pSkipListWriter_ = new FixedBlockSkipListWriter(pCache);
    curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
    positions_ = (uint32_t*)malloc(curr_position_buffer_size_*sizeof(uint32_t));
}

BlockPostingWriter::~BlockPostingWriter()
{
    delete pBlockDataPool_;
    if(pPosDataPool_)
        delete pPosDataPool_;
    delete pSkipListWriter_;
    free(positions_);
}

bool BlockPostingWriter::isEmpty()
{
    return nDF_ == 0;
}

void BlockPostingWriter::write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo)
{
    flush();
    if (pPosDataPool_)
        pPosDataPool_->truncTailChunk();

    termInfo.docFreq_ = nDF_;
    termInfo.ctf_ = nCTF_;
    termInfo.maxTF_ = nmaxTF_;
    termInfo.lastDocID_ = nLastDocID_;

    IndexOutput* pDOutput = pOutputDescriptor->getDPostingOutput();

    termInfo.skipPointer_ = pDOutput->getFilePointer();
    pSkipListWriter_->write(pDOutput);	///write skip list data

    ///save the offset of posting descriptor
    termInfo.docPointer_ = pDOutput->getFilePointer();

    ///write doc posting data
    pBlockDataPool_->write(pDOutput);

    termInfo.docPostingLen_ = pDOutput->getFilePointer() - termInfo.docPointer_;

    if (indexLevel_ == WORDLEVEL)
    {
        uint32_t num_blocks = termInfo.docPostingLen_ / BLOCK_SIZE;
        ///we reuse "skiplevel " to store the start block id for this posting.
        termInfo.skipLevel_ = current_block_id_ - num_blocks;

        IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();

        termInfo.positionPointer_ = pPOutput->getFilePointer();

        ///write position posting data
        if(pPosDataPool_)
            pPosDataPool_->write(pPOutput);

        termInfo.positionPostingLen_ = pPOutput->getFilePointer() - termInfo.positionPointer_;
    }
}

void BlockPostingWriter::reset()
{
    pBlockDataPool_->reset();
    if (pPosDataPool_)
        pPosDataPool_->reset();
    pSkipListWriter_->reset();
    current_nocomp_block_pointer_ = 0;
    position_buffer_pointer_ = 0;
    chunk_.reset();

    nDF_ = 0;
    nCurTermFreq_ = 0;
    nmaxTF_ = 0;
    nCTF_ = 0;
    nLastDocID_ = BAD_DOCID;
}

void BlockPostingWriter::add(docid_t docid, loc_t location, bool realTimeFlag)
{
    if(docid == nLastDocID_)
    {
    ///see it before,only position is needed
    if(position_buffer_pointer_ == curr_position_buffer_size_ - 1)
    {
           curr_position_buffer_size_ = curr_position_buffer_size_<<1;
           positions_ = (uint32_t*)realloc(positions_, curr_position_buffer_size_ * sizeof(uint32_t));
    }
       positions_[position_buffer_pointer_++] = location;
       nCurTermFreq_++;
    }
    else
    {
        /// new doc id
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_++] = nCurTermFreq_;

        if(current_nocomp_block_pointer_ == ChunkEncoder::kChunkSize)
        {
            chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_);
            if(!pBlockDataPool_->addChunk(chunk_))
            {
                current_block_id_++;
                pBlockDataPool_->copyBlockData();
                if(pPosDataPool_)
                    pSkipListWriter_->addSkipPoint(pBlockDataPool_->blockEncoder_.last_doc_id_,
                               pBlockDataPool_->num_doc_of_curr_block(),pPosDataPool_->getLength());
                else
                    pSkipListWriter_->addSkipPoint(pBlockDataPool_->blockEncoder_.last_doc_id_,
                               pBlockDataPool_->num_doc_of_curr_block(),0);
                pBlockDataPool_->addBlock();
                pBlockDataPool_->addChunk(chunk_);
            }
            if (pPosDataPool_)
                pPosDataPool_->addPOSChunk(chunk_);
            current_nocomp_block_pointer_ = 0;
            position_buffer_pointer_ = 0;
        }

        doc_ids_[current_nocomp_block_pointer_] = docid;
        positions_[position_buffer_pointer_++] = location;

        nCTF_ += nCurTermFreq_;
        nmaxTF_ = (nCurTermFreq_ > nmaxTF_) ? nCurTermFreq_ : nmaxTF_;
        nCurTermFreq_ = 1;

        nLastDocID_ = docid;

        nDF_++;
    }
}

void BlockPostingWriter::flush()
{
    if(nCurTermFreq_ > 0)
    {
        frequencies_[current_nocomp_block_pointer_++] = nCurTermFreq_;
        chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_);

        if(!pBlockDataPool_->addChunk(chunk_))
        {
            current_block_id_++;
            pBlockDataPool_->copyBlockData();

            if (pPosDataPool_)
                pSkipListWriter_->addSkipPoint(pBlockDataPool_->blockEncoder_.last_doc_id_,
                                   pBlockDataPool_->num_doc_of_curr_block(), pPosDataPool_->getLength());
            else
                pSkipListWriter_->addSkipPoint(pBlockDataPool_->blockEncoder_.last_doc_id_,
                                   pBlockDataPool_->num_doc_of_curr_block(), 0);
            pBlockDataPool_->addBlock();
            pBlockDataPool_->addChunk(chunk_);
        }

        current_block_id_++;
        pBlockDataPool_->copyBlockData();
        if (pPosDataPool_)
        {
            pPosDataPool_->addPOSChunk(chunk_);
        }
        if (pPosDataPool_)
            pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(), pBlockDataPool_->num_doc_of_curr_block(), pPosDataPool_->getLength());
        else
            pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(), pBlockDataPool_->num_doc_of_curr_block(), 0);

        nCTF_ += nCurTermFreq_;
        nmaxTF_ = (nCurTermFreq_ > nmaxTF_) ? nCurTermFreq_ : nmaxTF_;
        nCurTermFreq_ = 0;
    }
}


///////////////////////////////////////////////////////////////////////////////////
//ChunkPostingWriter
///////////////////////////////////////////////////////////////////////////////////

ChunkPostingWriter::ChunkPostingWriter(
    boost::shared_ptr<MemCache> pCache,
    int skipInterval,
    int maxSkipLevel,
    IndexLevel indexLevel)
    :pMemCache_(pCache)
    ,pSkipListWriter_(0)
    ,skipInterval_(skipInterval)
    ,maxSkipLevel_(maxSkipLevel)
    ,current_nocomp_block_pointer_(0)
    ,position_buffer_pointer_(0)
    ,nDF_(0)
    ,nCurTermFreq_(0)
    ,nmaxTF_(0)
    ,nCTF_(0)
    ,nLastDocID_(BAD_DOCID)
    ,indexLevel_(indexLevel)
{
    assert(ChunkEncoder::kChunkSize == skipInterval);
    pDocFreqDataPool_ = new ChunkDataPool(pCache);
    pPosDataPool_ = NULL;
    if(indexLevel == WORDLEVEL)
        pPosDataPool_ = new ChunkDataPool(pCache) ;
    if(skipInterval_ > 0 && maxSkipLevel_ > 0)
        pSkipListWriter_ = new SkipListWriter(skipInterval_,maxSkipLevel_,pMemCache_);
    curr_position_buffer_size_ = INIT_POS_CHUNK_SIZE;
    positions_ = (uint32_t*)malloc(curr_position_buffer_size_*sizeof(uint32_t));
}

ChunkPostingWriter::~ChunkPostingWriter()
{
    delete pDocFreqDataPool_;
    if (pPosDataPool_)
        delete pPosDataPool_;
    if(pSkipListWriter_) delete pSkipListWriter_;
    free(positions_);
}

bool ChunkPostingWriter::isEmpty()
{
    return nDF_ == 0;
}

void ChunkPostingWriter::write(OutputDescriptor* pOutputDescriptor, TermInfo& termInfo)
{
    flush();
    pDocFreqDataPool_->truncTailChunk();
    if (pPosDataPool_)
        pPosDataPool_->truncTailChunk();

    termInfo.docFreq_ = nDF_;
    termInfo.ctf_ = nCTF_;
    termInfo.maxTF_ = nmaxTF_;
    termInfo.lastDocID_ = nLastDocID_;

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

    if(indexLevel_ == WORDLEVEL)
    {
        IndexOutput* pPOutput = pOutputDescriptor->getPPostingOutput();

        termInfo.positionPointer_ = pPOutput->getFilePointer();

        ///write position posting data
        pPosDataPool_->write(pPOutput);

        termInfo.positionPostingLen_ = pPOutput->getFilePointer() - termInfo.positionPointer_;
    }
}

void ChunkPostingWriter::reset()
{
    pDocFreqDataPool_->reset();
    if (pPosDataPool_)
        pPosDataPool_->reset();
    if(pSkipListWriter_)
        pSkipListWriter_->reset();
    current_nocomp_block_pointer_ = 0;
    position_buffer_pointer_ = 0;
    chunk_.reset();

    nCTF_ = 0;
    nCurTermFreq_ = 0;
    nmaxTF_ = 0;
    nDF_ = 0;
    nLastDocID_ = BAD_DOCID;
}

void ChunkPostingWriter::add(docid_t docid, loc_t location, bool realTimeFlag)
{
    if(docid == nLastDocID_)
    {
    ///see it before,only position is needed
    if(position_buffer_pointer_ == curr_position_buffer_size_ - 1)
    {
           curr_position_buffer_size_ = curr_position_buffer_size_<<1;
           positions_ = (uint32_t*)realloc(positions_, curr_position_buffer_size_ * sizeof(uint32_t));
    }
       positions_[position_buffer_pointer_++] = location;
       nCurTermFreq_++;
    }
    else
    {
        /// new doc id
        if (nCurTermFreq_ > 0)
            frequencies_[current_nocomp_block_pointer_++] = nCurTermFreq_;

        if(current_nocomp_block_pointer_ == ChunkEncoder::kChunkSize)
        {
            chunk_.encode(doc_ids_, frequencies_, positions_, ChunkEncoder::kChunkSize);
            pDocFreqDataPool_->addDFChunk(chunk_);
            if (pPosDataPool_)
                pPosDataPool_->addPOSChunk(chunk_);
            if(pSkipListWriter_)
            {
                if (pPosDataPool_)
                    pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
                else
                    pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),0);
            }
            current_nocomp_block_pointer_ = 0;
            position_buffer_pointer_ = 0;
        }

        doc_ids_[current_nocomp_block_pointer_] = docid;
        positions_[position_buffer_pointer_++] = location;

        nCTF_ += nCurTermFreq_;
        nmaxTF_ = (nCurTermFreq_ > nmaxTF_) ? nCurTermFreq_ : nmaxTF_;
        nCurTermFreq_ = 1;

        nLastDocID_ = docid;

        nDF_++;
    }
}

void ChunkPostingWriter::flush()
{
    if(nCurTermFreq_ > 0)
    {
        frequencies_[current_nocomp_block_pointer_++] = nCurTermFreq_;
        chunk_.encode(doc_ids_, frequencies_, positions_, current_nocomp_block_pointer_);
        pDocFreqDataPool_->addDFChunk(chunk_);
        if (pPosDataPool_)
            pPosDataPool_->addPOSChunk(chunk_);

        if(pSkipListWriter_ && nDF_ > 0 && nDF_ % skipInterval_ == 0)
        {
            if (pPosDataPool_)
                pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),pPosDataPool_->getLength());
            else
                pSkipListWriter_->addSkipPoint(chunk_.last_doc_id(),pDocFreqDataPool_->getLength(),0);
        }

        nCTF_ += nCurTermFreq_;
        nmaxTF_ = (nCurTermFreq_ > nmaxTF_) ? nCurTermFreq_ : nmaxTF_;
        nCurTermFreq_ = 0;
    }
}

}

NS_IZENELIB_IR_END
