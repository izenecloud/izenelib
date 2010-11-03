#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/store/FSDirectory.h>

#include <boost/thread.hpp>
#include <util/ThreadModel.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

TermReader::TermReader()
        : pFieldInfo_(NULL)
        , pDocFilter_(NULL)
        , pBarrelInfo_(NULL)
{

}

TermReader::TermReader(FieldInfo* pFieldInfo)
        : pFieldInfo_(pFieldInfo)
        , pDocFilter_(NULL)
        , pBarrelInfo_(NULL)
{

}

TermReader::~TermReader(void)
{

}

void TermReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
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

    termid_t tid = 0;
    freq_t df = 0;
    freq_t ctf = 0;
    docid_t lastdoc = BAD_DOCID;
    freq_t skipLevel = 0;
    fileoffset_t skipPointer = 0;
    fileoffset_t docPointer = 0;
    freq_t docPostingLen = 0;
    fileoffset_t positionPointer = 0;
    freq_t positionPostingLen = 0;	
    ///read term table
    for (int32_t i = 0;i < nTermCount_;i++)
    {
        tid = pVocInput->readInt();
        df = pVocInput->readInt();
        ctf = pVocInput->readInt();
        lastdoc = pVocInput->readInt();
        skipLevel = pVocInput->readInt();
        skipPointer = pVocInput->readLong();
        docPointer = pVocInput->readLong();
        docPostingLen = pVocInput->readInt();
        positionPointer = pVocInput->readLong();
        positionPostingLen = pVocInput->readInt();

        pTermTable_[i].tid = tid;
        pTermTable_[i].ti.set(df,ctf,lastdoc,skipLevel,skipPointer,docPointer,docPostingLen,positionPointer,positionPostingLen);
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
VocReader::VocReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
        : TermReader()
        , pTermReaderImpl_(NULL)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(true)
        , pInputDescriptor_(NULL)
{
    pBarrelInfo_ = pBarrelInfo;
    open(pDirectory, pBarrelInfo, pFieldInfo);
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

void VocReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl_ = new TermReaderImpl(pFieldInfo);
    pTermReaderImpl_->open(pDirectory, pBarrelInfo->getName().c_str());
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
    RTDiskPostingReader* pPosting = 
        new RTDiskPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(DOCUMENT_LEVEL),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());

    TermDocFreqs* pTermDoc = 
      new TermDocFreqs(pPosting,*pCurTermInfo_);
	
    return pTermDoc;
}

TermPositions* VocReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;

    RTDiskPostingReader* pPosting = 
        new RTDiskPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(DOCUMENT_LEVEL),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());

    TermPositions* pTermPos = 
      new TermPositions(pPosting,*pCurTermInfo_);
    return pTermPos;
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
    TermIterator* pIterator = static_cast<TermIterator*>(new VocIterator(this));
    pIterator->setSkipInterval(skipInterval_);
    pIterator->setMaxSkipLevel(maxSkipLevel_);
    return pIterator;
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
    termid_t tid = 0;
    freq_t df = 0;
    freq_t ctf = 0;
    docid_t lastdoc = BAD_DOCID;
    freq_t skipLevel = 0;
    fileoffset_t skipPointer = 0;
    fileoffset_t docPointer = 0;
    freq_t docPostingLen = 0;
    fileoffset_t positionPointer = 0;
    freq_t positionPostingLen = 0;	
    ///read term table
    for (int32_t i = 0;i < nTermCount_;i++)
    {
        tid = pVocInput->readInt();
        df = pVocInput->readInt();
        ctf = pVocInput->readInt();
        lastdoc = pVocInput->readInt();
        skipLevel = pVocInput->readInt();
        skipPointer = pVocInput->readLong();
        docPointer = pVocInput->readLong();
        docPostingLen = pVocInput->readInt();
        positionPointer = pVocInput->readLong();
        positionPostingLen = pVocInput->readInt();

        if((i+1)%SPARSE_FACTOR == 0)
        {
            sparseTermTable_[i>>9].tid = tid;
            sparseTermTable_[i>>9].ti.set(df,ctf,lastdoc,skipLevel,skipPointer,docPointer,docPostingLen,positionPointer,positionPostingLen);
        }
    }
    delete pVocInput;

    pInputDescriptor_ = new InputDescriptor(true);
    if(dynamic_cast<FSDirectory*>(pDirectory)->isMMapEnable())
    {
        pInputDescriptor_->setDPostingInput(dynamic_cast<FSDirectory*>(pDirectory)->openMMapInput(barrelName_ + ".dfp"));
        pInputDescriptor_->setPPostingInput(dynamic_cast<FSDirectory*>(pDirectory)->openMMapInput(barrelName_ + ".pop"));
    }
    else
    {
        pInputDescriptor_->setDPostingInput(pDirectory->openInput(barrelName_ + ".dfp"));
        pInputDescriptor_->setPPostingInput(pDirectory->openInput(barrelName_ + ".pop"));
    }

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
///RTDiskTermReader
RTDiskTermReader::RTDiskTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
        : TermReader()
        , pTermReaderImpl_(NULL)
        , pCurTermInfo_(NULL)
        , ownTermReaderImpl_(true)
        , pVocInput_(NULL)
{
    pBarrelInfo_ = pBarrelInfo;
    open(pDirectory, pBarrelInfo->getName().c_str(), pFieldInfo);
    pTermReaderImpl_->pInputDescriptor_->setBarrelInfo(pBarrelInfo);
    pVocInput_ = pDirectory->openInput(pTermReaderImpl_->barrelName_ + ".voc",1025*VOC_ENTRY_LENGTH);
    bufferTermTable_ = new TERM_TABLE[1025];
    sparseTermTable_ = pTermReaderImpl_->sparseTermTable_;
    nTermCount_ = pTermReaderImpl_->nTermCount_;
    nBeginOfVoc_ = pTermReaderImpl_->nBeginOfVoc_;
}

