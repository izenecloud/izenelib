#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/MultiIndexBarrelReader.h>


using namespace izenelib::ir::indexmanager;

IndexReader::IndexReader(Indexer* pIndex)
        :pIndexer(pIndex)
        ,pBarrelsInfo(NULL)
        ,pBarrelReader(NULL)
        ,dirty(false)
{
    pBarrelsInfo = pIndexer->getBarrelsInfo();
}

IndexReader::~IndexReader(void)
{
    if (pBarrelReader)
    {
        delete pBarrelReader;
        pBarrelReader = NULL;
    }
}

void IndexReader::createBarrelReader()
{
    if (pBarrelReader)
        delete pBarrelReader;
    int32_t bc = pBarrelsInfo->getBarrelCount();
    BarrelInfo* pLastBarrel = pBarrelsInfo->getLastBarrel();
    if ( (bc > 0) && pLastBarrel)
    {
        if (pLastBarrel->getDocCount() <= 0)///skip empty barrel
            bc--;
        pLastBarrel = (*pBarrelsInfo)[bc - 1];
    }

    if (bc == 1)
    {
        if (pLastBarrel && pLastBarrel->getWriter())
            pBarrelReader = pLastBarrel->getWriter()->inMemoryReader();
        else
            pBarrelReader = new SingleIndexBarrelReader(pIndexer,pLastBarrel);
    }
    else if (bc > 1)
    {
        pBarrelReader = new MultiIndexBarrelReader(pIndexer,pBarrelsInfo);
    }
    else
        SF1V5_THROW(ERROR_INDEX_COLLAPSE,"the index barrel number is 0.");
    pIndexer->setDirty(false);///clear dirty flag

}

TermReader* IndexReader::getTermReader(collectionid_t colID)
{
    boost::mutex::scoped_lock lock(this->mutex_);

    if (dirty)
    {
        //collection has been removed, need to rebuild the barrel reader
        if (pBarrelReader == NULL)
        {
            createBarrelReader();
        }
        else
        {
            delete pBarrelReader;
            pBarrelReader = NULL;
        }
        dirty = false;
    }

    if (pBarrelReader == NULL)
    {
        createBarrelReader();
    }
    TermReader* pTermReader = pBarrelReader->termReader(colID);
    if (pTermReader)
        return pTermReader->clone();
    else
        return NULL;
}

count_t IndexReader::maxDoc()
{
    return pBarrelsInfo->getDocCount();
}

freq_t IndexReader::docFreq(collectionid_t colID, Term* term)
{
    return getTermReader(colID)->docFreq(term);
}

TermInfo* IndexReader::termInfo(collectionid_t colID,Term* term)
{
    return getTermReader(colID)->termInfo(term);
}

