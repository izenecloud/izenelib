#include <ir/index_manager/index/MultiIndexBarrelReader.h>
#include <ir/index_manager/index/MultiTermReader.h>
#include <ir/index_manager/index/Indexer.h>


using namespace izenelib::ir::indexmanager;

MultiIndexBarrelReader::MultiIndexBarrelReader(Indexer* pIndexer,BarrelsInfo* pBarrelsInfo_)
        :IndexBarrelReader(pIndexer)
        ,pBarrelsInfo(pBarrelsInfo_)
{
    pBarrelsInfo->startIterator();
    BarrelInfo* pBarrelInfo;
    while (pBarrelsInfo->hasNext())
    {
        pBarrelInfo = pBarrelsInfo->next();
        if (pBarrelInfo->getDocCount() > 0)
            addReader(pBarrelInfo);
    }
}

MultiIndexBarrelReader::~MultiIndexBarrelReader(void)
{
    pBarrelsInfo = NULL;
    close();
}

void MultiIndexBarrelReader::open(const char* name)
{
}

TermReader* MultiIndexBarrelReader::termReader(collectionid_t colID)
{
    if (termReaderMap.find(colID) == termReaderMap.end())
    {
        try
        {
            boost::shared_ptr<MultiTermReader > pTermReader(new MultiTermReader(this, colID));
            termReaderMap[colID] = pTermReader;
        }
        catch (std::bad_alloc& ba)
        {
            string serror = ba.what();
            SF1V5_THROW(ERROR_OUTOFMEM,"MultiIndexBarrelReader::termReader():" + serror);
        }
    }
    return termReaderMap[colID].get();
}

void MultiIndexBarrelReader::close()
{
    vector<BarrelReaderEntry*>::iterator iter = readers.begin();
    while (iter != readers.end())
    {
        delete (*iter);
        iter++;
    }
    readers.clear();

}

void MultiIndexBarrelReader::addReader(BarrelInfo* pBarrelInfo)
{
    readers.push_back(new BarrelReaderEntry(pIndexer,pBarrelInfo));
}

