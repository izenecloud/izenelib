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
#include <ir/index_manager/index/ForwardIndexReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class Term;

class IndexReader
{
private:
    ///ORDERED can not work for in-memory index, because current in-memory index is based on hash
    ///and it is expensive to convert the in-memory hash to an order-preserving map
    IndexReader(Indexer* pIndex, DiskIndexOpenMode openMode= UNORDERED);
public:
    virtual ~IndexReader();
public:
    count_t numDocs();

    // count_t maxDoc();

    freq_t docFreq(collectionid_t colID, Term* term);

    TermInfo* termInfo(collectionid_t colID, Term* term);

    BarrelsInfo* getBarrelsInfo();

    void setDirty(bool dirty)
    {
        dirty_ = dirty;
    }

    static int64_t lastModified(Directory* pDirectory);

    /**
     * @warn client must delete the returned object
     */
    TermReader* getTermReader(collectionid_t colID);

    /**
     * @warn client must delete the returned object
     */
    ForwardIndexReader* getForwardIndexReader();

private:
    void createBarrelReader();
    TermReader* doGetTermReader_(collectionid_t colID);

private:
    DiskIndexOpenMode openMode_; ///whether accessing vocabulary ordered preserving or not.(hash is unordered and is faster)

    Indexer* pIndexer_; ///reference to index object

    BarrelsInfo* pBarrelsInfo_; ///reference to Index's pBarrelsInfo

    IndexBarrelReader* pBarrelReader_; ///barrel reader

    ForwardIndexReader* pForwardIndexReader_;

    bool dirty_;

    mutable boost::mutex mutex_;

    friend class Indexer;
};
inline BarrelsInfo* IndexReader::getBarrelsInfo()
{
    return pBarrelsInfo_;
}

}

NS_IZENELIB_IR_END

#endif