RTDiskTermReader::RTDiskTermReader(SparseTermReaderImpl* pTermReaderImpl)
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

RTDiskTermReader::~RTDiskTermReader()
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

void RTDiskTermReader::open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
{
    setFieldInfo(pFieldInfo);

    pTermReaderImpl_ = new SparseTermReaderImpl(pFieldInfo);
    pTermReaderImpl_->open(pDirectory, barrelname);
}

void RTDiskTermReader::reopen()
{
    pTermReaderImpl_->reopen();
}

void RTDiskTermReader::close()
{
    pCurTermInfo_ = NULL;
}

bool RTDiskTermReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if (pCurTermInfo_&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

TermInfo* RTDiskTermReader::searchBuffer(termid_t termId, int end)
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

int RTDiskTermReader::fillBuffer(int pos)
{
    int begin = (pos-SPARSE_FACTOR) > 0 ? (pos-SPARSE_FACTOR) : 0;
    int end = (pos+SPARSE_FACTOR) >= (nTermCount_-1) ? (nTermCount_-1) : (pos+SPARSE_FACTOR);
    pVocInput_->reset();
    pVocInput_->seekInternal(pTermReaderImpl_->nBeginOfVoc_ + begin*VOC_ENTRY_LENGTH);
    //pVocInput_->seek(pTermReaderImpl_->nBeginOfVoc_ + begin*VOC_ENTRY_LENGTH);

    termid_t tid = 0;
    freq_t df = 0;
    freq_t ctf = 0;
    docid_t lastdoc = BAD_DOCID;
    freq_t skipLevel = 0;
    fileoffset_t skipPointer = 0;
    fileoffset_t docPointer = 0;
    freq_t docPostingLen = 0;
    fileoffset_t positionPointer = 0;
    freq_t positionPostingLen = 0;	

    for(int i = begin, j = 0; i <= end; ++i, ++j)
    {
        tid = pVocInput_->readInt();
        bufferTermTable_[j].tid = tid;
        df = pVocInput_->readInt();
        ctf = pVocInput_->readInt();
        lastdoc = pVocInput_->readInt();
        skipLevel = pVocInput_->readInt();
        skipPointer = pVocInput_->readLong();
        docPointer = pVocInput_->readLong();
        docPostingLen = pVocInput_->readInt();
        positionPointer = pVocInput_->readLong();
        positionPostingLen = pVocInput_->readInt();

        bufferTermTable_[j].ti.set(df,ctf,lastdoc,skipLevel,skipPointer,docPointer,docPostingLen,positionPointer,positionPostingLen);
    }
    return end - begin;
}

TermInfo* RTDiskTermReader::termInfo(Term* term)
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

TermReader* RTDiskTermReader::clone()
{
    TermReader* pTermReader = new RTDiskTermReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    pTermReader->setDocFilter(pDocFilter_);
    pTermReader->setSkipInterval(skipInterval_);
    pTermReader->setMaxSkipLevel(maxSkipLevel_);
    pTermReader->setBarrelInfo(pBarrelInfo_);
    return pTermReader;
}

TermDocFreqs* RTDiskTermReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    RTDiskPostingReader* pPosting = 
        new RTDiskPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(DOCUMENT_LEVEL),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());
	
    TermDocFreqs* pTermDoc = 
        new TermDocFreqs(pPosting,*pCurTermInfo_);
    return pTermDoc;
}

TermPositions* RTDiskTermReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;

    RTDiskPostingReader* pPosting = 
        new RTDiskPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());

    TermPositions* pTermPos = 
      new TermPositions(pPosting,*pCurTermInfo_);
    return pTermPos;
}

