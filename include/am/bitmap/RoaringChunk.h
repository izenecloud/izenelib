#ifndef IZENELIB_AM_BITMAP_ROARING_CHUNK_H
#define IZENELIB_AM_BITMAP_ROARING_CHUNK_H

#include "consts.h"
#include <util/mem_utils.h>
#include <util/izene_serialization.h>
#include <3rdparty/msgpack/msgpack.hpp>

#include <boost/shared_array.hpp>
#include <boost/atomic.hpp>

NS_IZENELIB_AM_BEGIN

class RoaringChunk
{
public:
    typedef RoaringChunk self_type;
    typedef boost::shared_array<uint32_t> data_type;

    enum Type
    {
        ARRAY,
        BITMAP,
        EMPTY,
        FULL,
        TYPE_END
    };

    RoaringChunk(uint32_t key = 0);
    virtual ~RoaringChunk();

    RoaringChunk(const self_type& b);
    self_type& operator=(const self_type& b);

    bool operator==(const self_type& b) const;

    void init(uint32_t key, uint32_t value);
    void trim();
    void add(uint32_t x);
    bool contains(uint32_t x) const;

    void resetChunk(Type type, uint32_t capacity);
    void cloneChunk(const data_type& chunk);
    void atomicCopy(const self_type& b);

    data_type getChunk() const;

    self_type operator~() const;
    self_type operator&(const self_type& b) const;
    self_type operator|(const self_type& b) const;
    self_type operator^(const self_type& b) const;
    self_type operator-(const self_type& b) const;

    uint32_t getKey() const
    {
        return key_;
    }

    uint32_t getCardinality() const
    {
        return chunk_ ? chunk_[1] : full_ ? 65536 : 0;
    }

private:
    void not_Bitmap(const data_type& a);
    void not_Array(const data_type& a);

    void and_BitmapBitmap(const data_type& a, const data_type& b);
    void and_BitmapArray(const data_type& a, const data_type& b);
    void and_ArrayArray(const data_type& a, const data_type& b);

    void or_BitmapBitmap(const data_type& a, const data_type& b);
    void or_BitmapArray(const data_type& a, const data_type& b);
    void or_ArrayArray(const data_type& a, const data_type& b);

    void xor_BitmapBitmap(const data_type& a, const data_type& b);
    void xor_BitmapArray(const data_type& a, const data_type& b);
    void xor_ArrayArray(const data_type& a, const data_type& b);

    void andNot_BitmapBitmap(const data_type& a, const data_type& b);
    void andNot_BitmapArray(const data_type& a, const data_type& b);
    void andNot_ArrayBitmap(const data_type& a, const data_type& b);
    void andNot_ArrayArray(const data_type& a, const data_type& b);

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & key_ & updatable_;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & key_ & updatable_;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    uint32_t key_;

    bool updatable_;
    bool full_;
    mutable boost::atomic_flag flag_;

    data_type chunk_;
};

NS_IZENELIB_AM_END

#endif
