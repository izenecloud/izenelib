#include <ir/index_manager/index/TermIterator.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/TermReader.h>

#include <sstream>

using namespace std;

using namespace izenelib::ir::indexmanager;

TermIterator::TermIterator()
{}

TermIterator::~TermIterator()
{}

VocIterator::VocIterator(VocReader* termReader)
        :pTermReader_(termReader)
        ,pCurTerm_(NULL)
        ,pCurTermInfo_(NULL)
        ,pCurTermPosting_(NULL)
        ,pInputDescriptor_(NULL)
        ,nCurPos_(-1)
{
}

VocIterator::~VocIterator(void)
{
    if (pCurTerm_)
    {
        delete pCurTerm_;
        pCurTerm_ = NULL;
    }
    if (pCurTermPosting_)
    {
        delete pCurTermPosting_;
        pCurTermPosting_ = NULL;
    }
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
    }
    pCurTermInfo_ = NULL;
}

const Term* VocIterator::term()
{
    return pCurTerm_;
}

const TermInfo* VocIterator::termInfo()
{
    return pCurTermInfo_;
}

Posting* VocIterator::termPosting()
{
    if (!pCurTermPosting_)
    {
        IndexInput* pInput;
        pInputDescriptor_ = new InputDescriptor(true);
        pInput = pTermReader_->getTermReaderImpl()->pInputDescriptor_->getDPostingInput()->clone();
        pInputDescriptor_->setDPostingInput(pInput);
        pInput = pTermReader_->getTermReaderImpl()->pInputDescriptor_->getPPostingInput()->clone();
        pInputDescriptor_->setPPostingInput(pInput);
        pCurTermPosting_ = new OnDiskPosting(pInputDescriptor_,*pCurTermInfo_);
    }
    else
    {
        ((OnDiskPosting*)pCurTermPosting_)->reset(*pCurTermInfo_);///reset to a new posting
    }
    return pCurTermPosting_;
}

bool VocIterator::next()
{
    if(pTermReader_->getTermReaderImpl()->nTermCount_ > nCurPos_+1)			
    {
        if(pCurTerm_ == NULL)
            pCurTerm_ = new Term(pTermReader_->getFieldInfo()->getName(),pTermReader_->getTermReaderImpl()->pTermTable_[++nCurPos_].tid);
        else 
            pCurTerm_->setValue(pTermReader_->getTermReaderImpl()->pTermTable_[++nCurPos_].tid);
        pCurTermInfo_ = &(pTermReader_->getTermReaderImpl()->pTermTable_[nCurPos_].ti);
        return true;
    }
    else return false;
}

//////////////////////////////////////////////////////////////////////////
///DiskTermIterator
DiskTermIterator::DiskTermIterator(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo)
    :pDirectory_(pDirectory)
    ,pFieldInfo_(pFieldInfo)
    ,pCurTerm_(NULL)
    ,pCurTermInfo_(NULL)
    ,pCurTermPosting_(NULL)
    ,pInputDescriptor_(NULL)
    ,nCurPos_(-1)
{
    barrelName_ = barrelname;
    pVocInput_ = pDirectory->openInput(barrelName_ + ".voc");
    pVocInput_->seek(pFieldInfo->getIndexOffset());
    fileoffset_t voffset = pVocInput_->getFilePointer();
    ///begin read vocabulary descriptor
    nVocLength_ = pVocInput_->readLong();
    nTermCount_ = (int32_t)pVocInput_->readLong(); ///get total term count
    ///end read vocabulary descriptor
    pVocInput_->seek(voffset - nVocLength_);///seek to begin of vocabulary data

}

DiskTermIterator::~DiskTermIterator()
{
    delete pVocInput_;
    if (pCurTerm_)
    {
        delete pCurTerm_;
        pCurTerm_ = NULL;
    }
    if (pCurTermPosting_)
    {
        delete pCurTermPosting_;
        pCurTermPosting_ = NULL;
    }
    if (pInputDescriptor_)
    {
        delete pInputDescriptor_;
        pInputDescriptor_ = NULL;
    }
    if(pCurTermInfo_)
    {
        delete pCurTermInfo_;
        pCurTermInfo_ = NULL;
    }
}

const Term* DiskTermIterator::term()
{
    return pCurTerm_;
}

