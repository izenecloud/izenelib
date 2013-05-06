#ifndef IZENELIB_UTIL_SKETCH_KMV_H_
#define IZENELIB_UTIL_SKETCH_KMV_H_

#include "ICardinality.hpp"
#include <util/hashFunction.h>
#include <boost/unordered_set.hpp>
#include <queue>
#include <vector>
#include <stdint.h>
#include <iostream>

# if __WORDSIZE == 64
#  define __UINT64_C(c)	c ## UL
# else
#  define __UINT64_C(c)	c ## ULL
# endif

/* Limits of integral types.  */
/* Maximum of unsigned integral types.  */
# define UINT64_MAX		(__UINT64_C(18446744073709551615))

NS_IZENELIB_UTIL_BEGIN

template <typename ElemType>
class KMV : public ICardinality<ElemType>
{
public:
    typedef std::priority_queue<uint64_t, std::vector<uint64_t>, less<uint64_t> >  KMVSketchT;
    typedef ElemType DataTypeT;
    typedef ICardinality<DataTypeT> BaseType;
    typedef KMV<DataTypeT> ThisType;

    KMV(uint64_t seed, int kmv_k)
        :seed_(seed),
        kmv_k_(kmv_k)
    {
        exist_hashed_.rehash(kmv_k_*4);
    }

    size_t size() const
    {
        return sketch_.size();
    }

    void load(std::istream& is)
    {
        is.read((char*)&seed_, sizeof(seed_));
        is.read((char*)&kmv_k_, sizeof(kmv_k_));
        sketch_ = KMVSketchT();
        exist_hashed_.clear();
        for(size_t i = 0; i < (size_t)kmv_k_; ++i)
        {
            uint64_t data;
            is.read((char*)&data, sizeof(data));
            sketch_.push(data);
            exist_hashed_.insert(data);
        }
    }

    void save(std::ostream& os) const
    {
        os.write((const char*)&seed_, sizeof(seed_));
        os.write((const char*)&kmv_k_, sizeof(kmv_k_));
        assert(sketch_.size() == kmv_k_);
        KMVSketchT tmp_sketch = sketch_;
        while(!tmp_sketch.empty())
        {
            uint64_t data = tmp_sketch.top();
            os.write((const char*)&data, sizeof(data));
            tmp_sketch.pop();
        }
    }

    void updateSketch(const DataTypeT& data)
    {
        uint64_t v = izenelib::util::MurmurHash64A(&data,
            sizeof(DataTypeT), seed_);
        if(exist_hashed_.find(v) == exist_hashed_.end())
        {
            if(sketch_.size() < (size_t)kmv_k_)
            {
                sketch_.push(v);
                exist_hashed_.insert(v);
            }
            else if(v < sketch_.top())
            {
                sketch_.push(v);
                exist_hashed_.erase(sketch_.top());
                sketch_.pop();
                exist_hashed_.insert(v);
            }
        }
    }

    void fillKMVSketch()
    {
        while(sketch_.size() < (size_t)kmv_k_)
        {
            sketch_.push(UINT64_MAX);
        }
    }

    size_t getCardinate() const
    {
        if(sketch_.empty())
            return 0;
        uint64_t reduce_num = 0;
        uint64_t top = sketch_.top();
        if(top == UINT64_MAX)
        {
            KMVSketchT tmp_sketch = sketch_;
            while(!tmp_sketch.empty())
            {
                if(tmp_sketch.top() == UINT64_MAX)
                    reduce_num++;
                else
                    break;
                tmp_sketch.pop();
            }
            reduce_num--;
        }
        return (uint128_t(kmv_k_ - 1) * UINT64_MAX) / (uint128_t)top - reduce_num;
    }

    size_t intersectCard(const BaseType* src) const
    {
        const ThisType* kmv_src = dynamic_cast<const ThisType*>(src);
        if(kmv_src == NULL)
            throw -1;
        ThisType tmp_union = *this;
        tmp_union.unionSketch(src);
        KMVSketchT cursketch = sketch_;
        size_t K = 0;
        while(!cursketch.empty())
        {
            uint64_t top = cursketch.top();
            if(top != UINT64_MAX && (kmv_src->exist_hashed_.find(top) != kmv_src->exist_hashed_.end()))
            {
                if(tmp_union.exist_hashed_.find(top) != tmp_union.exist_hashed_.end())
                {
                    ++K;
                }
            }
            cursketch.pop();
        }
        return (double)K/min(size(), src->size())*(double)tmp_union.getCardinate();
    }

    size_t intersectSketchK(const BaseType* src)
    {
        const ThisType* kmv_src = dynamic_cast<const ThisType*>(src);
        if(kmv_src == NULL)
            throw -1;
        ThisType tmp_union = *this;
        tmp_union.unionSketch(src);
        KMVSketchT cursketch = sketch_;
        size_t K = 0;
        while(!cursketch.empty())
        {
            uint64_t top = cursketch.top();
            if(top != UINT64_MAX && (kmv_src->exist_hashed_.find(top) != kmv_src->exist_hashed_.end()))
            {
                if(tmp_union.exist_hashed_.find(top) != tmp_union.exist_hashed_.end())
                {
                    ++K;
                }
            }
            cursketch.pop();
        }
        return K;
    }

    void unionSketch(const BaseType* src)
    {
        if(src->size() == 0)
            return;

        const ThisType* kmv_src = dynamic_cast<const ThisType*>(src);
        if(kmv_src == NULL)
        {
            throw -1;
        }
        if(sketch_.empty())
        {
            exist_hashed_.clear();
            *this = *kmv_src;
            return;
        }
        KMVSketchT tmp_src;
        while(sketch_.size() > src->size())
        {
            exist_hashed_.erase(sketch_.top());
            sketch_.pop();
        }
        tmp_src = kmv_src->sketch_;
        while(!tmp_src.empty())
        {
            uint64_t top = tmp_src.top();
            if(top < sketch_.top())
            {
                exist_hashed_.insert(top);
                sketch_.push(top);
                exist_hashed_.erase(sketch_.top());
                sketch_.pop();
            }
            tmp_src.pop();
        }
        fillKMVSketch();
    }

private:

    uint64_t seed_;
    int kmv_k_;
    KMVSketchT sketch_;
    boost::unordered_set<uint64_t> exist_hashed_;
};

NS_IZENELIB_UTIL_END

#endif
