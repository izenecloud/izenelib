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

    inline void setDocLen(uint32_t docid, uint32_t len)
    {
        docLen.set(docid, len);
        totalDocLen += len;
    }

    inline void setMaxTf(uint32_t term, uint32_t tf, uint32_t dl)
    {
        maxTf.set(term, tf);
        maxTfDocLen.set(term, dl);
    }

    inline uint32_t nextTerm(uint32_t currentTermId) const
    {
        return headPointers.nextIndex(currentTermId);
    }

private:
    void updateDefaultValues_();

public:
    uint32_t totalDocs;
    size_t totalDocLen;

    FixedCounter<uint32_t> df;
    FixedCounter<size_t> cf;
    FixedCounter<size_t> headPointers;
    FixedCounter<uint32_t> maxTf;
    FixedCounter<uint32_t> maxTfDocLen;

    FixedCounter<uint32_t> docLen;

    // Do not store
    uint32_t defaultDf;
    float defaultIdf;
    size_t defaultCf;
};

}

NS_IZENELIB_IR_END

#endif
