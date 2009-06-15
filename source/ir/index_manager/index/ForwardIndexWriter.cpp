#include <ir/index_manager/index/ForwardIndexWriter.h>

using namespace izenelib::ir::indexmanager;

ForwardIndexWriter::ForwardIndexWriter(Directory* pDirectory)
    :pDirectory_(pDirectory)
    ,forwardIndexArray_(NULL)
{
    pDOCOutput_ = pDirectory_->createOutput("forward.doc", "r+");
    pFDIOutput_ = pDirectory_->createOutput("forward.fdi", "a+");
    pVOCOutput_ = pDirectory_->createOutput("forward.voc", "a+");
    pPOSOutput_ = pDirectory_->createOutput("forward.pos", "a+");
}

ForwardIndexWriter::~ForwardIndexWriter()
{
    close();

    if(forwardIndexArray_)
        delete forwardIndexArray_;
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

    if(!forwardIndexArray_)
        forwardIndexArray_ = new DynForwardIndexArray;

   ForwardIndex* pForwardIndex = NULL;
    for(LAInput::iterator iter = laInput->begin(); iter != laInput->end(); ++iter)
    {
        pForwardIndex = (ForwardIndex*)(*forwardIndexArray_)[iter->termId_];
        if (pForwardIndex == NULL)
        {
            pForwardIndex = new ForwardIndex;
            (*forwardIndexArray_)[iter->termId_] = pForwardIndex;
        }
        pForwardIndex->push_back(make_pair(iter->wordOffset_, iter->byteOffset_));
    }

    size_t nNumTerms = forwardIndexArray_->length();

    pVOCOutput_->writeVInt(nNumTerms);

    unsigned int termId, lastTerm = 0;

    DynForwardIndexArray::array_iterator iter = forwardIndexArray_->elements();
    while(iter.next())
    {
        termId = (termid_t)iter.position();
        pVOCOutput_->writeVInt(termId - lastTerm);
        lastTerm = termId;
        pVOCOutput_->writeVInt(pPOSOutput_->getFilePointer());
        pForwardIndex = iter.element();
        int numPosition = pForwardIndex->size();
        unsigned int offset, lastOffset = 0;
        for (int j = 0; j < numPosition; j++)
        {
            ///start offset
            offset = (*pForwardIndex)[j].first;
            pPOSOutput_->writeVInt(offset - lastOffset);
            lastOffset = offset;
            ///end offset
            offset = (*pForwardIndex)[j].second;
            pPOSOutput_->writeVInt(offset - lastOffset);
            lastOffset = offset;
        }
        delete pForwardIndex;
    }

    forwardIndexArray_->reset();
}

void ForwardIndexWriter::close()
{
    if(pDOCOutput_)
        delete pDOCOutput_;
    if(pFDIOutput_)
        delete pFDIOutput_;
    if(pVOCOutput_)
        delete pVOCOutput_;
    if(pPOSOutput_)
        delete pPOSOutput_;
}


