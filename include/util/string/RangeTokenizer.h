#ifndef STRING_RANGE_TOKENIZER_H
#define STRING_RANGE_TOKENIZER_H
/**
 * @file string/RangeTokenizer.h
 * @author Ian Yang
 * @date Created <2010-01-29 11:33:23>
 * @date Updated <2010-01-29 11:57:47>
 * @brief Helper class to create RangeTokenIterator
 */

#include "RangeTokenIterator.h"

namespace izenelib {namespace util{

template<typename TokenizerFunc, typename Iterator>
class RangeTokenizer
{
public:
    typedef RangeTokenIterator<TokenizerFunc, Iterator> iterator;
    typedef iterator const_iterator;
    typedef std::pair<Iterator, Iterator> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    RangeTokenizer(Iterator first,
                   Iterator last,
                   const TokenizerFunc& func = TokenizerFunc())
    : first_(first), last_(last), func_(func)
    {}

    template<typename Container>
    explicit RangeTokenizer(Container& c,
                            const TokenizerFunc& func = TokenizerFunc())
    : first_(c.begin()), last_(c.end()), func_(func)
    {}
    template<typename Container>
    explicit RangeTokenizer(const Container& c,
                            const TokenizerFunc& func = TokenizerFunc())
    : first_(c.begin()), last_(c.end()), func_(func)
    {}

    void assign(Iterator first, Iterator last)
    {
        first_ = first;
        last_ = last;
    }

    void assign(Iterator first, Iterator last, const TokenizerFunc& func)
    {
        first_ = first;
        last_ = last;
        func_ = func;
    }

    template<typename Container>
    void assign(Container& c)
    {
        assign(c.begin(), c.end());
    }
    template<typename Container>
    void assign(const Container& c)
    {
        assign(c.begin(), c.end());
    }

    template<typename Container>
    void assign(Container& c, const TokenizerFunc& func)
    {
        assign(c.begin(), c.end(), func);
    }
    template<typename Container>
    void assign(const Container& c, const TokenizerFunc& func)
    {
        assign(c.begin(), c.end(), func);
    }

    iterator begin() const
    {
        return iterator(func_, first_, last_);
    }
    iterator end() const
    {
        return iterator(func_, last_, last_);
    }

    bool empty() const
    {
        return begin() == end();
    }

private:
    Iterator first_;
    Iterator last_;
    TokenizerFunc func_;
};

}} // namespace izenelib::util

#endif // STRING_RANGE_TOKENIZER_H
