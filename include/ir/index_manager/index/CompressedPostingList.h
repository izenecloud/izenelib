#ifndef COMPRESSED_POSTINGLIST_H
#define COMPRESSED_POSTINGLIST_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#pragma pack(push,1)
struct PostingChunk
{
    int32_t size;
    PostingChunk* next;
    uint8_t data[1];
};
#pragma pack(pop)

/// the descriptor of posting
struct PostingDescriptor
{
    int64_t length; 	///length of the posting
    count_t df; 		///document frequency of this field
    count_t tdf;		///document frequency regarding all fields in a document
    int64_t ctf;		///global  term frequency
    fileoffset_t	poffset;	///offset of the position postings in the .pop file
};

/// the descriptor of chunk
struct ChunkDescriptor
{
    int64_t length; 	///length of the chunk
    docid_t lastdocid;	///the last doc id of the chunk
};

/**
*compressed posting list
*/
class CompressedPostingList
{
public:
    CompressedPostingList()
            :pHeadChunk(NULL)
            ,pTailChunk(NULL)
            ,nTotalSize(0)
            ,nTotalUnused(0)
            ,nPosInCurChunk(0)
    {
    }
    CompressedPostingList(const CompressedPostingList& src)
            :pHeadChunk(src.pHeadChunk)
            ,pTailChunk(src.pTailChunk)
            ,nTotalSize(src.nTotalSize)
            ,nTotalUnused(src.nTotalUnused)
            ,nPosInCurChunk(src.nPosInCurChunk)
    {
    }
    ~CompressedPostingList()
    {
    }
public:
    /**
     * add posting which is 32 bit
     * @param posting32 32 bit posting
     * @return return false if no enough space, else return true.
     */
    bool addPosting(uint32_t posting32);

    /**
     * add posting which is 64 bit
     * @param posting64 64 bit posting
     * @return return false if no enough space, else return true.
     */
    bool addPosting(uint64_t posting64);

    /**
     * decode 32bit posting
     * @param posting the posting
     * @return decoded value
     */
    static int32_t decodePosting32(uint8_t*& posting);

    /**
     * decode 64bit posting
     * @param posting the posting
     * @return decoded value
     */
    static int64_t decodePosting64(uint8_t*& posting);

    /**
     * encode 32bit posting
     * @param posting the encoded posting
     * @param val value needed to encode
     */
    static void encodePosting32(uint8_t*& posting,int32_t val);

    /**
     * encode 64bit posting
     * @param posting the posting
     * @param val value needed to encode
     */
    static void encodePosting64(uint8_t*& posting,int64_t val);

    /**
     * add a new node to the list
     * @param pNode the node
     */
    void addChunk(PostingChunk* pChunk);

    /** truncation the tail chunk,let chunk size=real used size of this chunk */
    void truncTailChunk();

    /** get the real size of the list */
    int32_t getRealSize();

    /**
     * reset the list for using at next time
     */
    void reset();
protected:
    PostingChunk* pHeadChunk;		///Posting list header
    PostingChunk* pTailChunk;			///Posting list tail
    uint32_t nTotalSize;			///Total size
    uint32_t nTotalUnused;		///Total Unused size
    uint32_t nPosInCurChunk;

    friend class InMemoryPosting;
    friend class PostingMerger;
};

}
NS_IZENELIB_IR_END

#endif
