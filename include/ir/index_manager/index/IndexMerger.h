/**
* @file        IndexMerge.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Process the index merging process
*/
#ifndef INDEXMERGER_H
#define INDEXMERGER_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/CollectionInfo.h>
#include <ir/index_manager/utility/BitVector.h>
#include <vector>

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
    inline count_t numDocs()
    {
        return pBarrelInfo->getDocCount();
    }
    /**
    *load barrel info from both memory or disk index files
    */
    void load();
protected:
    Directory* pDirectory;		///index storage

    BarrelInfo* pBarrelInfo;		///barrel information

    CollectionsInfo* pCollectionsInfo;///collections information of barrel

    friend class MergeBarrel;
    friend class IndexMerger;
};

/**
*@brief A priority queue with element type of MergeBarrelEntry
*/
class MergeBarrel : public PriorityQueue<MergeBarrelEntry*>
{
public:
    MergeBarrel(size_t maxSize)
    {
        initialize(maxSize,true);
    }
    MergeBarrel(const string& id,size_t maxSize) : identifier(id)
    {
        initialize(maxSize,true);
    }
    ~MergeBarrel()
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
        return (o1->numDocs()) > (o2->numDocs());
    }
private:
    string identifier;		///identifier of merge barrel
};

/**
*@brief merge index
*/

class IndexMerger
{
public:
    IndexMerger();
    IndexMerger(Directory* pDirectory);
    IndexMerger(Directory* pDirectory,char* buffer,size_t bufsize);
public:
    virtual ~IndexMerger();
public:
    /**
     * merge sub indexes
     * @param pBarrelsInfo barrels of index
     */
    void merge(BarrelsInfo* pBarrelsInfo);

    /**
     * add a index barrel to merge
     * @param pBarrelsInfo the index
     * @param pBarrelInfo info of barrel need to merge
     */
    void addToMerge(BarrelsInfo* pBarrelsInfo,BarrelInfo* pBarrelInfo);

    /**
     * set buffer for merging
     * @param buffer outside buffer for merging
     * @param length size of buffer
     */
    void setBuffer(char* buffer,size_t bufsize);

    /**
     * set directory of index
     * @param pDirectory directory of index
     */
    void setDirectory(Directory* pDirectory)
    {
        this->pDirectory = pDirectory;
    }

    /**
     * get directory of index
     * @return directory of index
     */
    Directory* getDirectory()
    {
        return pDirectory;
    }

    void setDocFilter(BitVector* pFilter)
    {
        pDocFilter = pFilter;
    }

    /**
     * transfer in-memory barrel to disk
     * @param pszBarrelName barrel name
     */
    void transferToDisk(const char* pszBarrelName);

    /**
     * set threshold parameter
     * @param pszThreshold parameter
     */
    void setThreshold(const char* pszThreshold);
protected:
    /**
     * add new index barrel to merge,derived classes implement it,and could apply some merge strategies.
     * @param pEntry the index barrel,derived classes are responsible for deleting
     */
    virtual void addBarrel(MergeBarrelEntry* pEntry) = 0;

    /**
     * the merge is over,derived classes implement it,and could clear some resources for merging.
     */
    virtual void endMerge() = 0;

    /**
     * set parameter of merger
     * @param pszParam parameter string, format:name1=value1;param2=value2
     */
    virtual void setParam(const char* pszParam) {}
protected:
    /**
     * update barrel name and its base document id after merging.
     * @param pBarrelsInfo barrels of index
     */
    virtual void pendingUpdate(BarrelsInfo* pBarrelsInfo);

    /**
     * merge a merge barrel which contains more than one index barrels
     * @param pBarrel merge barrel
     */
    void mergeBarrel(MergeBarrel* pBarrel);

    /**
     * let document ids in a continuous form
     * @param pBarrelsInfo barrels information
     */
    void continueDocIDs(BarrelsInfo* pBarrelsInfo);

protected:
    static bool BarrelGreater(MergeBarrelEntry* o1, MergeBarrelEntry* o2)
    {
        return (o1->numDocs()) > (o2->numDocs());
    }
    /**
     * remove merged barrels from pMergeBarrels
     * @param pBarrel container of barrels
     */
    void removeMergedBarrels(MergeBarrel* pBarrel);
protected:
    Directory* pDirectory;				///index data source

    BarrelsInfo* pBarrelsInfo;			///reference to Index's barrels information

    char* buffer;					///buffer for merging process

    size_t bufsize;					///size of buffer

    bool bBorrowedBuffer;		///is the buffer borrowed from indexer?

    vector<MergeBarrelEntry*>* pMergeBarrels;

    BitVector* pDocFilter;

    friend class IndexWriter;
    friend class Indexer;
};

}

NS_IZENELIB_IR_END

#endif
