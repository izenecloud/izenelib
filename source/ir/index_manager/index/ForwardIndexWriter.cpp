#include <ir/index_manager/index/ForwardIndexWriter.h>

using namespace izenelib::ir::indexmanager;

ForwardIndexWriter::ForwardIndexWriter(Directory* pDirectory)
    :pDirectory_(pDirectory)
{
    pDOCOutput_ = pDirectory_->createOutput("forward.doc", "r+");
    pFDIOutput_ = pDirectory_->createOutput("forward.fdi", "a+");
    pVOCOutput_ = pDirectory_->createOutput("forward.voc", "a+");
    pPOSOutput_ = pDirectory_->createOutput("forward.pos", "a+");
}

ForwardIndexWriter::~ForwardIndexWriter()
{
    close();
}

void ForwardIndexWriter::addDocument(docid_t docID)
{
    pDOCOutput_->seek(docID*sizeof(fileoffset_t));
    pDOCOutput_->writeLong(pFDIOutput_->getFilePointer());
}

void ForwardIndexWriter::addField(fieldid_t fid, ForwardIndex& forwardIndex)
{
    pFDIOutput_->writeVInt(0);
    pFDIOutput_->writeVInt(fid);
    pFDIOutput_->writeVLong(pVOCOutput_->getFilePointer());

    TermArray& termIdList = forwardIndex.getTermIdArray();

    size_t nNumTerms = forwardIndex.getNumTerms();

    pVOCOutput_->writeVInt(nNumTerms);

    unsigned int termId, lastTerm = 0;

    TermArray::array_iterator iter = termIdList.elements();
    while(iter.next())
    {
        termid_t termId = (termid_t)iter.element();
        pVOCOutput_->writeVInt(termId - lastTerm);
        lastTerm = termId;
        pVOCOutput_->writeVInt(pPOSOutput_->getFilePointer());
        std::deque<std::pair<WordOffset, CharOffset> >* termOffsetList = forwardIndex.getTermOffsetListByTermId(termId);
        int numPosition = termOffsetList->size();
        unsigned int offset, lastOffset = 0;
        for (int j = 0; j < numPosition; j++)
        {
            offset = (*termOffsetList)[j].first;
            pPOSOutput_->writeVInt(offset - lastOffset);
            lastOffset = offset;
        }
    }

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


