#include <ir/index_manager/index/ForwardIndexReader.h>

using namespace izenelib::ir::indexmanager;

ForwardIndexReader::ForwardIndexReader(Directory* pDirectory)
{
    pDOCInput_ = pDirectory->openInput("forward.doc");
    pFDIInput_ = pDirectory->openInput("forward.fdi");
    pVOCInput_ = pDirectory->openInput("forward.voc");
    pPOSIInput_ = pDirectory->openInput("forward.pos");
}

ForwardIndexReader::~ForwardIndexReader()
{
    delete pDOCInput_;
    delete pFDIInput_;
    delete pVOCInput_;
    delete pPOSIInput_;
}

bool ForwardIndexReader::getTermOffsetList(unsigned int termId, docid_t docId, fieldid_t fid, std::vector<std::pair<unsigned int, unsigned int> >& offsetList)
{
    pDOCInput_->seek(docId*sizeof(fileoffset_t));
    fileoffset_t fdiOff = pDOCInput_->readLong();
    pFDIInput_->seek(fdiOff);

    int32_t numFields = pFDIInput_->readVInt();	//<NumFields(VInt32)>
    fileoffset_t vocOff = -1;
    fieldid_t f;
    for(int32_t i = 0;i<numFields;i++)
    {
        f = pFDIInput_->readVInt();				//<FieldNum(VInt32)>
        if(f == fid)
        {
            vocOff = pFDIInput_->readVLong();		//<FieldPositions(VInt64)>
                break;
        }
        else 
            pFDIInput_->readVLong();				//skip
     }

}