freq_t RTDiskTermReader::docFreq(Term* term)
{
    if(pCurTermInfo_)
    {
        return pCurTermInfo_->docFreq();
    }
    else
    {
        TermInfo* ti = termInfo(term);
        if(ti)
            return ti->docFreq();
    }
    return 0;
}

TermIterator* RTDiskTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;

    TermIterator* pIterator = static_cast<TermIterator*>(new RTDiskTermIterator(pTermReaderImpl_->pDirectory_,
                                                             pTermReaderImpl_->barrelName_.c_str(),
                                                             pTermReaderImpl_->pFieldInfo_));
    pIterator->setSkipInterval(skipInterval_);
    pIterator->setMaxSkipLevel(maxSkipLevel_);
    return pIterator;
}


//////////////////////////////////////////////////////////////////////////
///MemTermReader
MemTermReader::MemTermReader(const char* field,FieldIndexer* pIndexer)
        : field_(field)
        , pIndexer_(pIndexer)
        , pCurTermInfo_(NULL)
        , pCurPosting_(NULL)
        , pTermInfo_(NULL)
{
}

MemTermReader::~MemTermReader(void)
{
    close();
}

void MemTermReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{

}

TermIterator* MemTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (field_ != field))
        return NULL;

    TermIterator* pIterator = new MemTermIterator(this);
    pIterator->setSkipInterval(skipInterval_);
    pIterator->setMaxSkipLevel(maxSkipLevel_);
    return pIterator;
}

bool MemTermReader::seek(Term* term)
{
    pCurTermInfo_ = termInfo(term);
    if ((pCurTermInfo_)&&(pCurTermInfo_->docFreq() > 0))
        return true;
    return false;
}

TermDocFreqs* MemTermReader::termDocFreqs()
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->rwLock_);

    if( (pCurTermInfo_ == NULL)||(pCurPosting_ == NULL))
        return NULL;

    //InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting_;
    //pInMem->flushLastDoc(false);
    PostingReader* pPosting = pCurPosting_->createPostingReader();
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());
    TermDocFreqs* pTermDocs = new TermDocFreqs(pPosting,*pCurTermInfo_);
    pTermDocs->setSkipInterval(skipInterval_);
    pTermDocs->setMaxSkipLevel(maxSkipLevel_);
    return pTermDocs;
}

TermPositions* MemTermReader::termPositions()
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->rwLock_);

    if( (pCurTermInfo_ == NULL)||(pCurPosting_ == NULL))
        return NULL;
    //InMemoryPosting* pInMem = (InMemoryPosting*)pCurPosting_;
    //pInMem->flushLastDoc(false);
    PostingReader* pPosting = pCurPosting_->createPostingReader();
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());
    TermPositions* pPositions = new TermPositions(pPosting,*pCurTermInfo_);
    pPositions->setSkipInterval(skipInterval_);
    pPositions->setMaxSkipLevel(maxSkipLevel_);
    return pPositions;
}
freq_t MemTermReader::docFreq(Term* term)
{
    TermInfo* ti = termInfo(term);
    if (ti)
    {
        return ti->docFreq();
    }
    return 0;
}

TermInfo* MemTermReader::termInfo(Term* term)
{
    izenelib::util::ScopedReadLock<izenelib::util::ReadWriteLock> lock(pIndexer_->rwLock_);

    if (strcasecmp(term->getField(),field_.c_str()))
        return NULL;

    termid_t tid = term->getValue();

    InMemoryPostingMap::iterator postingIter = pIndexer_->postingMap_.find(tid);
    if(postingIter == pIndexer_->postingMap_.end())
        return NULL;
    pCurPosting_ = postingIter->second;
    if (!pCurPosting_ || (pCurPosting_->isEmpty() == true))
        return NULL;
    if (!pTermInfo_)
        pTermInfo_ = new TermInfo;

    pTermInfo_->set(
                                pCurPosting_->docFreq(),
                                pCurPosting_->getCTF(),
                                pCurPosting_->lastDocID(),
                                pCurPosting_->getSkipLevel(),
                                -1,-1,0,-1,0);
    return pTermInfo_;
}

void MemTermReader::close()
{
    if (pTermInfo_)
    {
        delete pCurTermInfo_;
        pCurTermInfo_ = NULL;
    }
    pCurTermInfo_ = NULL;
}

TermReader* MemTermReader::clone()
{
    TermReader* pTermReader = new MemTermReader(field_.c_str(), pIndexer_);
    pTermReader->setFieldInfo(pFieldInfo_);
    pTermReader->setDocFilter(pDocFilter_);
    pTermReader->setSkipInterval(skipInterval_);
    pTermReader->setMaxSkipLevel(maxSkipLevel_);
    return pTermReader;
}


