#include <ir/index_manager/index/ForwardIndexWriter.h>

using namespace izenelib::ir::indexmanager;

ForwardIndexWriter::ForwardIndexWriter(Directory* pDirectory)
    :pDirectory_(pDirectory)
    ,forwardIndex_(NULL)
{
    size_t buffersize = 1024*1024*10;
    pDOCOutput_ = pDirectory_->createOutput("forward.doc", buffersize, "r+");
    pFDIOutput_ = pDirectory_->createOutput("forward.fdi", buffersize, "a+");
    pVOCOutput_ = pDirectory_->createOutput("forward.voc", buffersize, "a+");
    pPOSOutput_ = pDirectory_->createOutput("forward.pos", buffersize, "a+");
}

ForwardIndexWriter::~ForwardIndexWriter()
{
    if(pDOCOutput_)
        delete pDOCOutput_;
    if(pFDIOutput_)
        delete pFDIOutput_;
    if(pVOCOutput_)
        delete pVOCOutput_;
    if(pPOSOutput_)
        delete pPOSOutput_;

    if(forwardIndex_)
        delete forwardIndex_;
}

void ForwardIndexWriter::addDocument(docid_t docID)
{
    pDOCOutput_->seek(docID*sizeof(fileoffset_t));
    pDOCOutput_->writeLong(pFDIOutput_->getFilePointer());
}

void ForwardIndexWriter::addProperty(fieldid_t fid, boost::shared_ptr<LAInput> laInput)
{
    pFDIOutput_->writeVInt(fid);
    pFDIOutput_->writeVLong(pVOCOutput_->getFilePointer());

    if(!forwardIndex_)
        forwardIndex_ = new ForwardIndex;

   ForwardIndexOffset* pForwardIndexOffset = NULL;
    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        pForwardIndexOffset = (ForwardIndexOffset*)(*forwardIndex_)[iter->termId_];
        if (pForwardIndexOffset == NULL)
        {
            pForwardIndexOffset = new ForwardIndexOffset;
            (*forwardIndex_)[iter->termId_] = pForwardIndexOffset;
        }
        pForwardIndexOffset->push_back(make_pair(iter->wordOffset_, iter->byteOffset_));
    }

    size_t nNumTerms = forwardIndex_->size();

    pVOCOutput_->writeVInt(nNumTerms);

    unsigned int termId, lastTerm = 0;

    for(ForwardIndex::iterator iter = forwardIndex_->begin(); iter != forwardIndex_->end(); ++iter)
    {
        termId = iter->first;
        pVOCOutput_->writeVInt(termId - lastTerm);
        lastTerm = termId;
        pVOCOutput_->writeVLong(pPOSOutput_->getFilePointer());
        pForwardIndexOffset = iter->second;
        int numPosition = pForwardIndexOffset->size();
        pPOSOutput_->writeVInt(numPosition);	
        unsigned int offset, lastOffset = 0;
        for (int j = 0; j < numPosition; j++)
        {
            ///start offset
            offset = (*pForwardIndexOffset)[j].first;
            pPOSOutput_->writeVInt(offset - lastOffset);
            lastOffset = offset;
            ///end offset
            offset = (*pForwardIndexOffset)[j].second;
            pPOSOutput_->writeVInt(offset - lastOffset);
            lastOffset = offset;
        }
		
        delete pForwardIndexOffset;
    }

    forwardIndex_->clear();
}

void ForwardIndexWriter::flush()
{
    pDOCOutput_->flush();
    pFDIOutput_->flush();
    pVOCOutput_->flush();
    pPOSOutput_->flush();
}


