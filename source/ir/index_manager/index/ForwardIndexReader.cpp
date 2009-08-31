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

bool ForwardIndexReader::getTermOffset(unsigned int termId, docid_t docId, fieldid_t fid, std::deque<std::pair<unsigned int, unsigned int> >& offsetList)
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
    unsigned int pos = 0, charpos = 0;
    for(size_t i = 0; i < nNumPosition; i++)
    {
        pos += pPOSInput_->readVInt();
        unsigned int pos1 = pos;
        charpos += pPOSInput_->readVInt();
        unsigned int pos2 = charpos;
        offsetList.push_back(make_pair(pos1,pos2));
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

bool ForwardIndexReader::getTermOffsetList(const std::vector<unsigned int>& termIds, docid_t docId, fieldid_t fid, std::deque<std::deque<std::pair<unsigned int, unsigned int> > >& offsetList)
{
    if(!locateTermPosByDoc(docId,fid))
        return false;

    VocInfoMap vocInfo;
    retrieve_voc_by_doc(vocInfo);

    VocInfoMap::iterator vocInfoIterator = vocInfo.end();
    for(std::vector<unsigned int>::const_iterator iter = termIds.begin(); iter != termIds.end(); ++iter)
    {
        std::deque<std::pair<unsigned int, unsigned int> > offset;
        offsetList.push_back(offset);

        vocInfoIterator = vocInfo.find(*iter);

        if(vocInfoIterator == vocInfo.end())
            continue;
        pPOSInput_->seek(vocInfoIterator->second);
        std::deque<std::deque<std::pair<unsigned int, unsigned int> > >::reverse_iterator offsetListIterator = offsetList.rbegin();

        size_t nNumPosition = pPOSInput_->readVInt();
        unsigned int pos = 0, charpos = 0;
        for(size_t i = 0; i < nNumPosition; i++)
        {
            pos += pPOSInput_->readVInt();
            unsigned int pos1 = pos;
            charpos += pPOSInput_->readVInt();
            unsigned int pos2 = charpos;
            offsetListIterator->push_back(make_pair(pos1,pos2));
        }
    }
    return true;
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
        unsigned int pos = 0, charpos = 0;
        for(size_t i = 0; i < nNumPosition; i++)
        {
            pos += pPOSInput_->readVInt();
            unsigned int pos1 = pos;
            charpos += pPOSInput_->readVInt();
            unsigned int pos2 = charpos;
            pForwardIndexOffset->push_back(make_pair(pos1,pos2));
        }
        forwardIndex[iter->first] = pForwardIndexOffset;
    }
    return true;
}

