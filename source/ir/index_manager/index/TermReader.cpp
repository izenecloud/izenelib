#include <ir/index_manager/index/TermReader.h>

#include <boost/thread.hpp>

using namespace izenelib::ir::indexmanager;

TermReader::TermReader(void)
        : pFieldInfo_(NULL)
{

}

TermReader::TermReader(FieldInfo* pFieldInfo)
        : pFieldInfo_(pFieldInfo)
{

}

TermReader::~TermReader(void)
{

}

void TermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{}


//////////////////////////////////////////////////////////////////////////
///TermReaderImpl
TermReaderImpl::TermReaderImpl(FieldInfo* pFieldInfo)
        :pFieldInfo_(pFieldInfo)
        ,pTermTable_(NULL)
        ,pInputDescriptor_(NULL)
        ,pDirectory_(NULL)
{}

TermReaderImpl::~TermReaderImpl()
{
    close();
}

void TermReaderImpl::open(Directory* pDirectory,const char* barrelname)
{
    close();///TODO

    barrelName_ = barrelname;

    string bn = barrelname;

    IndexInput* pVocInput = pDirectory->openInput(bn + ".voc");
    pVocInput->seek(pFieldInfo_->getIndexOffset());
    fileoffset_t voffset = pVocInput->getFilePointer();
    ///begin read vocabulary descriptor
    nVocLength_ = pVocInput->readLong();
    nTermCount_ = (int32_t)pVocInput->readLong(); ///get total term count
    ///end read vocabulary descriptor
    pVocInput->seek(voffset - nVocLength_);///seek to begin of vocabulary data
    pTermTable_ = new TERM_TABLE[nTermCount_];
    freq_t df = 0;
    fileoffset_t dfiP = 0;
    termid_t tid = 0;
    ///read term table
    for (int32_t i = 0;i < nTermCount_;i++)
    {
        tid = pVocInput->readInt();
        pTermTable_[i].tid = tid;
        df = pVocInput->readInt();
        dfiP = pVocInput->readLong();
        pTermTable_[i].ti.set(df,dfiP);
    }
    delete pVocInput;

    pInputDescriptor_ = new InputDescriptor(true);
    pInputDescriptor_->setDPostingInput(pDirectory->openInput(bn + ".dfp"));
    pInputDescriptor_->setPPostingInput(pDirectory->openInput(bn + ".pop"));

}

void TermReaderImpl::reopen()
{
    if(pDirectory_)
        open(pDirectory_, barrelName_.c_str());
}

void TermReaderImpl::updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset)
{
    termid_t tid = term->getValue();

    int32_t start = 0, end = nTermCount_ - 1;
    int32_t mid = (start + end)/2;
    while (start <= end)
    {
        mid = (start + end)/2;
        if (pTermTable_[mid].tid == tid)
        {
            pTermTable_[mid].ti.set(docFreq, offset);
            return;
        }
        if (pTermTable_[mid].tid > tid)
        {
            end = mid - 1;
        }
        else
        {
            start = mid + 1;
        }
    }
}

void TermReaderImpl::close()
{
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
    }

    if (pTermTable_)
    {
        delete[] pTermTable_;
        pTermTable_ = NULL;
        nTermCount_ = 0;
    }
}

