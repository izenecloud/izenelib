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
    virtual ~InMemoryIndexBarrelReader();
public:
    TermReader* termReader(collectionid_t colID);

    TermReader* termReader(collectionid_t colID,const char* field);

    size_t getDistinctNumTerms(collectionid_t colID, const std::string& property);

    void close();

    void reopen(){}

    bool hasMemBarrel() {return true;};

protected:
    IndexBarrelWriter* pIndexBarrelWriter_;
    map<collectionid_t, TermReader*> termReaderMap_;
};

}

NS_IZENELIB_IR_END

#endif

