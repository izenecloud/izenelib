/**
* @file        IndexMerger.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Process the index merging process
*
*/
#ifndef INDEXMERGER_H
#define INDEXMERGER_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/CollectionInfo.h>
#include <ir/index_manager/utility/Bitset.h>

#include <string>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
*@brief barrel to be merged
*/
class MergeBarrelEntry
{
public:
    MergeBarrelEntry(Directory* pDirectory,BarrelInfo* pBarrelInfo);

    ~MergeBarrelEntry();

public:
    inline count_t numDocs() { return pBarrelInfo_->getDocCount();}
    const std::string& barrelName() { return pBarrelInfo_->getName(); }
    ///load barrel info from both memory or disk index files
    void load();

    void setCurrColID(collectionid_t colID) { currColID_ = (int)colID;}

    docid_t baseDocID() { return currColID_ < 0 ? 0 : pBarrelInfo_->baseDocIDMap[currColID_];}

protected:
    Directory* pDirectory_;		///index storage

    BarrelInfo* pBarrelInfo_;		///barrel information

    CollectionsInfo* pCollectionsInfo_;///collections information of barrel

    int currColID_;

    friend class MergeBarrelQueue;
    friend class IndexMerger;
};

/**
*@brief A priority queue with element type of MergeBarrelEntry
*/
class MergeBarrelQueue : public PriorityQueue<MergeBarrelEntry*>
{
public:
    MergeBarrelQueue(size_t maxSize)
    {
        initialize(maxSize,true);
    }
    MergeBarrelQueue(const string& id,size_t maxSize) : identifier(id)
    {
        initialize(maxSize,true);
    }
    ~MergeBarrelQueue()
    {
    }
public:
    /** get/set identifier */
    void setIdentifier(const string& id)
    {
        identifier = id;
    }

    const string& getIdentifier()
    {
        return identifier;
    }

    void load()
    {
        size_t nEntryCount = size();
        for (size_t nEntry = 0;nEntry < nEntryCount;nEntry++)
        {
            getAt(nEntry)->load();
        }
    }
private:
    bool lessThan(MergeBarrelEntry* o1, MergeBarrelEntry* o2)
    {
        return o1->numDocs() > o2->numDocs();
    }
private:
    string identifier;		///identifier of merge barrel
};

/**
*@brief merge index
*/
class Indexer;
class IndexMergePolicy;

class IndexMerger
{
public:
    /**
     * Constructor.
     * @param pIndexer Indexer instance
     * @param pMergePolicy IndexMergePolicy instance
     * @note IndexMerger is responsible to delete @p pMergePolicy
     */
    IndexMerger(Indexer* pIndexer, IndexMergePolicy* pMergePolicy);

    virtual ~IndexMerger();

    /**
     * merge all barrels into one barrel.
     */
    void mergeBarrels();

    /**
     * add a index barrel to merge
     * @param pBarrelInfo info of barrel to merge
     */
    void addToMerge(BarrelInfo* pBarrelInfo);

    /**
     * merge a merge barrel which contains more than one index barrels
     * @param pBarrelQueue merge barrel queue
     */
    virtual void mergeBarrel(MergeBarrelQueue* pBarrelQueue);

    /**
     * set directory of index
     * @param pDirectory directory of index
     */
    void setDirectory(Directory* pDirectory)
    {
        this->pDirectory_ = pDirectory;
    }

    /**
     * get directory of index
     * @return directory of index
     */
    Directory* getDirectory()
    {
        return pDirectory_;
    }

    void setDocFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

    void setOptimize(bool optimize)
    {
        optimize_ = optimize;
    }

protected:
    /**
     * set parameter of merger
     * @param pszParam parameter string, format:name1=value1;param2=value2
     */
    virtual void setParam(const char* pszParam) {}

    /**
     * update all barrel names according to their base document ids.
     */
    void updateBarrels();

    /**
     * output new barrel contents merged from @p pBarrelQueue.
     * @param pBarrelQueue the barrels to merge
     * @param newBarrelName new barrel name
     */
    void outputNewBarrel(MergeBarrelQueue* pBarrelQueue, const string& newBarrelName);

    /**
     * Remove merged barrels and create new barrel.
     * @param pBarrelQueue the merged barrels
     * @param newBarrelName new barrel name
     * @return new barrel instance
     * @note this function is in the lock scope of @c IndexMergeManager::pauseMergeMutex_ and @c Indexer::mutex_.
     */
    BarrelInfo* createNewBarrelInfo(MergeBarrelQueue* pBarrelQueue, const string& newBarrelName);

protected:
    Indexer* pIndexer_;

    IndexMergePolicy* pMergePolicy_;

    Directory* pDirectory_;				///index data source

    BarrelsInfo* pBarrelsInfo_;			///reference to Index's barrels information

    Bitset* pDocFilter_;

    bool triggerMerge_;

    bool optimize_;  /// whether optimize BYTEALIGN index into BLOCK or CHUNK index

    friend class IndexWriter;
    friend class Indexer;
};

}

NS_IZENELIB_IR_END

#endif
