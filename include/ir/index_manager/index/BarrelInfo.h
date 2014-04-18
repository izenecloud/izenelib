/**
* @file        BarrelInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief barrels information, description of all barrels in the index database
*/

#ifndef BARRELINFO_H
#define BARRELINFO_H

#include <ir/index_manager/utility/IndexManagerConfig.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <util/izene_log.h>

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <cassert>

#define BARRELS_INFONAME "barrels"

NS_IZENELIB_IR_BEGIN
namespace indexmanager{
class IndexBarrelWriter;
///barrel information, description of index barrel
enum CompressionType
{
    BYTEALIGN,  /// byte-aligned compression, vint + d-gap compression
    BLOCK, /// block based compression
    CHUNK /// chunk based compression
};

class BarrelInfo
{
public:
    BarrelInfo(IndexLevel indexLevel)
            : nNumDocs(0)
            , pBarrelWriter(NULL)
            , isUpdate(false)
            , inMemoryBarrel(false)
            , maxDocId(0)
            , searchable(true)
            , indexLevel_(indexLevel)
            , compressType(BYTEALIGN)
            , isRemoved_(false)
    {
    }

    BarrelInfo(const string& name,count_t count,IndexLevel indexLevel,CompressionType compresstype = BYTEALIGN)
            : barrelName(name)
            , nNumDocs(count)
            , pBarrelWriter(NULL)
            , isUpdate(false)
            , inMemoryBarrel(false)
            , maxDocId(0)
            , searchable(true)
            , indexLevel_(indexLevel)
            , compressType(compresstype)
            , isRemoved_(false)
    {
    }


    BarrelInfo(BarrelInfo* pBarrelInfo)
            : barrelName(pBarrelInfo->barrelName)
            , baseDocIDMap(pBarrelInfo->baseDocIDMap)    
            , nNumDocs(pBarrelInfo->nNumDocs)
            , pBarrelWriter(NULL)
            , isUpdate(pBarrelInfo->isUpdate)
            , inMemoryBarrel(pBarrelInfo->inMemoryBarrel)
            , maxDocId(pBarrelInfo->maxDocId)
            , searchable(pBarrelInfo->searchable)
            , indexLevel_(pBarrelInfo->indexLevel_)
            , compressType(pBarrelInfo->compressType)
            , isRemoved_(pBarrelInfo->isRemoved_)
    {
    }


    ~BarrelInfo()
    {
        pBarrelWriter = NULL;
    }
public:
    /**
    * return the barrel name
    */
    const string& getName()
    {
        return barrelName;
    }
    /**
    * set the barrel name
    */
    void setName(const string& name)
    {
        barrelName = name;
    }
    /**
    * return the document count of this barrel
    */
    count_t getDocCount()
    {
        return nNumDocs;
    }
    /**
    * set the document count the barrel
    */
    void setDocCount(count_t count)
    {
        nNumDocs = count;
    }
    /**
     *add a collection's base document id to the barrel
     * Each barrel may include several collections, each collection has its own base doc id, base doc id is used when deleting a document together with the doc count of
     * the collection in the barrel to determine in which barrel the deleted document lies.
     */

    void addBaseDocID(collectionid_t colID, docid_t baseDocID)
    {
        baseDocIDMap[colID] = baseDocID;
    }
    /**
     * set base doc id map.
     * Each barrel may include several collections, each collection has its own base doc id, base doc id is used when deleting a document together with the doc count of
     * the collection in the barrel to determine in which barrel the deleted document lies.
     */
    void setBaseDocID(map<collectionid_t,docid_t>& baseMap)
    {
        baseDocIDMap = baseMap;
    }

    docid_t getBaseDocID()
    {
        if(baseDocIDMap.empty())
            return BAD_DOCID;
        else
            return baseDocIDMap.begin()->second;
    }

