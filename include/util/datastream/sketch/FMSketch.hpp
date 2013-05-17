#ifndef IZENELIB_UTIL_SKETCH_FMSKETCH_H_
#define IZENELIB_UTIL_SKETCH_FMSKETCH_H_

#include "ICardinality.hpp"
#include "Level2Sketch.hpp"
#include <util/hashFunction.h>
#include <vector>
#include <stdint.h>
#include <math.h>

#define BITSET_SIZE 64

NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class FMSketch : public ICardinality<ElemType>
{
public:
    typedef std::vector<std::bitset<BITSET_SIZE> >  FMSketchT;
    typedef ElemType DataTypeT;
    typedef ICardinality<DataTypeT> BaseType;
    typedef FMSketch<DataTypeT> ThisType;
    typedef Level2Sketch<DataTypeT>  Level2SketchT;

    FMSketch(uint64_t seed, int k)
        : seed_(seed),
        fm_k_((size_t)k)
    {
        sketch_.resize(fm_k_);
        level2sketches_.resize(fm_k_);
        for(size_t i = 0; i < (size_t)fm_k_; ++i)
            level2sketches_[i].resize(BITSET_SIZE);
    }

    size_t size() const
    {
        return sketch_.size();
    }

    void load(std::istream& is)
    {
        is.read((char*)&seed_, sizeof(seed_));
        is.read((char*)&fm_k_, sizeof(fm_k_));
        FMSketchT().swap(sketch_);
        sketch_.reserve(fm_k_);
        level2sketches_.resize(fm_k_);
        for(size_t i = 0; i < fm_k_; ++i)
        {
            char data[BITSET_SIZE + 1];
            is.read((char*)&data, BITSET_SIZE);
            data[BITSET_SIZE] = '\0';
            sketch_.push_back(std::bitset<BITSET_SIZE>(std::string(data)));
            level2sketches_[i].load(is);
        }
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&seed_, sizeof(seed_));
        os.write((const char*)&fm_k_, sizeof(fm_k_));
        for(size_t i = 0; i < fm_k_; ++i)
        {
            os.write((const char*)sketch_[i].to_string().data(), BITSET_SIZE);
            level2sketches_[i].save(os);
        }
    }

    void updateSketch(const DataTypeT& data)
    {
        uint64_t v = izenelib::util::MurmurHash64A(&data,
            sizeof(DataTypeT), seed_);
        uint8_t lsb = LSB(v/fm_k_);
        sketch_[v % fm_k_].set(lsb);
        level2sketches_[v % fm_k_].updateBucket(lsb, data);
    }

    size_t setUnionEstimator(const ThisType& src) const
    {
        double f = (1 + E) * (double)fm_k_ / 8;
        size_t index = 0;
        size_t count = 0;
        while(index < BITSET_SIZE)
        {
            count = 0;
            for(size_t i = 0; i < fm_k_; ++i)
            {
                if(!level2sketches_[i].emptyBucket(index) ||
                    !src.level2sketches_[i].emptyBucket(index))
                {
                    count++;
                }
            }
            if(count <= f)
                break;
            else
                index++;
        }
        double p = count/(double)fm_k_;
        double R = pow(2, index + 1);
        return log(1 - p)/log(1 - 1/R) * fm_k_;
    }

    size_t setIntersectEstimator(const ThisType& src) const
    {
        if(src.size() == 0 || size() == 0)
            return 0;

        assert(src.size() == size());
        size_t sum = 0;
        size_t count = 0;
        size_t union_card = setUnionEstimator(src);
        for(size_t i = 0; i < fm_k_; ++i)
        {
            //size_t index = std::ceil(std::log(2*union_card/(1 - E)));
            size_t index = std::ceil(std::log(2 * (double)union_card / (fm_k_ * (1 - E) * (1 - E))));
            if(index > BITSET_SIZE)
                continue;
            int atomic_estimate = level2sketches_[i].atomicIntersectEstimator(index, src.level2sketches_[i]);
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

    size_t intersectCard(const BaseType* src) const
    {
        const ThisType* fm_src = dynamic_cast<const ThisType*>(src);
        if(fm_src == NULL)
        {
            throw -1;
        }

        return setIntersectEstimator(*fm_src);
    }

    size_t getCardinate() const
    {
        if(sketch_.empty())
            return 0;
        int sum = 0;
        for(size_t i = 0; i < fm_k_; ++i)
        {
            int leftmost_zero = BITSET_SIZE;
            for(int m = BITSET_SIZE - 1; m >= 0; --m)
            {
                if(!sketch_[i].test(m))
                    leftmost_zero = m;
            }
            sum += leftmost_zero;
        }
        return 1.2928 * pow(2, sum/(double)fm_k_) * fm_k_;
    }

    void unionSketch(const BaseType* src)
    {
        if(src->size() == 0)
            return;

        const ThisType* fm_src = dynamic_cast<const ThisType*>(src);
        if(fm_src == NULL)
        {
            throw -1;
        }
        if(sketch_.empty())
        {
            *this = *fm_src;
            return;
        }
        assert(src->size() == size());
        const FMSketchT& tmp_src = fm_src->sketch_;
        for(size_t i = 0; i < sketch_.size(); ++i)
        {
            sketch_[i] |= tmp_src[i];
            level2sketches_[i].unionLevel2Sketch(fm_src->level2sketches_[i]);
        }
    }

    static size_t setUnionEstimator(const std::vector<ThisType>& sketches)
    {
        if(sketches.empty())
            return 0;
        size_t fm_k = sketches[0].fm_k_;
        double f = (1 + E) * (double)fm_k / 8;
        size_t index = 0;
        std::vector<Level2SketchT> tmp_level2sketches;
        tmp_level2sketches.resize(sketches.size());
        size_t count = 0;
        while(index < BITSET_SIZE)
        {
            count = 0;
            for(size_t i = 0; i < fm_k; ++i)
            {
                for(size_t j = 0; j < sketches.size(); ++j)
                {
                    tmp_level2sketches[j] = sketches[j].level2sketches_[i];
                }
                if(atomicUnionBucketEstimator(index, tmp_level2sketches))
                {
                    count++;
                }
            }
            if(count <= f)
                break;
            else
                index++;
        }
        double p = count/(double)fm_k;
        double R = pow(2, index + 1);
        return log(1 - p)/log(1 - 1/R) * fm_k;
    }

    static size_t setUnionWithIntersectEstimator(const std::vector<ThisType>& union_sketch,
        const std::vector<ThisType>& filter_sketch)
    {
        if(union_sketch.empty() || filter_sketch.empty())
            return 0;
        size_t sum = 0;
        size_t count = 0;
        size_t fm_k = union_sketch[0].fm_k_;
        std::vector<ThisType> all_sketches = union_sketch;
        all_sketches.insert(all_sketches.end(), filter_sketch.begin(), filter_sketch.end());
        size_t union_card = setUnionEstimator(all_sketches);

        std::vector<Level2SketchT> tmp_level2sketches;
        tmp_level2sketches.resize(union_sketch.size());
        std::vector<Level2SketchT> tmpfilter_level2sketches;
        tmpfilter_level2sketches.resize(filter_sketch.size());

        for(size_t i = 0; i < fm_k; ++i)
        {
            //size_t index = std::ceil(std::log(2*union_card/(1 - E)));
            size_t index = std::ceil(std::log(2 * (double)union_card / ( fm_k * (1 - E) * (1 - E) )));
            if(index > BITSET_SIZE)
                continue;
            for(size_t j = 0; j < union_sketch.size(); ++j)
            {
                tmp_level2sketches[j] = union_sketch[j].level2sketches_[i];
            }
            for(size_t j = 0; j < filter_sketch.size(); ++j)
            {
                tmpfilter_level2sketches[j] = filter_sketch[j].level2sketches_[i];
            }
            int atomic_estimate = atomicUnionWithIntersectBucketEstimator(index, tmp_level2sketches,
                tmpfilter_level2sketches);
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


private:
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
    size_t fm_k_;
    FMSketchT sketch_;
    std::vector<Level2SketchT> level2sketches_;
    static const double E = 0.618;
};

NS_IZENELIB_UTIL_END

#endif