//////////////////////////////////////////////////////////////////////////
///BlockTermReader
BlockTermReader::BlockTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
        : RTDiskTermReader(pDirectory,pBarrelInfo,pFieldInfo)
{
}

BlockTermReader::BlockTermReader(SparseTermReaderImpl* pTermReaderImpl)
        : RTDiskTermReader(pTermReaderImpl)
{
}


TermReader* BlockTermReader::clone()
{
    TermReader* pTermReader = new BlockTermReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    pTermReader->setDocFilter(pDocFilter_);
    pTermReader->setSkipInterval(skipInterval_);
    pTermReader->setMaxSkipLevel(maxSkipLevel_);
    pTermReader->setBarrelInfo(pBarrelInfo_);
    return pTermReader;
}

TermDocFreqs* BlockTermReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    BlockPostingReader* pPosting = 
        new BlockPostingReader(pTermReaderImpl_->pInputDescriptor_->clone(DOCUMENT_LEVEL),*pCurTermInfo_, DOCUMENT_LEVEL);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());
    TermDocFreqs* pTermDoc = 
        new TermDocFreqs(pPosting,*pCurTermInfo_);
    return pTermDoc;
}

TermPositions* BlockTermReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;

    BlockPostingReader* pPosting = 
        new BlockPostingReader(pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());

    TermPositions* pTermPos = 
      new TermPositions(pPosting,*pCurTermInfo_);
    pTermPos->setUseFixedPosBuffer(false);/// for block based encoding, we can not use fixed buffer to decode positions
    return pTermPos;
}

TermIterator* BlockTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;

    TermIterator* pIterator = static_cast<TermIterator*>(new BlockTermIterator(pTermReaderImpl_->pDirectory_,
                                                             pTermReaderImpl_->barrelName_.c_str(),
                                                             pTermReaderImpl_->pFieldInfo_));
    pIterator->setSkipInterval(skipInterval_);
    pIterator->setMaxSkipLevel(maxSkipLevel_);
    return pIterator;
}


//////////////////////////////////////////////////////////////////////////
///ChunkTermReader
ChunkTermReader::ChunkTermReader(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
        : RTDiskTermReader(pDirectory,pBarrelInfo,pFieldInfo)
{
}

ChunkTermReader::ChunkTermReader(SparseTermReaderImpl* pTermReaderImpl)
        : RTDiskTermReader(pTermReaderImpl)
{
}

TermReader* ChunkTermReader::clone()
{
    TermReader* pTermReader = new ChunkTermReader(pTermReaderImpl_);
    pTermReader->setFieldInfo(pFieldInfo_);
    pTermReader->setDocFilter(pDocFilter_);
    pTermReader->setSkipInterval(skipInterval_);
    pTermReader->setMaxSkipLevel(maxSkipLevel_);
    pTermReader->setBarrelInfo(pBarrelInfo_);
    return pTermReader;
}

TermDocFreqs* ChunkTermReader::termDocFreqs()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;
    ChunkPostingReader* pPosting = 
        new ChunkPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(DOCUMENT_LEVEL),*pCurTermInfo_, DOCUMENT_LEVEL);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());
	
    TermDocFreqs* pTermDoc = 
        new TermDocFreqs(pPosting,*pCurTermInfo_);
    return pTermDoc;
}

TermPositions* ChunkTermReader::termPositions()
{
    if (pCurTermInfo_ == NULL || pTermReaderImpl_ == NULL )
        return NULL;

    ChunkPostingReader* pPosting = 
        new ChunkPostingReader(skipInterval_, maxSkipLevel_, pTermReaderImpl_->pInputDescriptor_->clone(),*pCurTermInfo_);
    if(getDocFilter())
        pPosting->setFilter(getDocFilter());

    TermPositions* pTermPos = 
      new TermPositions(pPosting,*pCurTermInfo_);
    pTermPos->setUseFixedPosBuffer(false);/// for block based encoding, we can not use fixed buffer to decode positions
    return pTermPos;
}

TermIterator* ChunkTermReader::termIterator(const char* field)
{
    if ((field != NULL) && (strcasecmp(getFieldInfo()->getName(),field)))
        return NULL;

    TermIterator* pIterator = static_cast<TermIterator*>(new ChunkTermIterator(pTermReaderImpl_->pDirectory_,
                                                             pTermReaderImpl_->barrelName_.c_str(),
                                                             pTermReaderImpl_->pFieldInfo_));
    pIterator->setSkipInterval(skipInterval_);
    pIterator->setMaxSkipLevel(maxSkipLevel_);
    return pIterator;
}

}

NS_IZENELIB_IR_END

