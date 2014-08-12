#ifndef IZENELIB_AM_BITMAP_ROARING_BITMAP_H
#define IZENELIB_AM_BITMAP_ROARING_BITMAP_H

#include <types.h>
#include <util/mem_utils.h>
#include <util/izene_serialization.h>
#include <3rdparty/msgpack/msgpack.hpp>

#include <boost/shared_array.hpp>

NS_IZENELIB_AM_BEGIN

class RoaringChunk;

class RoaringBitmap
{
public:
    typedef RoaringBitmap self_type;
    typedef RoaringChunk chunk_type;
    typedef boost::shared_array<chunk_type> array_type;

    RoaringBitmap(bool updatable = true);
    virtual ~RoaringBitmap();

    RoaringBitmap(const self_type& b);
    self_type& operator=(const self_type& b);

    bool operator==(const self_type& b) const;

    void add(uint32_t x);

    array_type getArray() const;

    self_type operator&(const self_type& b) const;
    self_type operator|(const self_type& b) const;
    self_type operator^(const self_type& b) const;
    self_type operator-(const self_type& b) const;

    void swap(self_type& b);

    size_t getCardinality() const
    {
        return cardinality_;
    }

private:
    void extendArray(uint32_t k);

    void append(const chunk_type& value);
    void appendCopy(const array_type& sa, uint32_t index);
    void appendCopy(const array_type& sa, uint32_t begin, uint32_t end);

    uint32_t getIndex(uint32_t x);

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & capacity_ & size_ & cardinality_;
        if (cardinality_)
        {
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & capacity_ & size_ & cardinality_;
        if (cardinality_)
        {
        }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    uint32_t capacity_;
    uint32_t size_;
    uint64_t cardinality_;

    bool updatable_;
    mutable boost::atomic_flag flag_;

    array_type array_;
};

NS_IZENELIB_AM_END

#endif
