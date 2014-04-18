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
#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/utility/MemCache.h>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#define NUM_CACHEDTERMINFO 100

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

    void writeTermInfo(
    IndexOutput* pVocWriter,
    termid_t tid,
    const TermInfo& termInfo);
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
        int64_t ret = a->pCurTerm_->compare(b->pCurTerm_);
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
struct MergeTermInfo
{
    termid_t term_;

    TermInfo termInfo_;

    friend class FieldMerger;
};

/**
*@brief FieldMerger will be called by IndexMerger internally, it process the index merge process within a certain Field
*/
class FieldMerger
{
public:
    FieldMerger(bool sortingMerge, int skipInterval, int maxSkipLevel, IndexLevel indexLevel);

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

    void setDocFilter(Bitset* pFilter)
    {
        pDocFilter_ = pFilter;
    }

    void initPostingMerger(
        CompressionType compressType,
        bool optimize,
        bool requireIntermediateFileForMerging,
        size_t memPoolSizeForPostingMerger
    );

private:
    /** initialize merge queue */
    bool initQueue();

    /**
     * merge terms
     * @param ppMergeInfos merge information array
     * @param numInfos size of info array
     */
    inline void mergeTerms(FieldMergeInfo** ppMergeInfos,int32_t numInfos,TermInfo& ti);

    void sortingMerge(FieldMergeInfo** ppMergeInfos,int32_t numInfos,TermInfo& ti);

private:
    /**
     * flush merged term info, Subclasses must define this one method.
     * @param pOutputDescriptor output descriptor
     * @param numTermInfos number of term infos.
     */
    void flushTermInfo(OutputDescriptor* pOutputDescriptor, int32_t numTermInfos) ;

    /**
     * merge is over, Subclasses must define this one method.
     * @param pOutputDescriptor output descriptor
     * @return offset of vocabulary descriptor.
     */
    fileoffset_t endMerge(OutputDescriptor* pOutputDescriptor) ;
private:
    bool sortingMerge_;

    int skipInterval_;

    int maxSkipLevel_;

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

    MergeTermInfo cachedTermInfos_[NUM_CACHEDTERMINFO];

    Bitset* pDocFilter_;

    boost::shared_ptr<MemCache> pMemCache_;

    IndexLevel indexLevel_;
};
//////////////////////////////////////////////////////////////////////////
//inline
inline void FieldMerger::mergeTerms(FieldMergeInfo** ppMergeInfos,int32_t numInfos,TermInfo& ti)
{
    if(sortingMerge_)
        return sortingMerge(ppMergeInfos, numInfos, ti);

    PostingReader* pPosting;
    for (int32_t i = 0;i< numInfos;i++)
    {
        pPosting = ppMergeInfos[i]->pIterator_->termPosting();
        if (ppMergeInfos[i]->pBarrelInfo_->getWriter())///in-memory posting
            pPostingMerger_->mergeWith((MemPostingReader*)pPosting);
        else
        {
            switch(ppMergeInfos[i]->pBarrelInfo_->compressType)
            {
            case BYTEALIGN:
                pPostingMerger_->mergeWith((RTDiskPostingReader*)pPosting, pDocFilter_);
                break;
            case BLOCK:
                pPostingMerger_->mergeWith((BlockPostingReader*)pPosting, pDocFilter_);
                break;
            case CHUNK:
                pPostingMerger_->mergeWith((ChunkPostingReader*)pPosting, pDocFilter_);
                break;
            default:
                assert(false);
            }
        }
    }
    fileoffset_t ret = pPostingMerger_->endMerge();
    if(ret != -1)
        ti.set(pPostingMerger_->termInfo_);
    else
        ti.reset();
}

}

NS_IZENELIB_IR_END

#endif