const TermInfo* DiskTermIterator::termInfo()
{
    return pCurTermInfo_;
}

Posting* DiskTermIterator::termPosting()
{
    if (!pCurTermPosting_)
    {
        pInputDescriptor_ = new InputDescriptor(true);
        pInputDescriptor_->setDPostingInput(pDirectory_->openInput(barrelName_ + ".dfp"));
        pInputDescriptor_->setPPostingInput(pDirectory_->openInput(barrelName_ + ".pop"));
        pCurTermPosting_ = new OnDiskPosting(pInputDescriptor_,*pCurTermInfo_);
    }
    else
    {
        ((OnDiskPosting*)pCurTermPosting_)->reset(*pCurTermInfo_);///reset to a new posting
    }
    return pCurTermPosting_;
}

bool DiskTermIterator::next()
{
    if(nTermCount_ > nCurPos_+1) 		
    {
        ++nCurPos_;
        termid_t tid = pVocInput_->readInt();
        freq_t df = pVocInput_->readInt();
        freq_t ctf = pVocInput_->readInt();
        docid_t lastdoc = pVocInput_->readInt();
        freq_t skipLevel = pVocInput_->readInt();
        fileoffset_t skipPointer = pVocInput_->readLong();
        fileoffset_t docPointer = pVocInput_->readLong();
        freq_t docPostingLen = pVocInput_->readInt();
        fileoffset_t positionPointer = pVocInput_->readLong();
        freq_t positionPostingLen = pVocInput_->readInt();	

        if(pCurTerm_ == NULL)
            pCurTerm_ = new Term(pFieldInfo_->getName(),tid);
        else 
            pCurTerm_->setValue(tid);
        if(pCurTermInfo_ == NULL)
            pCurTermInfo_ = new TermInfo();
        pCurTermInfo_->set(df,ctf,lastdoc,skipLevel,skipPointer,docPointer,docPostingLen,positionPointer,positionPostingLen);;
        return true;
    }
    else return false;
}


//////////////////////////////////////////////////////////////////////////
//
InMemoryTermIterator::InMemoryTermIterator(InMemoryTermReader* pTermReader)
        :pTermReader_(pTermReader)
        ,pCurTerm_(NULL)
        ,pCurTermInfo_(NULL)
        ,pCurTermPosting_(NULL)
        ,postingIterator_(pTermReader->pIndexer_->postingMap_.begin())
        ,postingIteratorEnd_(pTermReader->pIndexer_->postingMap_.end())
{
}

InMemoryTermIterator::~InMemoryTermIterator(void)
{
    if (pCurTerm_)
    {
        delete pCurTerm_;
        pCurTerm_ = NULL;
    }
    pCurTermPosting_ = NULL;
    if (pCurTermInfo_)
    {
        delete pCurTermInfo_;
        pCurTermInfo_ = NULL;
    }
}

bool InMemoryTermIterator::next()
{
    postingIterator_++;

    if(postingIterator_ != postingIteratorEnd_)
    {
        pCurTermPosting_ = postingIterator_->second;
        while (pCurTermPosting_->hasNoChunk())
        {
            postingIterator_++;
            if(postingIterator_ != postingIteratorEnd_)
                pCurTermPosting_ = postingIterator_->second;
            else return false;
        }

        //TODO
        if (pCurTerm_ == NULL)
            pCurTerm_ = new Term(pTermReader_->field_.c_str(),postingIterator_->first);
        else pCurTerm_->setValue(postingIterator_->first);
        pCurTermPosting_ = postingIterator_->second;
        if (pCurTermInfo_ == NULL)
            pCurTermInfo_ = new TermInfo();
        pCurTermInfo_->set(
                                pCurTermPosting_->docFreq(),
                                pCurTermPosting_->getCTF(),
                                pCurTermPosting_->lastDocID(),
                                pCurTermPosting_->getSkipLevel(),
                                -1,-1,0,-1,0);
        return true;
    }
    else return false;
}

const Term* InMemoryTermIterator::term()
{
    return pCurTerm_;
}
const TermInfo* InMemoryTermIterator::termInfo()
{
    return pCurTermInfo_;
}
Posting* InMemoryTermIterator::termPosting()
{
    return pCurTermPosting_;
}

