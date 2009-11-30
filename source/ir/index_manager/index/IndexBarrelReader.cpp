#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/Indexer.h>

using namespace izenelib::ir::indexmanager;

IndexBarrelReader::IndexBarrelReader(Indexer* pIndexer)
        : pIndexer_(pIndexer)
{
}
IndexBarrelReader::IndexBarrelReader()
        : pIndexer_(NULL)
{
}
IndexBarrelReader::~IndexBarrelReader()
{
    pIndexer_ = NULL;
}

