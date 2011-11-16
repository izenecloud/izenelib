/**
* @file        MultiIndexBarrelReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Multi Index Barrel Reader
*/

#ifndef MULTIINDEXBARRELREADER_H
#define MULTIINDEXBARRELREADER_H

#include <ir/index_manager/index/IndexBarrelWriter.h>
#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/SingleIndexBarrelReader.h>
#include <ir/index_manager/store/Directory.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* @brief The helper class to make pair between BarrelInfo and SingleIndexBarrelReader, 
* this pair will be stored inside the MultiIndexBarrelReader
*/
class BarrelReaderEntry
{
public:
    BarrelReaderEntry(IndexReader* pIndexReader,BarrelInfo* pBarrelInfo)
    {
        pBarrelInfo_ = pBarrelInfo;
        if (pBarrelInfo->getWriter())
            pBarrelReader_ = pBarrelInfo->getWriter()->inMemoryReader();
        else
            pBarrelReader_ = new SingleIndexBarrelReader(pIndexReader,pBarrelInfo);
    }

    ~BarrelReaderEntry()
    {
        delete pBarrelReader_;
        pBarrelReader_ = NULL;
        pBarrelInfo_ = NULL;
    }

public:
    BarrelInfo* pBarrelInfo_;
    IndexBarrelReader* pBarrelReader_;

    friend class MultiIndexBarrelReader;
};

/**
* @brief Open multi index barrel and return instance of TermReader, 
* each index barrel is opened and read by SingleIndexBarrelReader
*/
class TermReader;
class MultiIndexBarrelReader : public IndexBarrelReader
{
public:
    MultiIndexBarrelReader(
        IndexReader* pIndexReader,
        BarrelsInfo* pBarrelsInfo);

    virtual ~MultiIndexBarrelReader();
public:
    void open(const char* name);

    TermReader* termReader(collectionid_t colID);

    size_t getDistinctNumTerms(collectionid_t colID, const std::string& property);

    void close();

    bool hasMemBarrel() { return hasMemBarrel_;}

private:
    BarrelsInfo* pBarrelsInfo_;

    map<collectionid_t, TermReader*> termReaderMap_;

    vector<BarrelReaderEntry*> readers_;

    bool hasMemBarrel_;

    friend class MultiTermReader;
};

}

NS_IZENELIB_IR_END

#endif

