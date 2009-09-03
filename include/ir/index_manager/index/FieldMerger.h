/**
* @file        IndexMerge.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Process the index merging process in a certain field
*/
#ifndef FIELDMERGER_H
#define FIELDMERGER_H

#include <ir/index_manager/index/FieldInfo.h>
#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/AbsTermIterator.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/utility/PriorityQueue.h>
#include <ir/index_manager/index/PostingMerger.h>
#include <ir/index_manager/index/AbsTermReader.h>

#include <string>
#include <vector>


#define NUM_CACHEDTERMINFO 100

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
*@brief MergeFieldEntry is a pair of FieldInfo and BarrelInfo
*/
class MergeFieldEntry
{
public:
    MergeFieldEntry()
            :pFieldInfo(NULL)
            ,pBarrelInfo(NULL)
    {
    }
    MergeFieldEntry(BarrelInfo* pBarrel,FieldInfo* pFieldInfo_)
            :pFieldInfo(new FieldInfo(*pFieldInfo_))
            ,pBarrelInfo(pBarrel)
    {

    }
    ~MergeFieldEntry()
    {
        if (pFieldInfo)
        {
            delete pFieldInfo;
            pFieldInfo = NULL;
        }
        pBarrelInfo = NULL;
    }
public:
    FieldInfo* pFieldInfo;
    BarrelInfo* pBarrelInfo;

    friend class FieldMerger;
};
/**
*@brief FieldMergeInfo is the helper class to merge term one by one in each Field
*/
class FieldMergeInfo
{
public:
    FieldMergeInfo(int32_t nOrder_, collectionid_t currColId, BarrelInfo* pBarrelInfo_,TermReader* pTermReader_)
            :nOrder(nOrder_)
            ,pBarrelInfo(pBarrelInfo_)
            ,pCurTerm(NULL)
            ,pTermReader(pTermReader_)
    {
        pIterator = pTermReader->termIterator(NULL);
        baseDocId = pBarrelInfo->baseDocIDMap[currColId];;
    }
    ~FieldMergeInfo()
    {
        delete pTermReader;
        pTermReader = NULL;
        if (pIterator)
            delete pIterator;
        pIterator = NULL;
    }
public:
    bool next()
    {
        if (pIterator && pIterator->next())
        {
            pCurTerm = (Term*)pIterator->term();
            return true;
        }
        return false;
    }
	
public:
    int32_t nOrder;			///order of barrel
    BarrelInfo* pBarrelInfo;		///reference to barrel info
    Term* pCurTerm;		///current term
    TermReader* pTermReader;
    TermIterator* pIterator;		///term iterator
    docid_t baseDocId;

    friend class FieldMergeQueue;
    friend class FieldMerger;
};

/**
* @brief FieldMergeQueue is a priorityqueue with element type of FieldMergeInfo
*/
class FieldMergeQueue : public PriorityQueue<FieldMergeInfo*>
{
public:
    FieldMergeQueue(size_t maxSize)
    {
        initialize(maxSize,true);
    }
    ~FieldMergeQueue() {}
private:
    /**
     * Determines the ordering of objects in this priority queue.
     * Subclasses must define this one method.
     */
    bool lessThan(FieldMergeInfo* a, FieldMergeInfo* b)
    {
        int32_t ret = a->pCurTerm->compare(b->pCurTerm);
        if(ret == 0)
        {
            return a->baseDocId < b->baseDocId;
        }
        else
            return (ret < 0);
    }
};

/**
*@brief MergeTermInfo is a pair used when merge postings
*/
class MergeTermInfo
{
public:
    MergeTermInfo()
            :pTerm(NULL)
            ,pTermInfo(NULL)
    {}
    MergeTermInfo(Term* pTerm_,TermInfo* pTermInfo_)
            :pTerm(pTerm_)
            ,pTermInfo(pTermInfo_)
    {}
    ~MergeTermInfo()
    {
        if (pTerm)
            delete pTerm;
        if (pTermInfo)
            delete pTermInfo;
    }
public:
    Term* getTerm()
    {
        return pTerm;
    }
    TermInfo* getTermInfo()
    {
        return pTermInfo;
    }
public:
    Term* pTerm;
    TermInfo* pTermInfo;

