/**
* @file        MultiIndexBarrelReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Multi Index Barrel Reader
*/

#ifndef MULTIINDEXBARRELREADER_H
#define MULTIINDEXBARRELREADER_H

#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/SingleIndexBarrelReader.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/IndexBarrelWriter.h>

#include <boost/shared_ptr.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
* @brief The helper class to make pair between BarrelInfo and SingleIndexBarrelReader, this pair will be stored inside the
* MultiIndexBarrelReader
*/
class BarrelReaderEntry
{
public:
    BarrelReaderEntry(Indexer* pIndexer,BarrelInfo* pBarrelInfo_,DiskIndexOpenMode mode)
    {
        pBarrelInfo = pBarrelInfo_;
        if (pBarrelInfo->getWriter())
            pBarrel = pBarrelInfo->getWriter()->inMemoryReader();
        else
            pBarrel = new SingleIndexBarrelReader(pIndexer,pBarrelInfo,mode);
    }

    ~BarrelReaderEntry()
    {
        delete pBarrel;
        pBarrel = NULL;
        pBarrelInfo = NULL;
    }

public:
    BarrelInfo* pBarrelInfo;
    IndexBarrelReader* pBarrel;

    friend class MultiIndexBarrelReader;
};

/**
* @brief Open multi index barrel and return instance of TermReader, each index barrel is opened and read by SingleIndexBarrelReader
*/
class MultiTermReader;
class MultiIndexBarrelReader : public IndexBarrelReader
{
public:
    MultiIndexBarrelReader(Indexer* pIndex,BarrelsInfo* pBarrelsInfo,DiskIndexOpenMode mode);

    virtual ~MultiIndexBarrelReader(void);
public:
    void open(const char* name);

    TermReader* termReader(collectionid_t colID);

    void close();

public:
    void startIterator()
    {
        readersIterator = readers.begin();
    }

    bool hasNext()
    {
        return (readersIterator != readers.end());
    }

    BarrelReaderEntry* nextEntry();

    IndexBarrelReader* nextReader();

    BarrelInfo* nextBarrel();

protected:

    void addReader(BarrelInfo* pBarrelInfo,DiskIndexOpenMode mode);
private:
    BarrelsInfo* pBarrelsInfo;

    map<collectionid_t, boost::shared_ptr<MultiTermReader > > termReaderMap;

    vector<BarrelReaderEntry*> readers;

    vector<BarrelReaderEntry*>::iterator readersIterator;
};
//////////////////////////////////////////////////////////////////////////
//Inline Functions
inline BarrelReaderEntry* MultiIndexBarrelReader::nextEntry()
{
    return *readersIterator++;
}
inline IndexBarrelReader* MultiIndexBarrelReader::nextReader()
{
    IndexBarrelReader* pReader = (*readersIterator)->pBarrel;
    readersIterator++;
    return pReader;
}
inline BarrelInfo* MultiIndexBarrelReader::nextBarrel()
{
    BarrelInfo* pBarrelInfo = (*readersIterator)->pBarrelInfo;
    readersIterator++;
    return pBarrelInfo;
}


}

NS_IZENELIB_IR_END

#endif

