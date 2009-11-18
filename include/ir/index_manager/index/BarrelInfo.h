/**
* @file        BarrelInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief barrels information, description of all barrels in the index database
*/

#ifndef BARRELINFO_H
#define BARRELINFO_H

#include <ir/index_manager/store/Directory.h>

#include <vector>
#include <map>
#include <sstream>


#define BARRELS_INFONAME "barrels"

NS_IZENELIB_IR_BEGIN
namespace indexmanager{
class IndexBarrelWriter;
///barrel information, description of index barrel
class BarrelInfo
{
public:
    BarrelInfo()
            : nNumDocs(0)
            , pBarrelWriter(NULL)
            , hasUpdateDocs(false)
            , maxDocId(0)
            , modified(false)
 
    {
    }

    BarrelInfo(const string& name,count_t count)
            : barrelName(name)
            , nNumDocs(count)
            , pBarrelWriter(NULL)
            , hasUpdateDocs(false)
            , maxDocId(0)
            , modified(false)
    {
    }


    BarrelInfo(BarrelInfo* pBarrelInfo)
            : barrelName(pBarrelInfo->barrelName)
            , baseDocIDMap(pBarrelInfo->baseDocIDMap)    
            , nNumDocs(pBarrelInfo->nNumDocs)
            , pBarrelWriter(NULL)
            , hasUpdateDocs(pBarrelInfo->hasUpdateDocs)
            , maxDocId(pBarrelInfo->maxDocId)
            , modified(pBarrelInfo->modified)
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
        baseDocIDMap.insert(make_pair(colID,baseDocID));
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
            return 0;
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
     void setWriter(IndexBarrelWriter* pWriter) { pBarrelWriter = pWriter; }
    /**
     * after delete a document from a certain barrel, the document counter should be updated
     */
    void deleteDocument(docid_t docId)
    {
        nNumDocs--;
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

    bool isModified() { return modified; }

    ///compare function to sort all the barrels, the compare function will be based on the document count of a certain barrel.
    ///we will sort barrels according to base doc id of the first collection.
    static bool less (BarrelInfo* pElem1, BarrelInfo* pElem2 )
    {
        if( pElem1->getBaseDocID() != pElem2->getBaseDocID() )
            return pElem1->getBaseDocID() < pElem2->getBaseDocID();
        else
            return pElem1->getDocCount() > pElem2->getDocCount();
    }
     
    static bool greater (BarrelInfo* pElem1, BarrelInfo* pElem2 )
    {
        if( pElem1->getBaseDocID() != pElem2->getBaseDocID() )
            return pElem1->getBaseDocID() > pElem2->getBaseDocID();
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

    bool hasUpdateDocs;
    ///max doc of this barrel
    docid_t maxDocId;

    bool modified;
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
    BarrelsInfo();

    ~BarrelsInfo(void);
public:
    const string newBarrel();

    void addBarrel(const char* name,count_t docCount);

    void addBarrel(BarrelInfo* pBarrelInfo,bool bCopy = true);

    void read(Directory* pDirectory, const char* name = BARRELS_INFONAME);

    void write(Directory* pDirectory);

    void remove(Directory* pDirectory);

    void removeBarrel(Directory* pDirectory,const string& barrelname);
    /// get number of barrels
    int32_t getBarrelCount();

    int32_t getBarrelCounter() {return nBarrelCounter;}

    int32_t getDocCount();

    count_t numDeletedDocs();

    const char* getVersion() { return version.c_str();  }

    void setLock(bool lock_){lock = lock_; }

    bool getLock() {return lock; }

    void setVersion(const char* ver);

    BarrelInfo* getBarrelInfo(const char* barrel);

    BarrelInfo* getLastBarrel();

    void deleteLastBarrel();

    void sort(Directory* pDirectory);

    void clear();

    void updateMaxDoc(docid_t docId);

    docid_t maxDocId() {return maxDoc;}

    void resetMaxDocId(docid_t docId) { maxDoc = docId;}

    BarrelInfo* operator [](int32_t i) { return barrelInfos[i];}

    void startIterator();

    bool hasNext();

    BarrelInfo* next();
private:
    //lock means index is under merging and can not serve the query requests.
    bool lock;

    string version;

    int32_t nBarrelCounter; ///barrel counter

    vector<BarrelInfo*> barrelInfos;

    vector<BarrelInfo*>::iterator barrelsiterator;

    docid_t maxDoc;
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
