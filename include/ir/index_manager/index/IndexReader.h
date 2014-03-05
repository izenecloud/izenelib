/**
* @file        IndexReader.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Index Reader
*/
#ifndef INDEXREADER_H
#define INDEXREADER_H

#include <boost/thread.hpp>

#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/IndexBarrelReader.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/CollectionInfo.h>
#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/index/DocLengthReader.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/Bitset.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class Term;

class IndexReader
{
private:
    IndexReader(Indexer* pIndex);
public:
    virtual ~IndexReader();
public:
    count_t numDocs();

    docid_t maxDoc();

    size_t docLength(docid_t docId, fieldid_t fid);

    double getAveragePropertyLength(fieldid_t fid);

    freq_t docFreq(collectionid_t colID, Term* term);

    TermInfo* termInfo(collectionid_t colID, Term* term);

    BarrelsInfo* getBarrelsInfo();

    /**
     * Get the TermReader instance for query.
     * @param colID collection id
     * @return the newly created TermReader instance
     * @note client must delete the instance returned
     */
    TermReader* getTermReader(collectionid_t colID);

    /// mark a document as deleted;
    void delDocument(collectionid_t colID, docid_t docId);

    size_t getDistinctNumTerms(collectionid_t colID, const std::string& property);

    Bitset* getDocFilter() { return pDocFilter_; }

    void delDocFilter();

    void reopen();

    izenelib::util::ReadWriteLock& getGlobalLock() { return pIndexer_->mutex_; }

    boost::mutex& getDocFilterMutex() { return docFilterMutex_; }

    bool isDirty() {return pIndexer_->isDirty(); }

    /**
     * If the Bitset recording which docs are removed exists and any bit is set,
     * write it to disk.
     */
    void flush();

    IndexLevel getIndexLevel() { return pIndexer_->pConfigurationManager_->indexStrategy_.indexLevel_; }

    bool hasMemBarrelReader();

private:
    void createBarrelReader();

private:
    Indexer* pIndexer_; ///reference to index object

    BarrelsInfo* pBarrelsInfo_; ///reference to Index's pBarrelsInfo

    IndexBarrelReader* pBarrelReader_; ///barrel reader

    mutable boost::mutex mutex_;

    Bitset* pDocFilter_;

    boost::mutex docFilterMutex_; /// mutex for pDocFilter_

    DocLengthReader* pDocLengthReader_;

    friend class Indexer;
    friend class SingleIndexBarrelReader;
};
inline BarrelsInfo* IndexReader::getBarrelsInfo()
{
    return pBarrelsInfo_;
}

}

NS_IZENELIB_IR_END

#endif
