#ifndef IZENELIB_IR_ZAMBEZI_BUFFER_FIXED_COUNTER_HPP
#define IZENELIB_IR_ZAMBEZI_BUFFER_FIXED_COUNTER_HPP

#include <types.h>

#include <vector>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

template <class T>
class FixedCounter
{
public:
    FixedCounter()
        : defaultValue_(T())
    {
    }

    FixedCounter(uint32_t initialSize, T defaultValue = T())
        : defaultValue_(defaultValue)
        , counter_(initialSize, defaultValue)
    {
    }

    bool expand(uint32_t newSize)
    {
        if (newSize <= counter_.size())
            return false;

        size_t newCap = counter_.capacity();

        if (newCap == 0)
        {
            newCap = newSize;
        }
        else
        {
            do
            {
                newCap *= 2;
            }
            while (newSize > newCap);
        }

        counter_.resize(newCap, defaultValue_);

        return true;
    }

    uint32_t size() const
    {
        uint32_t nbElements = 0;
        for (uint32_t i = 0; i < counter_.size(); i++)
        {
            if (counter_[i] != defaultValue_)
            {
                ++nbElements;
            }
        }

        return nbElements;
    }

    const T& get(uint32_t index) const
    {
        if (index >= counter_.size())
            return defaultValue_;
        else
            return counter_[index];
    }

    void add(uint32_t index, T c)
    {
        expand(index + 1);
        counter_[index] += c;
    }

    void increment(uint32_t index)
    {
        expand(index + 1);
        ++counter_[index];
    }

    void set(uint32_t index, T c)
    {
        expand(index + 1);
        counter_[index] = c;
    }

    void reset(uint32_t index)
    {
        if (!expand(index + 1))
            counter_[index] = defaultValue_;
    }

    uint32_t nextIndex(uint32_t pos) const
    {
        do
        {
            if (++pos >= counter_.size())
            {
                return -1;
            }
        }
        while (counter_[pos] == defaultValue_);

        return pos;
    }

    T defaultValue_;
    std::vector<T> counter_;
};

}

NS_IZENELIB_IR_END

#endif
