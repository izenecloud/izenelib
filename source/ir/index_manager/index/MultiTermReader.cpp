#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/MultiTermDocs.h>
#include <ir/index_manager/index/MultiTermPositions.h>
#include <ir/index_manager/index/MultiTermIterator.h>

#include <memory> // unique_ptr 

using namespace izenelib::ir::indexmanager;

MultiTermReader::MultiTermReader(MultiIndexBarrelReader* pBarrelReader, collectionid_t colID)
    :colID_(colID)
    ,isOwnTermReaders_(false)
{
    for(vector<BarrelReaderEntry*>::iterator iter = pBarrelReader->readers_.begin();
            iter != pBarrelReader->readers_.end(); ++iter)
    {
        if(TermReader* pTermReader = (*iter)->pBarrelReader_->termReader(colID_))
            termReaders_.push_back(std::make_pair((*iter)->pBarrelInfo_, pTermReader));
    }
}

MultiTermReader::MultiTermReader(const MultiTermReader& multiTermReader)
    :colID_(multiTermReader.colID_)
    ,isOwnTermReaders_(true)
{
    try
    {
        for(vector<BarrelTermReaderEntry>::const_iterator iter = multiTermReader.termReaders_.begin();
                iter != multiTermReader.termReaders_.end(); ++iter)
        {
            termReaders_.push_back(std::make_pair(iter->first, iter->second->clone()));
        }
    }
    catch(...)
    {
        // destroy the resource before rethrow the exception
        close();
        throw;
    }
}

MultiTermReader::~MultiTermReader(void)
{
    close();
}

void MultiTermReader::open(Directory* pDirectory,BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo)
{
}

void MultiTermReader::reopen()
{
    for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
        iter->second->reopen();
}

TermIterator* MultiTermReader::termIterator(const char* field)
{
    MultiTermIterator* pTermIterator = new MultiTermIterator();
    for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
    {
        if(TermIterator* pIt = iter->second->termIterator(field))
            pTermIterator->addIterator(pIt);
    }
    return pTermIterator;
}

bool MultiTermReader::seek(Term* term)
{
    bool bSuc = false;

    for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
        bSuc = iter->second->seek(term) || bSuc;

    return bSuc;
}

TermDocFreqs* MultiTermReader::termDocFreqs()
{
    // use unique_ptr in case of memory leak when exception is thrown in below TermReader::termDocFreqs()
    std::unique_ptr<MultiTermDocs> termDocsPtr(new MultiTermDocs());
    bool bAdd = false;
    try{
    vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
    for(;iter != termReaders_.end(); ++iter)
    {
        if(TermDocFreqs* pTmpTermDocs = iter->second->termDocFreqs())
        {
            termDocsPtr->add(iter->first, pTmpTermDocs);
            bAdd = true;
        }
    }

    if (bAdd == false)
        return NULL;

    return termDocsPtr.release();
    }catch(std::exception& e)
    {
        return NULL;
    }
}

TermPositions* MultiTermReader::termPositions()
{
    // use unique_ptr in case of memory leak when exception is thrown in below TermReader::termPositions()
    std::unique_ptr<MultiTermPositions> termPositionsPtr(new MultiTermPositions());
    bool bAdd = false;
    try{
    vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
    for(;iter != termReaders_.end(); ++iter)
    {
        if(TermPositions* pTmpTermPositions = iter->second->termPositions())
        {
            termPositionsPtr->add(iter->first, pTmpTermPositions);
            bAdd = true;
        }
    }

    if (bAdd == false)
        return NULL;

    return termPositionsPtr.release();
    }catch(std::exception& e){
        return NULL;
    }
}

freq_t MultiTermReader::docFreq(Term* term)
{
    freq_t df = 0;

    for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
        df += iter->second->docFreq(term);

    return df;
}

TermInfo* MultiTermReader::termInfo(Term* term)
{
    termInfo_.reset();

    for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
    {
        if(const TermInfo* pTermInfo = iter->second->termInfo(term))
        {
            termInfo_.docFreq_ += pTermInfo->docFreq_;
            termInfo_.ctf_ += pTermInfo->ctf_;
            if(termInfo_.lastDocID_ == BAD_DOCID || termInfo_.lastDocID_ < pTermInfo->lastDocID_)
                termInfo_.lastDocID_ = pTermInfo->lastDocID_;
        }
    }

    if(termInfo_.docFreq() > 0)
        return &termInfo_;
    return NULL;
}

void MultiTermReader::setSkipInterval(int skipInterval)
{
    skipInterval_ = skipInterval;
    std::vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
    for(;iter != termReaders_.end(); ++iter)
        iter->second->setSkipInterval(skipInterval);
}

void MultiTermReader::setMaxSkipLevel(int maxSkipLevel)
{
    maxSkipLevel_ = maxSkipLevel;
    std::vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
    for(;iter != termReaders_.end(); ++iter)
        iter->second->setMaxSkipLevel(maxSkipLevel);
}

void MultiTermReader::setDocFilter(Bitset* pFilter)
{
    std::vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
    for(;iter != termReaders_.end(); ++iter)
        iter->second->setDocFilter(pFilter);
}

void MultiTermReader::close()
{
    if(isOwnTermReaders_)
    {
        for(vector<BarrelTermReaderEntry>::iterator iter = termReaders_.begin();
            iter != termReaders_.end(); ++iter)
            delete iter->second;
    }
}

TermReader* MultiTermReader::clone()
{
    return new MultiTermReader(*this);
}
