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

    if(pSkipListReader_)
    {
        delete pSkipListReader_;
        pSkipListReader_ = 0;	
    }

    if((termInfo.docFreq_ >= 4096)&&(termInfo.skipPointer_ != -1))
    {
        pDPInput->seek(termInfo.skipPointer_);
        pSkipListReader_ = new FixedBlockSkipListReader(pDPInput);
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

docid_t BlockPostingReader::decodeTo(docid_t target)
{
    int curr_chunk_num = blockDecoder_.curr_chunk();
    if (curr_chunk_num < blockDecoder_.num_chunks())
    {
        
    }
    return 0;
}

void BlockPostingReader::seekTo(FixedBlockSkipListReader* pSkipListReader)
{
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


