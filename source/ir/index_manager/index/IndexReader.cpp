#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>


using namespace izenelib::ir::indexmanager;

IndexReader::IndexReader(Indexer* pIndex)
        :pIndexer_(pIndex)
        ,pBarrelsInfo_(NULL)
        ,pBarrelReader_(NULL)
        ,pForwardIndexReader_(NULL)
        ,dirty_(false)
{
    pBarrelsInfo_ = pIndexer_->getBarrelsInfo();
}

IndexReader::~IndexReader(void)
{
    if (pBarrelReader_)
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
    }
    if(pForwardIndexReader_)
        delete pForwardIndexReader_;
}

void IndexReader::createBarrelReader()
{
    if (pBarrelReader_)
        delete pBarrelReader_;
    int32_t bc = pBarrelsInfo_->getBarrelCount();
    BarrelInfo* pLastBarrel = pBarrelsInfo_->getLastBarrel();
    if ( (bc > 0) && pLastBarrel)
    {
        if (pLastBarrel->getDocCount() <= 0)///skip empty barrel
            bc--;
        pLastBarrel = (*pBarrelsInfo_)[bc - 1];
    }

    if (bc == 1)
    {
        if (pLastBarrel && pLastBarrel->getWriter())
            pBarrelReader_ = pLastBarrel->getWriter()->inMemoryReader();
        else
            pBarrelReader_ = new SingleIndexBarrelReader(pIndexer_,pLastBarrel);
    }
    else if (bc > 1)
    {
        pBarrelReader_ = new MultiIndexBarrelReader(pIndexer_,pBarrelsInfo_);
    }
    else
        SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the index barrel number is 0.");
    pIndexer_->setDirty(false);///clear dirty_ flag

}

TermReader* IndexReader::getTermReader(collectionid_t colID)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if (dirty_)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader_ == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader_;
            pBarrelReader_ = NULL;
        }
        dirty_ = false;
    }

    if (pBarrelReader_ == NULL)
    {
        createBarrelReader();
    }
    TermReader* pTermReader = pBarrelReader_->termReader(colID);
    if (pTermReader)
        return pTermReader->clone();
    else
        return NULL;
}

ForwardIndexReader* IndexReader::getForwardIndexReader()
{
    if(!pForwardIndexReader_)
        pForwardIndexReader_ = new ForwardIndexReader(pIndexer_->getDirectory());

    return pForwardIndexReader_->clone();
}

count_t IndexReader::maxDoc()
{
    return pBarrelsInfo_->getDocCount();
}

freq_t IndexReader::docFreq(collectionid_t colID, Term* term)
{
    return getTermReader(colID)->docFreq(term);
}

TermInfo* IndexReader::termInfo(collectionid_t colID,Term* term)
{
    return getTermReader(colID)->termInfo(term);
}

