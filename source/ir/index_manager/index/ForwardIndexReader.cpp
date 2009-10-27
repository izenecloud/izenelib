#include <ir/index_manager/index/ForwardIndexReader.h>

using namespace izenelib::ir::indexmanager;

ForwardIndexReader::ForwardIndexReader(Directory* pDirectory)
    :pDirectory_(pDirectory)
{
    pDOCInput_ = pDirectory->openInput("forward.doc");
    pFDIInput_ = pDirectory->openInput("forward.fdi");
    pVOCInput_ = pDirectory->openInput("forward.voc");
    pPOSInput_ = pDirectory->openInput("forward.pos");
}

ForwardIndexReader::~ForwardIndexReader()
{
    delete pDOCInput_;
    delete pFDIInput_;
    delete pVOCInput_;
    delete pPOSInput_;
}

ForwardIndexReader* ForwardIndexReader::clone()
{
    return new ForwardIndexReader(pDirectory_);
}

inline bool ForwardIndexReader::locateTermPosByDoc(docid_t docId, fieldid_t fid)
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
    if(vocOff == -1)
        return false;
    pVOCInput_->seek(vocOff);
    return true;
}

bool ForwardIndexReader::getTermOffset(unsigned int termId, docid_t docId, fieldid_t fid, std::deque<unsigned int>& offsetList)
{
    if(!locateTermPosByDoc(docId,fid))
        return false;

    size_t nNumTerms = pVOCInput_->readInt();
    unsigned int tid = 0;
    fileoffset_t posOff = -1;
    for(size_t i = 0; i < nNumTerms; i++)
    {
        tid = pVOCInput_->readInt();
        if(tid == termId)
        {
            posOff = pVOCInput_->readLong();
            break;
        }
        else
            pVOCInput_->readLong();			//skip
    }
    if(posOff == -1)
        return false;
    pPOSInput_->seek(posOff);
    size_t nNumPosition = pPOSInput_->readVInt();
    offsetList.resize(nNumPosition);
    unsigned int pos = 0;
    for(size_t i = 0; i < nNumPosition; i++)
    {
        pos += pPOSInput_->readVInt();
        offsetList.push_back(pos);
    }
    return true;
}

inline void ForwardIndexReader::retrieve_voc_by_doc(VocInfoMap & vocInfo)
{
    size_t nNumTerms = pVOCInput_->readInt();
    unsigned int tid = 0;
    fileoffset_t posOff = -1;
    for(size_t i = 0; i < nNumTerms; i++)
    {
        tid = pVOCInput_->readInt();
        posOff = pVOCInput_->readLong();
        vocInfo.insert(rde::pair<unsigned int, fileoffset_t>(tid,posOff));
    }
}

bool ForwardIndexReader::getForwardIndexByDoc(docid_t docId, fieldid_t fid, ForwardIndex& forwardIndex)
{
    if(!locateTermPosByDoc(docId,fid))
        return false;
    
    VocInfoMap vocInfo;
    retrieve_voc_by_doc(vocInfo);

    for(VocInfoMap::iterator iter = vocInfo.begin(); iter != vocInfo.end(); ++iter)
    {
        pPOSInput_->seek(iter->second);
        size_t nNumPosition = pPOSInput_->readVInt();
        ForwardIndexOffset* pForwardIndexOffset = new ForwardIndexOffset;
        unsigned int pos = 0;
        for(size_t i = 0; i < nNumPosition; i++)
        {
            pos += pPOSInput_->readVInt();
            pForwardIndexOffset->push_back(pos);
        }
        forwardIndex[iter->first] = pForwardIndexOffset;
    }
    return true;
}