TermInfo* TermReaderImpl::termInfo(Term* term)
{
    if (strcmp(term->getField(),pFieldInfo_->getName()))
        return NULL;

    termid_t tid = term->getValue();

    int32_t start = 0,end = nTermCount_ - 1;
    int32_t mid = (start + end)/2;
    ///tid == 0 means we return the last term to see whether
    ///the index is consistent;
    if(MAX_TERMID == tid) return &(pTermTable_[end].ti);
	
    while (start <= end)
    {
        mid = (start + end)/2;
        if (pTermTable_[mid].tid == tid)
        {
            return &(pTermTable_[mid].ti);
        }
        if (pTermTable_[mid].tid > tid)
        {
            end = mid - 1;
        }
        else
        {
            start = mid + 1;
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////
///VocReader
VocReader::VocReader(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
        : TermReader()
        , pTermReaderImpl_(NULL)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(true)
        , pInputDescriptor_(NULL)
{
    open(pDirectory, barrelname, pFieldInfo);
}

VocReader::VocReader(TermReaderImpl* pTermReaderImpl)
        : TermReader()
        , pTermReaderImpl_(pTermReaderImpl)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(false)
        , pInputDescriptor_(pTermReaderImpl->pInputDescriptor_->clone())
{
}

VocReader::~VocReader()
{
    close();
    if ((pTermReaderImpl_)&&(ownTermReaderImpl_))
        delete pTermReaderImpl_;
    if (pInputDescriptor_)
        delete pInputDescriptor_;
}

void VocReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl_ = new TermReaderImpl(pFieldInfo);
    pTermReaderImpl_->open(pDirectory, barrelname);
    pInputDescriptor_ = pTermReaderImpl_->pInputDescriptor_->clone();
}

void VocReader::reopen()
{
    pTermReaderImpl_->reopen();
}

void VocReader::close()
{
    pCurTermInfo_ = NULL;
}

bool VocReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if ((pCurTermInfo_)&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

TermInfo* VocReader::termInfo(Term* term)
{
    return pTermReaderImpl_->termInfo(term);
}

TermReader* VocReader::clone()
{
    TermReader* pTermReader = new VocReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    return pTermReader;
}


TermDocFreqs* VocReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    //TermDocFreqs* pTermDocs = new TermDocFreqs(this,pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    return new TermDocFreqs(this,pInputDescriptor_->clone(),*pCurTermInfo_);;
}

TermPositions* VocReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    //return new TermPositions(this,pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    return new TermPositions(this,pInputDescriptor_->clone(),*pCurTermInfo_);
}

freq_t VocReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermIterator* VocReader::termIterator(const char* field)
{
    if ((field != NULL) && (strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;

    return static_cast<TermIterator*>(new VocIterator(this));
}

void VocReader::updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset)
{
    pTermReaderImpl_->updateTermInfo(term, docFreq, offset);
}

//////////////////////////////////////////////////////////////////////////
///SparseTermReaderImpl
SparseTermReaderImpl::SparseTermReaderImpl(FieldInfo* pFieldInfo)
        :pFieldInfo_(pFieldInfo)
        ,sparseTermTable_(NULL)
        ,pInputDescriptor_(NULL)
        ,pDirectory_(NULL)
{}

SparseTermReaderImpl::~SparseTermReaderImpl()
{
    close();
}

void SparseTermReaderImpl::open(Directory* pDirectory,const char* barrelname)
{
    close();

    pDirectory_ = pDirectory;

    barrelName_ = barrelname;

    IndexInput* pVocInput = pDirectory->openInput(barrelName_ + ".voc");
    pVocInput->seek(pFieldInfo_->getIndexOffset());
    fileoffset_t voffset = pVocInput->getFilePointer();
    ///begin read vocabulary descriptor
    nVocLength_ = pVocInput->readLong();
    nTermCount_ = (int32_t)pVocInput->readLong(); ///get total term count
    nSparseSize_ = nTermCount_>>9;
    ///end read vocabulary descriptor
    nBeginOfVoc_ = voffset - nVocLength_;
    pVocInput->seek(nBeginOfVoc_);///seek to begin of vocabulary data
    sparseTermTable_ = new TERM_TABLE[nSparseSize_];
    freq_t df = 0;
    fileoffset_t dfiP = 0;
    termid_t tid = 0;
    ///read term table
    for (int32_t i = 0;i < nTermCount_;i++)
    {
        tid = pVocInput->readInt();
        df = pVocInput->readInt();
        dfiP = pVocInput->readLong();
        if((i+1)%SPARSE_FACTOR == 0)
        {
            sparseTermTable_[i>>9].tid = tid;
            sparseTermTable_[i>>9].ti.set(df,dfiP);
        }
    }
    delete pVocInput;

    pInputDescriptor_ = new InputDescriptor(true);
    pInputDescriptor_->setDPostingInput(pDirectory->openInput(barrelName_ + ".dfp"));
    pInputDescriptor_->setPPostingInput(pDirectory->openInput(barrelName_ + ".pop"));

}

void SparseTermReaderImpl::reopen()
{
    if(pDirectory_)
        open(pDirectory_, barrelName_.c_str());
}

void SparseTermReaderImpl::close()
{
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
    }

    if (sparseTermTable_)
    {
        delete[] sparseTermTable_;
        sparseTermTable_ = NULL;
        nTermCount_ = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
///DiskTermReader
DiskTermReader::DiskTermReader(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
        : TermReader()
        , pTermReaderImpl_(NULL)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(true)
        , pVocInput_(NULL)
{
    open(pDirectory, barrelname, pFieldInfo);
    pVocInput_ = pDirectory->openInput(pTermReaderImpl_->barrelName_ + ".voc",1025*VOC_ENTRY_LENGTH);
    bufferTermTable_ = new TERM_TABLE[1025];
    sparseTermTable_ = pTermReaderImpl_->sparseTermTable_;
    nTermCount_ = pTermReaderImpl_->nTermCount_;
    nBeginOfVoc_ = pTermReaderImpl_->nBeginOfVoc_;
}

DiskTermReader::DiskTermReader(SparseTermReaderImpl* pTermReaderImpl)
        : TermReader()
        , pTermReaderImpl_(pTermReaderImpl)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(false)
        , pVocInput_(NULL)
{
    pVocInput_ = pTermReaderImpl_->pDirectory_->openInput(pTermReaderImpl_->barrelName_ + ".voc",1025*VOC_ENTRY_LENGTH);
    bufferTermTable_ = new TERM_TABLE[1025];
    sparseTermTable_ = pTermReaderImpl_->sparseTermTable_;
    nTermCount_ = pTermReaderImpl_->nTermCount_;
    nBeginOfVoc_ = pTermReaderImpl_->nBeginOfVoc_;
}

DiskTermReader::~DiskTermReader()
{
    close();
    if (pTermReaderImpl_&&ownTermReaderImpl_)
        delete pTermReaderImpl_;
    sparseTermTable_ = NULL;
    if(pVocInput_)
        delete pVocInput_;
    if(bufferTermTable_)
        delete[] bufferTermTable_;
}

void DiskTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl_ = new SparseTermReaderImpl(pFieldInfo);
    pTermReaderImpl_->open(pDirectory, barrelname);
}

void DiskTermReader::reopen()
{
    pTermReaderImpl_->reopen();
}

void DiskTermReader::close()
{
    pCurTermInfo_ = NULL;
}

bool DiskTermReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if (pCurTermInfo_&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

inline TermInfo* DiskTermReader::searchBuffer(termid_t termId, int end)
{
    int32_t start = 0;
    int32_t mid = (start + end)/2;

    while (start <= end)
    {
        mid = (start + end)/2;
        if (bufferTermTable_[mid].tid == termId)
        {
            return &(bufferTermTable_[mid].ti);
        }
        if (bufferTermTable_[mid].tid > termId)
        {
            end = mid - 1;
        }
        else
        {
            start = mid + 1;
        }
    }
    return NULL;
}

inline int DiskTermReader::fillBuffer(int pos)
{
    int begin = (pos-SPARSE_FACTOR) > 0 ? (pos-SPARSE_FACTOR) : 0;
    int end = (pos+SPARSE_FACTOR) >= nTermCount_ ? nTermCount_ : (pos+SPARSE_FACTOR);
    pVocInput_->reset();
    pVocInput_->seekInternal(pTermReaderImpl_->nBeginOfVoc_ + begin*VOC_ENTRY_LENGTH);
    freq_t df = 0;
    fileoffset_t dfiP = 0;
    termid_t tid = 0;

    for(int i = begin, j = 0; i <= end; ++i, ++j)
    {
        tid = pVocInput_->readInt();
        bufferTermTable_[j].tid = tid;
        df = pVocInput_->readInt();
        dfiP = pVocInput_->readLong();
        bufferTermTable_[j].ti.set(df,dfiP);
    }
    return end - begin;
}

TermInfo* DiskTermReader::termInfo(Term* term)
{
    if (strcmp(term->getField(),pFieldInfo_->getName()))
        return NULL;
    termid_t termId = term->getValue();
    ///tid == 0 means we return the last term to see whether
    ///the index is consistent;
    if(MAX_TERMID == termId) return &(sparseTermTable_[pTermReaderImpl_->nSparseSize_-1].ti);

    unsigned int delta = 1;
    while((int)delta < nTermCount_)
        delta = delta << 1;
    delta = delta >> 1;
    unsigned int pos = delta - 1;
    int cmpres = 0;
    if(delta <= 512)
    {
        int end = fillBuffer(pos);
        return searchBuffer(termId, end);
    }
	
    while(delta > 0)
    {
        if((pos&511) == 511)
        {
            termid_t tid = sparseTermTable_[pos>>9].tid;
            if(termId == tid)
                return &(sparseTermTable_[pos>>9].ti);
            else if (termId > tid)
                cmpres = 1;
            else
                cmpres = -1;
        }  
        if(delta <= 512)
        {
            int end = fillBuffer(pos);
            return searchBuffer(termId, end);
        }
        delta = delta>>1;
	  
        if(cmpres == 1)
        {
            pos += delta;
            if((int)pos >= nTermCount_)  
            {
                do
                {
                    pos -= delta; 		   
                    delta = delta>>1;
                    pos += delta;
                }while(((int32_t)pos >= nTermCount_)&&(delta > 0));
            }
        }
        else
            pos -= delta;
    }

    return NULL;
}

TermReader* DiskTermReader::clone()
{
    TermReader* pTermReader = new DiskTermReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    return pTermReader;
}


TermDocFreqs* DiskTermReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    return new TermDocFreqs(this,pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
}

TermPositions* DiskTermReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    return new TermPositions(this,pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
}

freq_t DiskTermReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermIterator* DiskTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;

    return static_cast<TermIterator*>(new DiskTermIterator(pTermReaderImpl_->pDirectory_,
                       pTermReaderImpl_->barrelName_.c_str(),
                       pTermReaderImpl_->pFieldInfo_));
}


//////////////////////////////////////////////////////////////////////////
///InMemoryTermReader
InMemoryTermReader::InMemoryTermReader(const char* field,FieldIndexer* pIndexer)
        : field_(field)
        , pIndexer_(pIndexer)
        , pCurTermInfo_(NULL)
        , pCurPosting_(NULL)
        , pTermInfo_(NULL)
{
}

InMemoryTermReader::~InMemoryTermReader(void)
{
    close();
}

void InMemoryTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{

}

TermIterator* InMemoryTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (field_ != field))
        return NULL;
    return new InMemoryTermIterator(this);
}

bool InMemoryTermReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if ((pCurTermInfo_)&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

TermDocFreqs* InMemoryTermReader::termDocFreqs()
{
    if( (pCurTermInfo_ == NULL)||(pCurPosting_ == NULL))
        return NULL;

    //InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting_;
    //pInMem->flushLastDoc(false);
    TermDocFreqs* pTermDocs = new TermDocFreqs(this,pCurPosting_,*pCurTermInfo_);
    return pTermDocs;
}

TermPositions* InMemoryTermReader::termPositions()
{
    if( (pCurTermInfo_ == NULL)||(pCurPosting_ == NULL))
        return NULL;
    //InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting_;
    //pInMem->flushLastDoc(false);
    TermPositions* pPositions = new TermPositions(this,pCurPosting_,*pCurTermInfo_);
    return pPositions;
}
freq_t InMemoryTermReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermInfo* InMemoryTermReader::termInfo(Term* term)
{
    if (strcasecmp(term->getField(),field_.c_str()))
        return NULL;

    termid_t tid = term->getValue();

    InMemoryPostingMap::iterator postingIter = pIndexer_->postingMap_.find(tid);
    if(postingIter == pIndexer_->postingMap_.end())
        return NULL;
    pCurPosting_ = postingIter->second;
    if (!pCurPosting_ || (pCurPosting_->hasNoChunk() == true))
        return NULL;
    if (!pTermInfo_)
        pTermInfo_ = new TermInfo;
    pTermInfo_->set(pCurPosting_->docFreq(),-1);
    return pTermInfo_;
}

InMemoryPosting* InMemoryTermReader::inMemoryPosting()
{
    if (!pCurPosting_ || (pCurPosting_->hasNoChunk() == true))
        return NULL;
    return pCurPosting_;
}

void InMemoryTermReader::close()
{
    if (pTermInfo_)
    {
        delete pCurTermInfo_;
        pCurTermInfo_ = NULL;
    }
    pCurTermInfo_ = NULL;
}

TermReader* InMemoryTermReader::clone()
{
    return new InMemoryTermReader(field_.c_str(), pIndexer_);
}

