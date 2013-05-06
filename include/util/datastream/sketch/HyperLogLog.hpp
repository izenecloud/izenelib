#ifndef IZENELIB_UTIL_SKETCH_HYPERLOGLOG_H_
#define IZENELIB_UTIL_SKETCH_HYPERLOGLOG_H_

#include "ICardinality.hpp"
#include "Level2Sketch.hpp"
#include <util/hashFunction.h>
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
    typedef Level2Sketch<DataTypeT>  Level2SketchT;

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
        level2sketches_.resize(m);
        for(size_t i = 0; i < (size_t)m; ++i)
            level2sketches_[i].resize(sizeof(uint64_t)*8);
    }

    size_t size() const
    {
        return sketch_.size();
    }

    void load(std::istream& is)
    {
        is.read((char*)&seed_, sizeof(seed_));
        is.read((char*)&hll_k_, sizeof(hll_k_));
        is.read((char*)&alphaMM_, sizeof(alphaMM_));
        HyperLLSketchT().swap(sketch_);
        int m = pow(2, hll_k_);
        sketch_.reserve(m);
        level2sketches_.resize(m);
        for(size_t i = 0; i < (size_t)m; ++i)
        {
            uint8_t data;
            is.read((char*)&data, sizeof(data));
            sketch_.push_back(data);
            level2sketches_[i].load(is);
        }
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&seed_, sizeof(seed_));
        os.write((const char*)&hll_k_, sizeof(hll_k_));
        os.write((const char*)&alphaMM_, sizeof(alphaMM_));
        for(size_t i = 0; i < sketch_.size(); ++i)
        {
            uint8_t data = sketch_[i];
            os.write((const char*)&data, sizeof(data));
            level2sketches_[i].save(os);
        }
    }

    void updateSketch(const DataTypeT& data)
    {
        uint64_t v = izenelib::util::MurmurHash64A(&data,
            sizeof(DataTypeT), seed_);
        uint32_t index = getBucketIndex(v);
        uint8_t zero_rank = rankZero(v);
        assert(zero_rank <= 64 - hll_k_);
        sketch_[index] = max(sketch_[index], zero_rank);
        level2sketches_[v % level2sketches_.size()].updateBucket(LSB(v / level2sketches_.size()), data);
    }

    size_t intersectCard(const BaseType* src) const
    {
        if(src->size() == 0 || size() == 0)
            return 0;

        const ThisType* hll_src = dynamic_cast<const ThisType*>(src);
        if(hll_src == NULL)
        {
            throw -1;
        }
        assert(src->size() == size());
        size_t sum = 0;
        size_t count = 0;
        ThisType tmp_union = *this;
        tmp_union.unionSketch(src);
        size_t union_card = tmp_union.getCardinate();
        for(size_t i = 0; i < level2sketches_.size(); ++i)
        {
            //size_t index = std::ceil(std::log(2*union_card/(1 - E)));
            size_t index = std::ceil(std::log(2*union_card/(level2sketches_.size()*(1 - E)*(1 - E))));
            if(index > level2sketches_[i].size())
                continue;
            int atomic_estimate = level2sketches_[i].atomicIntersectEstimator(index, hll_src->level2sketches_[i]);
            if(atomic_estimate != -1)
            {
                sum += atomic_estimate;
                count++;
            }
        }
        if(count == 0)
            return 0;
        return sum*union_card/count;
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
            return cnt * log(cnt/(double)zeros);
        }
        else if( estimate <= (1.0/30.0)*POW_2_32)
        {
            return (size_t)estimate;
        }
        else if(estimate > (1.0/30.0) * POW_2_32)
        {
            return (size_t)(NEGATIVE_POW_2_32 * log(1 - estimate/POW_2_32));
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
            level2sketches_[i].unionLevel2Sketch(hll_src->level2sketches_[i]);
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

    inline uint8_t LSB(uint64_t v)
    {
        for(uint8_t i = 0; i < 64; ++i)
        {
            if((v & ((uint64_t)1 << i)) != 0)
                return i;
        }
        return 63;
    }

    // the bits used for bucket index.
    uint64_t seed_;
    int hll_k_;
    double alphaMM_;
    HyperLLSketchT sketch_;
    std::vector<Level2SketchT> level2sketches_;
    static const double POW_2_32 = 4294967296.0 ;
    static const double NEGATIVE_POW_2_32 = -4294967296.0;
    static const double E = 0.618;
};

NS_IZENELIB_UTIL_END

#endif
