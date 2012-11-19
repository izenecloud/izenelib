#ifndef IZENELIB_UTIL_SKETCH_HYPERLOGLOG_H_
#define IZENELIB_UTIL_SKETCH_HYPERLOGLOG_H_

#include "ICardinality.hpp"
#include <util/hashFunction.h>
#include <util/math.h>
#include <common/type_defs.h>
#include <vector>
#include <stdint.h>
#include <math.h>

NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class HyperLL : public ICardinality<ElemType>
{
public:
    typedef std::vector<uint8_t>  HyperLLSketchT;
    typedef ElemType DataTypeT;
    typedef ICardinality<DataTypeT> BaseType;
    typedef HyperLL<DataTypeT> ThisType;

    HyperLL(uint64_t seed, int k)
        : seed_(seed), 
        hll_k_(k)
    {
        assert(hll_k_ <= 16 && hll_k_ >=4);
        int m = pow(2, hll_k_);
        if(hll_k_ == 4)
            alphaMM_ = 0.673 * m * m;
        else if(hll_k_ == 5)
            alphaMM_ = 0.697 * m * m;
        else if(hll_k_ == 6)
            alphaMM_ = 0.709 * m * m;
        else
            alphaMM_ = (0.7213 / (1 + 1.079 / m)) * m * m;
        sketch_.assign(m, 0);
    }

    size_t size() const
    {
        return sketch_.size();
    }

    void updateSketch(const DataTypeT& data)
    {
        uint64_t v = izenelib::util::MurmurHash64A(&data,
            sizeof(DataTypeT), seed_);
        uint32_t index = getBucketIndex(v);
        uint8_t zero_rank = rankZero(v);
        assert(zero_rank <= 64 - hll_k_);
        sketch_[index] = max(sketch_[index], zero_rank);
    }


    size_t getCardinate() const
    {
        if(sketch_.empty())
            return 0;
        double sum = 0;
        size_t cnt = sketch_.size();
        for(size_t i = 0; i < cnt; ++i)
        {
            sum += pow(2, -1*sketch_[i]);
        }
        double estimate = alphaMM_ / sum;
        if( estimate <= (5.0/2.0)*cnt )
        {
            int zeros = 0;
            for(size_t i = 0; i < cnt; ++i)
            {
                if(sketch_[i] == 0)
                    zeros++;
            }
            if(zeros == 0)
                return estimate;
            return cnt * fastlog(cnt/(double)zeros);
        }
        else if( estimate <= (1.0/30.0)*POW_2_32)
        {
            return (size_t)estimate;
        }
        else if(estimate > (1.0/30.0) * POW_2_32)
        {
            return (size_t)(NEGATIVE_POW_2_32 * fastlog(1 - estimate/POW_2_32));
        }
        return 0;
    }

    void unionSketch(const BaseType* src)
    {
        if(src->size() == 0)
            return;

        const ThisType* hll_src = dynamic_cast<const ThisType*>(src);
        if(hll_src == NULL)
        {
            throw -1;
        }
        if(sketch_.empty())
        {
            *this = *hll_src;
            return;
        }
        assert(src->size() == size());
        const HyperLLSketchT& tmp_src = hll_src->sketch_;
        for(size_t i = 0; i < sketch_.size(); ++i)
        {
            sketch_[i] = max(sketch_[i], tmp_src[i]);
        }
    }

private:
    inline uint32_t getBucketIndex(uint64_t hashvalue)
    {
        // use the first hll_k_ bits to generate the index.
        return hashvalue >> (64 - hll_k_);
    }

    inline uint8_t rankZero(uint64_t hashvalue)
    {
        return leadingZero(((hashvalue << hll_k_) | ((uint64_t)1 << (hll_k_ - 1))) + 1) + 1;
    }

    inline uint8_t leadingZero(uint64_t i)
    {
        if (i == 0)
            return 64;
        uint8_t n = 0;
        uint32_t x = (uint32_t)(i >> 32);
        if (x == 0) { n += 32; x = (uint32_t)i; }
        if (x >> 16 == 0) { n += 16; x <<= 16; }
        if (x >> 24 == 0) { n +=  8; x <<=  8; }
        if (x >> 28 == 0) { n +=  4; x <<=  4; }
        if (x >> 30 == 0) { n +=  2; x <<=  2; }
        if (x >> 31 == 0) { n +=  1; x <<=  1; }
        return n;
    }

    // the bits used for bucket index.
    uint64_t seed_;
    int hll_k_;
    double alphaMM_;
    HyperLLSketchT sketch_;
    static const double POW_2_32 = 4294967296.0 ; 
    static const double NEGATIVE_POW_2_32 = -4294967296.0; 
};

NS_IZENELIB_UTIL_END

#endif
