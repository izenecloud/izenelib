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
    /**
     * set buffer for posting merging
     * @param buf buffer
     * @param bufSize size of buffer
     */
    void setBuffer(char* buf,size_t bufSize);

    /**
     * set output descriptor for merging
     * @param pOutputDescriptor output descriptor
     */
    void setOutputDescriptor(OutputDescriptor*	pOutputDescriptor)
    {
        this->pOutputDescriptor=pOutputDescriptor;
    }

public:
    /**
     * merge a in-memory posting
     * @param pInMemoryPosting in-memory posting
     */
    void mergeWith(InMemoryPosting* pInMemoryPosting);

    /**
     * merge a on-disk posting
     * @param pOnDiskPosting on-disk posting
     */
    void mergeWith(OnDiskPosting* pOnDiskPosting);

    /**
     * end the merge,flush posting descriptor and chunk descriptor to disk.
     * @return offset of posting descriptor
     */
    fileoffset_t endMerge();

public:
    /**
     * reset descriptors value
     */
    inline void reset();

public:
    /**
     * get posting descriptor
     * @return posting descriptor,internal object
     */
    PostingDescriptor& getPostingDescriptor()
    {
        return postingDesc;
    }

    /**
     * get chunk descriptor
     * @return chunk descriptor,internal object
     */
    ChunkDescriptor& getChunkDescriptor()
    {
        return chunkDesc;
    }

private:
    /** create buffer for merging */
    void createBuffer();

private:
    char* buffer;				///buffer for posting merging

    size_t bufsize;				///size of buffer

    bool bOwnBuffer;			///does we own the buffer?

    OutputDescriptor* pOutputDescriptor;	///where merged data store

    PostingDescriptor postingDesc;

    ChunkDescriptor chunkDesc;

    int64_t nPPostingLength;

    bool bFirstPosting;
};

}

NS_IZENELIB_IR_END

#endif
