#include <ir/index_manager/index/DocIdMap.h>

using namespace izenelib::ir::indexmanager;

DocIdMap::DocIdMap(Directory* pDirectory)
    :idMap_(0)
{
}

DocIdMap::~DocIdMap()
{
}

void add(docid_t docId, docid_t val)
{
}

docid_t getDocId(docid_t docId)
{
    return 0;
}

void load()
{
}

void flush()
{
}
