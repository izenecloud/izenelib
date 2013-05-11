#ifndef IZENELIB_UTIL_SKETCH_LinearCounting_H_
#define IZENELIB_UTIL_SKETCH_LinearCounting_H_

#include "ICardinality.hpp"
#include <util/hashFunction.h>
#include <bitset>
#include <stdint.h>
#include <math.h>

#define BITSIZE  1000000
NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class LinearCounting : public ICardinality<ElemType>
{
public:
    typedef std::bitset<BITSIZE>  LinearCountingSketchT;
    typedef ElemType DataTypeT;
    typedef ICardinality<DataTypeT> BaseType;
    typedef LinearCounting<DataTypeT> ThisType;

    LinearCounting(uint64_t seed, int lc_k)
        :seed_(seed),
        lc_k_(lc_k)
    {
    }

    size_t size() const
    {
        return sketch_.size();
    }

    void load(std::istream& is)
    {
        is.read((char*)&seed_, sizeof(seed_));
        is.read((char*)&lc_k_, sizeof(lc_k_));
        uint32_t bitsize = 0;
        is.read((char*)&bitsize, sizeof(bitsize));
        if(bitsize != BITSIZE)
        {
            cout << "the bitset data size is not compatible with the current size." << endl;
            return;
        }
        char *data = new char[BITSIZE + 1];
        is.read(data, BITSIZE);
        data[BITSIZE] = '\0';
        sketch_ = LinearCountingSketchT(std::string(data));
        delete[] data;
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&seed_, sizeof(seed_));
        os.write((const char*)&lc_k_, sizeof(lc_k_));
        const uint32_t bitsize = BITSIZE;
        os.write((const char*)&bitsize, sizeof(bitsize));
        os.write(sketch_.to_string().data(), BITSIZE);
    }

    void updateSketch(const DataTypeT& data)
    {
        uint64_t v = izenelib::util::MurmurHash64A(&data,
            sizeof(DataTypeT), seed_);
        uint32_t bit = (v & 0xFFFFFFFF) % BITSIZE;
        sketch_.set(bit);
    }

    size_t intersectCard(const BaseType* src) const
    {
        size_t dv = getCardinate();
        size_t filter_dv = src->getCardinate();
        const ThisType* lc_src = dynamic_cast<const ThisType*>(src);
        if(lc_src == NULL)
        {
            throw -1;
        }
        ThisType tmp_union = *lc_src;
        tmp_union.unionSketch(src);
        size_t union_dv = tmp_union.getCardinate();
        if(union_dv > dv + filter_dv)
            return 0;
        return dv + filter_dv - union_dv;
    }

    size_t getCardinate() const
    {
        size_t zero_num = sketch_.size() - sketch_.count();
        return -BITSIZE * log((double)zero_num/BITSIZE);
    }

    void unionSketch(const BaseType* src)
    {
        assert(sketch_.size() == src->size());
        const ThisType* lc_src = dynamic_cast<const ThisType*>(src);
        if(lc_src == NULL)
        {
            throw -1;
        }
        const LinearCountingSketchT& tmp_src = lc_src->sketch_;
        if(tmp_src.count() == 0)
            return;
        sketch_ |= tmp_src;
    }

private:

    uint64_t seed_;
    int lc_k_;
    LinearCountingSketchT sketch_;
};

NS_IZENELIB_UTIL_END

#endif
