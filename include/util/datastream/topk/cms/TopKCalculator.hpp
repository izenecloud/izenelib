/*
 *  @file    TopKCalculator.hpp
 *  @author  Kuilong Liu
 *  @date    2013.04.25
 *  @        for TopK frequent items
 *  @        in dependence of "madoka" (a realize of count min sketch)
 *
 *           The type of element had better to be string, that depends on madoka
 *
 *           Each operation (insert, getTopK) is O(1)
 */

#ifndef IZENELIB_UTIL_TOPK_CALCULATOR_H_
#define IZENELIB_UTIL_TOPK_CALCULATOR_H_

#include "TopKItemEstimation.hpp"
#include <util/datastream/sketch/madoka/sketch.h>
#include <list>
#include <string>
#include <stdint.h>
#include <map>

namespace izenelib { namespace util {

template<typename ElemType, typename CountType>
class TopKCalculator
{
public:
    typedef TopKEstimation<ElemType, CountType> TopKEsT;
    TopKCalculator(CountType sw, uint64_t mv, CountType tm)
        :sketch_width_(sw), sketch_maxvalue_(mv), TOPK_MAXCOUNT_(tm)
    {
        sketch_ = new madoka::Sketch();
        sketch_->create(sw);
        tke_ = new TopKEsT(tm);
    }
    ~TopKCalculator()
    {
        if(sketch_)delete sketch_;
        if(tke_)delete tke_;
    }

    bool reset()
    {
        sketch_->clear();
        return tke_->reset();
    }

    bool infomation(std::map<std::string, uint64_t>& inf)
    {
        inf["sketch width: "] = sketch_->width();
        inf["sketch depth: "] = sketch_->depth();
        inf["sketch max value: "] = sketch_->max_value();
        inf["sketch table size: "] = sketch_->table_size();
        inf["topk max count: "] = TOPK_MAXCOUNT_;
        return true;
    }

    bool insert(ElemType elem)
    {
        //the elem type need to be string
        CountType count = sketch_->inc(elem.c_str(), elem.length());
        return tke_->insert(elem, count);
    }

    bool getTopK(std::list<std::pair<ElemType, CountType> >& topk)
    {
        return tke_->get(topk);
    }

    bool getTopK(CountType k, std::list<std::pair<ElemType, CountType> >& topk)
    {
        return tke_->get(k, topk);
    }

    madoka::Sketch* getSketch()
    {
        return sketch_;
    }
private:
    CountType sketch_width_;
    uint64_t sketch_maxvalue_;
    std::string sketch_path_;

    CountType TOPK_MAXCOUNT_;

    madoka::Sketch* sketch_;
    TopKEsT* tke_;
};

} //end of namespace util
} //end of namespace izenelib
#endif
