/**
* @file        IndexReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index Reader
*/
#ifndef INDEXREADER_H
#define INDEXREADER_H

#include <boost/thread.hpp>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/CollectionInfo.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class Term;
/**
*IndexReader
*/
class IndexReader
{
private:
    IndexReader(Indexer* pIndex);
public:
    virtual ~IndexReader();
public:
    count_t numDocs();

    count_t maxDoc();

    freq_t docFreq(collectionid_t colID, Term* term);

    TermInfo* termInfo(collectionid_t colID, Term* term);

    BarrelsInfo* getBarrelsInfo();

    void setDirty(bool bDirty)
    {
        dirty = bDirty;
    }

    static int64_t lastModified(Directory* pDirectory);
    /**
     * get internal term reader
     * @return term reader, internal object
     */
    TermReader* getTermReader(collectionid_t colID);
private:
    /**
     * create barrel reader
     */
    void createBarrelReader();

private:

    Indexer* pIndexer; ///reference to index object

    BarrelsInfo* pBarrelsInfo; ///reference to Index's pBarrelsInfo

    IndexBarrelReader* pBarrelReader; ///barrel reader

    bool dirty;

    mutable boost::mutex mutex_;

    friend class Indexer;
};
inline BarrelsInfo* IndexReader::getBarrelsInfo()
{
    return pBarrelsInfo;
}

}

NS_IZENELIB_IR_END

#endif
