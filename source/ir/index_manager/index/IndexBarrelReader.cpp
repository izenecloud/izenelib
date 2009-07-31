#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/Indexer.h>

using namespace izenelib::ir::indexmanager;

IndexBarrelReader::IndexBarrelReader(Indexer* pIndexer_)
        : pIndexer(pIndexer_)
{
}
IndexBarrelReader::IndexBarrelReader()
        : pIndexer(NULL)
{
}
IndexBarrelReader::~IndexBarrelReader(void)
{
    pIndexer = NULL;
}