    docid_t getMaxDocID() { return maxDocId; }
    /**
     * Get IndexBarrelWriter handle, after the indexed document has been flushed into barrel files, the IndexBarrelWriter handle will be set NULL,
     * if it is not NULL, it means the index is currently still an in-memory index
     */
    IndexBarrelWriter* getWriter() { return pBarrelWriter; }
    /**
     * Set IndexBarrelWriter, then the writer will index the documents within this barrel
     */
     void setWriter(IndexBarrelWriter* pWriter) {
        DVLOG(2) << "BarrelInfo::setWriter(), barrel: " << getName() << ", pWriter: " << pWriter;
        pBarrelWriter = pWriter;
     }

    /**
     * Decrease the document counter @c nNumDocs.
     * @param docId doc id to delete
     * @return true for success, false for fail (document counter is already 0)
     */
    bool deleteDocument(docid_t docId)
    {
        assert(getBaseDocID() <= docId && getMaxDocID() >= docId);

        if(nNumDocs > 0)
        {
            --nNumDocs;
            return true;
        }

        DVLOG(2) << "BarrelInfo::deleteDocument() failed, nNumDocs: " << nNumDocs;
        return false;

        ///TODO  maxDocId issue
    }

    void updateMaxDoc(docid_t docId)
    {
        maxDocId = (docId>maxDocId)?docId:maxDocId;
    }
    /**
     * delete index files of this barrel
     */
    void remove(Directory* pDirectory);
    /**
     * rename the barrel name, it is used when index merge happens.
     */
    void rename(Directory* pDirectory,const string& newName);
    /**
     * write in-memory barrel into disk files such as ".voc", ".dfp", ".pop" and ".fdi".
     */
    void write(Directory* pDirectory);

    /**
     * whether this barrel is removed because of merge.
     */
    bool isRemoved() { return isRemoved_; }

    void setSearchable(bool cansearch)
    {
        searchable = cansearch;
    }

    bool isSearchable() { return searchable; }

    void setRealTime(bool realTime)
    {
        inMemoryBarrel = realTime;
    }
    bool isRealTime() { return inMemoryBarrel; }

    void registerIndexInput(IndexInput* pIndexInput);

    void unRegisterIndexInput(IndexInput* pIndexInput);

    void setDirty();

    ///compare function to sort all the barrels, the compare function will be based on the document count of a certain barrel.
    ///we will sort barrels according to base doc id of the first collection.
    static bool greater (BarrelInfo* pElem1, BarrelInfo* pElem2 )
    {
        if( pElem1->getBaseDocID() != pElem2->getBaseDocID() )
            return pElem1->getBaseDocID() < pElem2->getBaseDocID();
        else
            return pElem1->getDocCount() > pElem2->getDocCount();
    }

public:
    ///barrel name
    string barrelName;
    ///map of document base id of different collections in a certain barrel
    map<collectionid_t,docid_t> baseDocIDMap;
    ///document count of this barrel
    count_t nNumDocs;
    ///only valid when this barrel is a in-memory barrel,otherwise NULL.
    IndexBarrelWriter* pBarrelWriter;
    ///whether this barrel contains updated documents
    bool isUpdate;
    ///whether this barrel is in-memory barrel 
    bool inMemoryBarrel;
    ///max doc of this barrel
    docid_t maxDocId;

    bool searchable;

    ///all index input instances generated for this barrel
    std::set<IndexInput*> indexInputs;

    IndexLevel indexLevel_;

    CompressionType compressType;

private:
    boost::mutex mutex_;

    ///whether this barrel is removed because of merge
    bool isRemoved_;
};


///barrels information, description of all barrels in the index database
/**barrels information will be flushed to the file "barrels", it is a XML file generated automatically and should not be modified manually.
*Example:
*<!--start of the barrels info -->
*<database>
* <!--version of indexes -->
* <version>5.0</version>
* <!-- barrel info counter  -->
* <barrel_counter>5</barrel_counter>
* <!-- number of the barrels  -->
* <barrel_count>3</barrel_count>
* <!-- start of barrels -->
* <barrels>
*  <!-- barrel _0  -->
*  <barrel>
*   <!-- barrel name  -->
*   <name>_0</name>
*   <!-- baseDocIDMap  -->
*   <doc_begin>0:0,</doc_begin>
*   <!-- number of the document indexed in this barrel  -->
*   <doc_count>17824</doc_count>
*  </barrel>
*  <!-- barrel _1 -->
*  <barrel>
*   <!-- barrel name  -->
*   <name>_1</name>
*   <!-- baseDocIDMap  -->
*   <doc_begin>0:58933,</doc_begin>
*   <!-- number of the document indexed in this barrel  -->
*   <doc_count>5955</doc_count>
*  </barrel>
* </barrels>
*</database>
*/
class BarrelsInfo
{
public:
    BarrelsInfo(IndexLevel indexLevel);

