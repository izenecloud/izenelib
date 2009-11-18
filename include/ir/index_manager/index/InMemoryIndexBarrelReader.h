/**
* @file        InMemoryIndexBarrelReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   Index barrel reader in memory
*/

#ifndef INMEMORYINDEXBARRELREADER_H
#define INMEMORYINDEXBARRELREADER_H

#include <ir/index_manager/index/IndexBarrelReader.h>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class IndexBarrelWriter;
///Index barrel reader in memory
class InMemoryIndexBarrelReader : public IndexBarrelReader
{
public:
    InMemoryIndexBarrelReader(IndexBarrelWriter* pIndexBarrelWriter);
    virtual ~InMemoryIndexBarrelReader(void);
public:
    TermReader* termReader(collectionid_t colID);

    TermReader* termReader(collectionid_t colID,const char* field);

    void deleteDocumentPhysically(IndexerDocument* pDoc);

    size_t getDistinctNumTerms(collectionid_t colID, const std::string& property);

    void close();

    void reopen(){}

protected:
    IndexBarrelWriter* pIndexBarrelWriter;
    map<collectionid_t, TermReader*> termReaderMap;
};

}

NS_IZENELIB_IR_END

#endif

