/**
* @file        PostingMerger.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   merge index at posting level
*/

#ifndef POSTINGMERGER_H
#define POSTINGMERGER_H

#include <ir/index_manager/index/Posting.h>
#include <ir/index_manager/index/OutputDescriptor.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/SkipListMerger.h>
#include <ir/index_manager/utility/BitVector.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
#define POSTINGMERGE_BUFFERSIZE	32768
///Posting Merger
class PostingMerger
{
public:
    PostingMerger();

    PostingMerger(OutputDescriptor* pOutputDescriptor);

    virtual ~PostingMerger();
public:
    void setBuffer(char* buf,size_t bufSize);

    void setOutputDescriptor(OutputDescriptor* pOutputDescriptor)
    {
        pOutputDescriptor_ = pOutputDescriptor;
    }

    OutputDescriptor* getOutputDescriptor() {return pOutputDescriptor_;}

    void mergeWith(InMemoryPosting* pInMemoryPosting);

    void mergeWith(OnDiskPosting* pOnDiskPosting);

    void mergeWith(OnDiskPosting* pOnDiskPosting,BitVector* pFilter);

    void mergeWith_GC(OnDiskPosting* pOnDiskPosting,BitVector* pFilter);

    fileoffset_t endMerge();

    void reset();

private:
    /** create buffer_ for merging */
    void init();

private:
    friend class FieldMerger;

    TermInfo termInfo_;

    char* buffer_; ///buffer_ for posting merging

    size_t bufsize_; ///size of buffer_

    bool bOwnBuffer_; ///does we own the buffer_?

    OutputDescriptor* pOutputDescriptor_; ///where merged data store

    PostingDescriptor postingDesc_;

    ChunkDescriptor chunkDesc_;

    int64_t nPPostingLength_;

    bool bFirstPosting_;

    SkipListMerger* pSkipListMerger_;

    MemCache* pMemCache_; /// memory cache
};

}

NS_IZENELIB_IR_END

#endif