    ~BarrelsInfo(void);
public:
    const string newBarrel();

    /**
     * Add @p pBarrelInfo into @c barrelInfos.
     * @p pBarrelInfo pointer to the BarrelInfo to add
     * @p bLock whether to acquire the lock of @c mutex_ before add @p pBarrelInfo
     * @note this function might be called with @c next() concurrently,
     * to avoid invalidate @c barrelsiterator,
     * if @p bLock is true, this function would first acquire the lock of @c mutex_,
     * if @p bLock is false, the caller should ensure the lock of @c mutex_
     * has been acquired before calling this function.
     */
    void addBarrel(BarrelInfo* pBarrelInfo, bool bLock = true);

    void read(Directory* pDirectory, const char* name = BARRELS_INFONAME);

    void write(Directory* pDirectory);

    void remove(Directory* pDirectory);

    /**
     * Remove the BarrelInfo and its files of barrel @p barrelname.
     * @p pDirectory pointer to file system interface used to remove files
     * @p barrelname the name of barrel to remove
     * @note the same to the note of @c addBarrel().
     */
    void removeBarrel(Directory* pDirectory, const string& barrelname, bool bLock = true);
    /// get number of barrels
    int32_t getBarrelCount();
    /// counter of barrel name
    int32_t getBarrelCounter() {return nBarrelCounter;}

    int32_t getDocCount();

    const char* getVersion() { return version.c_str();  }

    void setVersion(const char* ver);

    void sort(Directory* pDirectory);

    void clear();

    void updateMaxDoc(docid_t docId);

    docid_t maxDocId() {return maxDoc;}

    void resetMaxDocId(docid_t docId) { maxDoc = docId;}

    BarrelInfo* operator [](int32_t i) { return barrelInfos[i];}

    /**
     * Move the iterator of BarrelInfo to the first one.
     * @note as @c next() might be called with @c addBarrel() or @c removeBarrel() concurrently,
     * to avoid invalidate @c barrelsiterator, the caller should ensure the lock of @c mutex_
     * has been acquired before calling this function.
     */
    void startIterator();

    bool hasNext();

    BarrelInfo* next();

    boost::mutex& getMutex() { return mutex_; }

    void setSearchable();

    /**
     * Find the barrel which doc range includes @p docId to delete,
     * then decrease the document counter of that barrel.
     * @param colId collection id
     * @param docId doc id to delete
     * @return true for success, false for fail (no barrel is found to include @p docId)
     */
    bool deleteDocument(collectionid_t colId, docid_t docId);

    void printBarrelsInfo(Directory* pDirectory);

private:
    string version;

    int32_t nBarrelCounter; ///barrel counter

    vector<BarrelInfo*> barrelInfos;

    vector<BarrelInfo*>::iterator barrelsiterator;

    vector<BarrelInfo*> rubbishBarrelInfos;

    docid_t maxDoc;

    boost::mutex mutex_;

    IndexLevel indexLevel_;
};

//////////////////////////////////////////////////////////////////////////
//Inline functions

inline void BarrelsInfo::startIterator()
{
    barrelsiterator = barrelInfos.begin();
}
inline  bool BarrelsInfo::hasNext()
{
    return (barrelsiterator != barrelInfos.end());
}
inline  BarrelInfo* BarrelsInfo::next()
{
    return *barrelsiterator++;
}


}

NS_IZENELIB_IR_END

#endif