    friend class FieldMerger;
};

/**
*@brief FieldMerger will be called by IndexMerger internally, it process the index merge process within a certain Field
*/
class FieldMerger
{
public:
    FieldMerger();
    FieldMerger(Directory* pDirectory);
    virtual ~FieldMerger(void);
public:
    /**
     * add a field to merger
     * @param pBarrelInfo_ about the barrel of the field
     * @param pFieldInfo_ about the field
     */
    void addField(BarrelInfo* pBarrelInfo_,FieldInfo* pFieldInfo_);

    /** number of fields */
    int32_t numFields()
    {
        return (int32_t)fieldEntries.size();
    }

    /**
     * merge
     * @param pDesc to store merge result
     * @return offset of field vocabulary descriptor
     */
    fileoffset_t merge(OutputDescriptor* pOutputDescriptor);

    /**
     * set buffer for merging
     * @param buf buffer
     * @param bufSize size of buffer
     */
    void setBuffer(char* buf,size_t bufSize);

    /**
     * set directory of index database
     * @param pDirectory directory
     */
    void setDirectory(Directory* pDirectory_)
    {
        pDirectory = pDirectory_;
    }

    /**
     * get directory of index database
     * @return internal directory object
     */
    Directory* getDirectory()
    {
        return pDirectory;
    }

    /**
     * get total number of merged terms
     * @return number of merged terms
     */
    int64_t numMergedTerms()
    {
        return nMergedTerms;
    }
private:
    /** initialize merge queue */
    bool initQueue();

    /**
     * merge terms
     * @param ppMergeInfos merge information array
     * @param numInfos size of info array
     */
    inline fileoffset_t	mergeTerms(FieldMergeInfo** ppMergeInfos,int32_t numInfos);
private:
    /**
     * flush merged term info, Subclasses must define this one method.
     * @param pOutputDescriptor output descriptor
     * @param ppTermInfo merged term infos
     * @param numTermInfos number of term infos.
     */
    void flushTermInfo(OutputDescriptor* pOutputDescriptor,MergeTermInfo** ppTermInfo,int32_t numTermInfos) ;

    /**
     * merge is over, Subclasses must define this one method.
     * @param pOutputDescriptor output descriptor
     * @return offset of vocabulary descriptor.
     */
    fileoffset_t endMerge(OutputDescriptor* pOutputDescriptor) ;
private:
    char* buffer;

    size_t bufsize;

    FieldMergeQueue* pMergeQueue;

    PostingMerger* pPostingMerger;

    int32_t nNumTermCached;

    Directory* pDirectory;

    MergeFieldEntry** ppFieldInfos;

    size_t nNumInfos;

    int64_t termCount;	///total term count

    termid_t lastTerm;		///last term id

    fileoffset_t lastPOffset;	///last posting offset

    fileoffset_t beginOfVoc;	///begin of vocabulary data

    int64_t nMergedTerms;

    vector<MergeFieldEntry*> fieldEntries;

    MergeTermInfo* cachedTermInfos[NUM_CACHEDTERMINFO];
};
//////////////////////////////////////////////////////////////////////////
//inline
inline fileoffset_t FieldMerger::mergeTerms(FieldMergeInfo** ppMergeInfos,int32_t numInfos)
{
    Posting* pPosting;
    for (int32_t i = 0;i< numInfos;i++)
    {
        pPosting = ppMergeInfos[i]->pIterator->termPosting();
        if (ppMergeInfos[i]->pBarrelInfo->getWriter())///in-memory posting
            pPostingMerger->mergeWith((InMemoryPosting*)pPosting);
        else
            pPostingMerger->mergeWith((OnDiskPosting*)pPosting);
    }
    ///end merge
    return pPostingMerger->endMerge();
}

}

NS_IZENELIB_IR_END

#endif
