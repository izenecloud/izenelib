/**
* @file        VariantDataPool.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief CompressedPosting data
*/

#ifndef VARIANT_DATA_POOL_H
#define VARIANT_DATA_POOL_H

#include <ir/index_manager/utility/system.h>

#include <ir/index_manager/store/IndexOutput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#pragma pack(push,1)
struct VariantDataChunk
{
    int32_t size;
    VariantDataChunk* next;
    uint8_t data[1];
};
#pragma pack(pop)

/**
* VariantDataPool
*/
class VariantDataPool
{
public:
    VariantDataPool()
            :pHeadChunk_(NULL)
            ,pTailChunk_(NULL)
            ,nTotalSize_(0)
            ,nTotalUnused_(0)
            ,nPosInCurChunk_(0)
    {
    }
    VariantDataPool(const VariantDataPool& src)
            :pHeadChunk_(src.pHeadChunk_)
            ,pTailChunk_(src.pTailChunk_)
            ,nTotalSize_(src.nTotalSize_)
            ,nTotalUnused_(src.nTotalUnused_)
            ,nPosInCurChunk_(src.nPosInCurChunk_)
    {
    }
    ~VariantDataPool()
    {
    }
public:
    /**
     * add variant length data which is 32 bit
     * @param vdata32 32 bit variant length data
     * @return return false if no enough space, else return true.
     */
    bool addVData(uint32_t vdata32);

    /**
     * add variant length data which is 64 bit
     * @param vdata64 64 bit variant length data
     * @return return false if no enough space, else return true.
     */
    bool addVData(uint64_t vdata64);

    /**
     * write all data contained to disk;
     */
    void write(IndexOutput* pOutput);

    /**
     * decode 32bit vdata
     * @param vdata the vdata
     * @return decoded value
     */
    static int32_t decodeVData32(uint8_t*& vdata);

    /**
     * decode 64bit vdata
     * @param vdata the vdata
     * @return decoded value
     */
    static int64_t decodeVData64(uint8_t*& vdata);

    /**
     * encode 32bit vdata
     * @param vdata the encoded vdata
     * @param val value needed to encode
     */
    static void encodeVData32(uint8_t*& vdata,int32_t val);

    /**
     * encode 64bit vdata
     * @param vdata the vdata
     * @param val value needed to encode
     */
    static void encodeVData64(uint8_t*& vdata,int64_t val);

    /**
     * add a new node to the list
     * @param pNode the node
     */
    void addChunk(VariantDataChunk* pChunk);

    /** truncation the tail chunk,let chunk size=real used size of this chunk */
    void truncTailChunk();

    /** get the real size of the list */
    int32_t getRealSize();

    /**
     * reset the list for using at next time
     */
    void reset();
protected:
    VariantDataChunk* pHeadChunk_; ///Posting list header
    VariantDataChunk* pTailChunk_; ///Posting list tail
    uint32_t nTotalSize_; ///Total size
    uint32_t nTotalUnused_; ///Total Unused size
    uint32_t nPosInCurChunk_;

    friend class InMemoryPosting;
    friend class PostingMerger;
};

}
NS_IZENELIB_IR_END

#endif
