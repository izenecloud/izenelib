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

    enum Type
    {
        ARRAY,
        BITMAP,
        FULL,
        DEFAULT
    };

    RoaringChunk();
    RoaringChunk(uint32_t key, uint32_t value);
    RoaringChunk(uint32_t key, Type type, uint32_t capacity);
    virtual ~RoaringChunk();

    RoaringChunk(const RoaringChunk& b, bool clone = false);
    self_type& operator=(const self_type& b);

    bool operator==(const self_type& b) const;

    void add(uint32_t x);
//  void trim();

    boost::shared_array<uint32_t> getChunk() const;

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
        return cardinality_;
    }

    void unsetUpdatable()
    {
        updatable_ = false;
    }

private:
    self_type and_BitmapBitmap(const self_type& b) const;
    self_type and_BitmapArray(const self_type& b) const;
    self_type and_ArrayArray(const self_type& b) const;

    self_type or_BitmapBitmap(const self_type& b) const;
    self_type or_BitmapArray(const self_type& b) const;
    self_type or_ArrayArray(const self_type& b) const;

    self_type xor_BitmapBitmap(const self_type& b) const;
    self_type xor_BitmapArray(const self_type& b) const;
    self_type xor_ArrayArray(const self_type& b) const;

    self_type andNot_BitmapBitmap(const self_type& b) const;
    self_type andNot_BitmapArray(const self_type& b) const;
    self_type andNot_ArrayBitmap(const self_type& b) const;
    self_type andNot_ArrayArray(const self_type& b) const;

    void convertFromBitmapToArray();

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
    uint32_t cardinality_;

    bool updatable_;
    mutable boost::atomic_flag flag_;

    boost::shared_array<uint32_t> chunk_;
};

NS_IZENELIB_AM_END

#endif
