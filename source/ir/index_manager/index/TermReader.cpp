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

DiskTermReader::DiskTermReader()
        : TermReader()
        , pTermReaderImpl_(NULL)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(true)
{
}

DiskTermReader::DiskTermReader(TermReaderImpl* pTermReaderImpl)
        : TermReader()
        , pTermReaderImpl_(pTermReaderImpl)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(false)
{
}

DiskTermReader::~DiskTermReader()
{
    close();
    if ((pTermReaderImpl_)&&(ownTermReaderImpl_))
        delete pTermReaderImpl_;
}

void DiskTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl_ = new TermReaderImpl(pFieldInfo);
    pTermReaderImpl_->open(pDirectory, barrelname);
}

void DiskTermReader::close()
{
    pCurTermInfo_ = NULL;
}

bool DiskTermReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if ((pCurTermInfo_)&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

TermInfo* DiskTermReader::termInfo(Term* term)
{
    return pTermReaderImpl_->termInfo(term);
}

TermReader* DiskTermReader::clone()
{
    TermReader* pTermReader = new DiskTermReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    return pTermReader;
}


TermDocFreqs* DiskTermReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL)
        return NULL;
    TermDocFreqs* pTermDocs = new TermDocFreqs(this,pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    return pTermDocs;
}

TermPositions* DiskTermReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    if(pTermReaderImpl_->pInputDescriptor_ == NULL)
        return NULL;
    if(pTermReaderImpl_->pInputDescriptor_->getPPostingInput() == NULL)
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

    return static_cast<TermIterator*>(new DiskTermIterator(this));
}

void DiskTermReader::updateTermInfo(Term* term, count_t docFreq, fileoffset_t offset)
{
    pTermReaderImpl_->updateTermInfo(term, docFreq, offset);
}

TermReaderImpl::TermReaderImpl(FieldInfo* pFieldInfo)
        :pFieldInfo_(pFieldInfo)
        ,pTermTable_(NULL)
        ,pInputDescriptor_(NULL)
{}

TermReaderImpl::~TermReaderImpl()
{
    close();
}

void TermReaderImpl::open(Directory* pDirectory,const char* barrelname)
{
    close();///TODO

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
///InMemoryTermReader
InMemoryTermReader::InMemoryTermReader(void)
        : pIndexer_(NULL)
        , pCurTermInfo_(NULL)
        , pCurPosting_(NULL)
        , pTermInfo_(NULL)
{
}
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
    boost::mutex::scoped_lock lock(pIndexer_->getLock());
    //pInMem->flushLastDoc(false);
    TermDocFreqs* pTermDocs = new TermDocFreqs(this,pCurPosting_,*pCurTermInfo_);
    return pTermDocs;
}

TermPositions* InMemoryTermReader::termPositions()
{
    if( (pCurTermInfo_ == NULL)||(pCurPosting_ == NULL))
        return NULL;
    //InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting_;
    boost::mutex::scoped_lock lock(pIndexer_->getLock());
    //pInMem->flushLastDoc(false);
    TermPositions* pPositions = new TermPositions(this,pCurPosting_,*pCurTermInfo_);
	cout<<"in memory termPositions "<<pPositions<<endl;
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

