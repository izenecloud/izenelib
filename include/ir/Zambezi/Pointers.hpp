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
    Pointers(uint32_t size);
    ~Pointers();

    void save(std::ostream& ostr) const;
    void load(std::istream& istr);

    inline uint32_t getTotalDocs() const
    {
        return totalDocs_;
    }

    inline size_t getTotalDocLen() const
    {
        return totalDocLen_;
    }

    inline uint32_t getDefaultDf() const
    {
        return defaultDf_;
    }

    inline float getDefaultIdf() const
    {
        return defaultIdf_;
    }

    inline size_t getDefaultCf() const
    {
        return defaultCf_;
    }

    inline uint32_t getDf(uint32_t term) const
    {
        return df_.get(term);
    }

    inline void setDf(uint32_t term, uint32_t df)
    {
        df_.set(term, df);
    }

    inline size_t getCf(uint32_t term) const
    {
        return cf_.get(term);
    }

    inline void setCf(uint32_t term, size_t cf)
    {
        cf_.set(term, cf);
    }

    inline uint32_t getDocLen(uint32_t docid) const
    {
        return docLen_.get(docid);
    }

    inline void setDocLen(uint32_t docid, uint32_t docLen)
    {
        docLen_.set(docid, docLen);
        totalDocLen_ += docLen;
        ++totalDocs_;
    }

    inline uint32_t getMaxTf(uint32_t term) const
    {
        return maxTf_.get(term);
    }

    inline uint32_t getMaxTfDocLen(uint32_t term) const
    {
        return maxTfDocLen_.get(term);
    }

    inline void setMaxTf(uint32_t term, uint32_t tf, uint32_t dl)
    {
        maxTf_.set(term, tf);
        maxTfDocLen_.set(term, dl);
    }

    inline size_t getHeadPointer(uint32_t term) const
    {
        return headPointers_.get(term);
    }

    inline void setHeadPointer(uint32_t term, size_t sp)
    {
        headPointers_.set(term, sp);
    }

    inline uint32_t nextTerm(uint32_t currentTermId) const
    {
        return headPointers_.nextIndex(currentTermId);
    }

private:
    void updateDefaultValues_();

private:
    friend class InvertedIndex;
    friend class NewInvertedIndex;

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
