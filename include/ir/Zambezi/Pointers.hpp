#ifndef POINTERS_H_GUARD
#define POINTERS_H_GUARD

#include "buffer/FixedCounter.hpp"
#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class Pointers
{
public:
    Pointers(uint32_t termNum, uint32_t docNum);
    ~Pointers();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    inline void setDocLen(uint32_t docid, uint32_t docLen)
    {
        docLen_.set(docid, docLen);
        totalDocLen_ += docLen;
    }

    inline void setMaxTf(uint32_t term, uint32_t tf, uint32_t dl)
    {
        maxTf_.set(term, tf);
        maxTfDocLen_.set(term, dl);
    }

    inline uint32_t nextTerm(uint32_t currentTermId) const
    {
        return headPointers_.nextIndex(currentTermId);
    }

private:
    void updateDefaultValues_();

public:
    uint32_t totalDocs_;
    size_t totalDocLen_;

    FixedCounter<uint32_t> df_;
    FixedCounter<size_t> cf_;
    FixedCounter<size_t> headPointers_;
    FixedCounter<uint32_t> docLen_;
    FixedCounter<uint32_t> maxTf_;
    FixedCounter<uint32_t> maxTfDocLen_;

    // Do not store
    uint32_t defaultDf_;
    float defaultIdf_;
    size_t defaultCf_;
};

}

NS_IZENELIB_IR_END

#endif
