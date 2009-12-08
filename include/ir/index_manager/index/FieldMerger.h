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
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/utility/MemCache.h>

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
            :pFieldInfo_(NULL)
            ,pBarrelInfo_(NULL)
    {
    }
    MergeFieldEntry(BarrelInfo* pBarrel,FieldInfo* pFieldInfo)
            :pFieldInfo_(new FieldInfo(*pFieldInfo))
            ,pBarrelInfo_(pBarrel)
    {

    }
    ~MergeFieldEntry()
    {
        if (pFieldInfo_)
        {
            delete pFieldInfo_;
            pFieldInfo_ = NULL;
        }
        pBarrelInfo_ = NULL;
    }
public:
    FieldInfo* pFieldInfo_;
    BarrelInfo* pBarrelInfo_;

    friend class FieldMerger;
};
/**
*@brief FieldMergeInfo is the helper class to merge term one by one in each Field
*/
class FieldMergeInfo
{
public:
    FieldMergeInfo(int32_t nOrder, collectionid_t currColId, BarrelInfo* pBarrelInfo,TermReader* pTermReader)
            :nOrder_(nOrder)
            ,pBarrelInfo_(pBarrelInfo)
            ,pCurTerm_(NULL)
            ,pTermReader_(pTermReader)
    {
        pIterator_ = pTermReader_->termIterator(NULL);
        baseDocId_ = pBarrelInfo_->baseDocIDMap[currColId];;
    }
    ~FieldMergeInfo()
    {
        delete pTermReader_;
        pTermReader_ = NULL;
        if (pIterator_)
            delete pIterator_;
        pIterator_ = NULL;
    }
public:
    bool next()
    {
        if (pIterator_ && pIterator_->next())
        {
            pCurTerm_ = (Term*)pIterator_->term();
            return true;
        }
        return false;
    }
	
public:
    int32_t nOrder_;			///order of barrel
    BarrelInfo* pBarrelInfo_;		///reference to barrel info
    Term* pCurTerm_;		///current term
    TermReader* pTermReader_;
    TermIterator* pIterator_;		///term iterator
    docid_t baseDocId_;

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
        int32_t ret = a->pCurTerm_->compare(b->pCurTerm_);
        if(ret == 0)
        {
            return a->baseDocId_ < b->baseDocId_;
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
            :pTerm_(NULL)
            ,pTermInfo_(NULL)
    {}
    MergeTermInfo(Term* pTerm,TermInfo* pTermInfo)
            :pTerm_(pTerm)
            ,pTermInfo_(pTermInfo)
    {}
    ~MergeTermInfo()
    {
        if (pTerm_)
            delete pTerm_;
        if (pTermInfo_)
            delete pTermInfo_;
    }
public:
    Term* getTerm() { return pTerm_; }

    TermInfo* getTermInfo() { return pTermInfo_; }

public:
    Term* pTerm_;

    TermInfo* pTermInfo_;

    friend class FieldMerger;
};

/**
*@brief FieldMerger will be called by IndexMerger internally, it process the index merge process within a certain Field
*/
class FieldMerger
{
public:
    FieldMerger();

    virtual ~FieldMerger(void);
public:
    /**
     * add a field to merger
     * @param pBarrelInfo_ about the barrel of the field
     * @param pFieldInfo_ about the field
     */
    void addField(BarrelInfo* pBarrelInfo,FieldInfo* pFieldInfo);

    /** number of fields */
    int32_t numFields()
    {
        return (int32_t)fieldEntries_.size();
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
    void setDirectory(Directory* pDirectory)
    {
        pDirectory_ = pDirectory;
    }

    /**
     * get directory of index database
     * @return internal directory object
     */
    Directory* getDirectory()
    {
        return pDirectory_;
    }

    /**
     * get total number of merged terms
     * @return number of merged terms
     */
    int64_t numMergedTerms()
    {
        return nMergedTerms_;
    }

    void setDocFilter(BitVector* pFilter)
    {
        pDocFilter_ = pFilter;
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

    fileoffset_t sortingMerge(FieldMergeInfo** ppMergeInfos,int32_t numInfos, BitVector* pFilter);

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
    char* buffer_;

    size_t bufsize_;

    FieldMergeQueue* pMergeQueue_;

    PostingMerger* pPostingMerger_;

    int32_t nNumTermCached_;

    Directory* pDirectory_;

    MergeFieldEntry** ppFieldInfos_;

    size_t nNumInfos_;

    int64_t termCount_;	///total term count

    termid_t lastTerm_;		///last term id

    fileoffset_t lastPOffset_;	///last posting offset

    fileoffset_t beginOfVoc_;	///begin of vocabulary data

    int64_t nMergedTerms_;

    vector<MergeFieldEntry*> fieldEntries_;

    MergeTermInfo* cachedTermInfos_[NUM_CACHEDTERMINFO];

    BitVector* pDocFilter_;

    MemCache* pMemCache_;
};
//////////////////////////////////////////////////////////////////////////
//inline
inline fileoffset_t FieldMerger::mergeTerms(FieldMergeInfo** ppMergeInfos,int32_t numInfos)
{
    Posting* pPosting;
    bool mergePostingHasUpdatedDocs = false;
    for (int32_t i = 0;i< numInfos;i++)
    {
        if(ppMergeInfos[i]->pBarrelInfo_->hasUpdateDocs)
        {
            mergePostingHasUpdatedDocs = true;
            break;
        }
    }
    if(mergePostingHasUpdatedDocs)
        return sortingMerge(ppMergeInfos, numInfos, pDocFilter_);

    for (int32_t i = 0;i< numInfos;i++)
    {
        pPosting = ppMergeInfos[i]->pIterator_->termPosting();
        if (ppMergeInfos[i]->pBarrelInfo_->getWriter())///in-memory posting
            pPostingMerger_->mergeWith((InMemoryPosting*)pPosting);
        else
        {
            if(pDocFilter_)
                pPostingMerger_->mergeWith((OnDiskPosting*)pPosting,pDocFilter_);
            else
                pPostingMerger_->mergeWith((OnDiskPosting*)pPosting);
        }

    }
    ///end merge
    return pPostingMerger_->endMerge();
}

}

NS_IZENELIB_IR_END

#endif
