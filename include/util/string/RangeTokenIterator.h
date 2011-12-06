#ifndef INCLUDE_STRING_RANGE_TOKEN_ITERATOR_H
#define INCLUDE_STRING_RANGE_TOKEN_ITERATOR_H
/**
 * @file include/string/RangeTokenIterator.h
 * @author Ian Yang
 * @date Created <2010-01-25 18:04:46>
 * @date Updated <2010-01-29 11:38:39>
 * @brief tokenizer similar to \c boost::tokenizer. This tokenizer returns pair
 * of iterators instead.
 */

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/detail/minimum_category.hpp>

namespace izenelib {namespace util{

template<typename TokenizerFunc, typename Iterator>
class RangeTokenIterator
: public boost::iterator_facade<
    RangeTokenIterator<TokenizerFunc, Iterator>,
    std::pair<Iterator, Iterator>,
    typename boost::detail::minimum_category<
        boost::forward_traversal_tag,
        typename boost::iterator_traversal<Iterator>::type
    >::type,
    const std::pair<Iterator, Iterator>&
>
{
    typedef RangeTokenIterator<TokenizerFunc, Iterator> self_t;
    friend class boost::iterator_core_access;
public:
    RangeTokenIterator()
    : func_(), next_(), end_(), valid_(false), tok_()
    {}

    RangeTokenIterator(TokenizerFunc f,
                       Iterator begin,
                       Iterator end = Iterator())
    : func_(f), next_(begin), end_(end), valid_(false), tok_(begin, begin)
    {
        initialize();
    }

    RangeTokenIterator(Iterator begin,
                       Iterator end = Iterator())
    : func_(), next_(begin), end_(end), valid_(false), tok_(begin, begin)
    {
        initialize();
    }

    template<class OtherIter>
    RangeTokenIterator(
        const self_t& rhs,
        typename boost::enable_if<boost::is_convertible<OtherIter, Iterator> >::type* = 0
    )
    : func_(rhs.tokenizerFunction())
    , next_(rhs.base())
    , end_(rhs.end())
    , valid_(!rhs.atEnd())
    , tok_(rhs.currentToken())
    {}

    Iterator base() const
    {
        return next_;
    }
    Iterator end() const
    {
        return end_;
    }
    TokenizerFunc tokenizerFunction() const
    {
        return func_;
    }
    const std::pair<Iterator, Iterator>& currentToken() const
    {
        return tok_;
    }
    bool atEnd() const
    {
        return !valid_;
    }

private:
    void increment()
    {
        BOOST_ASSERT(valid_);
        valid_ = func_(next_, end_, tok_);
    }
    const std::pair<Iterator, Iterator>& dereference() const
    {
        BOOST_ASSERT(valid_);
        return tok_;
    }
    template<class Other>
    bool equal(const Other& a) const
    {
        return (a.valid_ && valid_ && a.next_ == next_ && a.end_ == end_)
            || (!a.valid_ && !valid_);
    }
    void initialize()
    {
        if (!valid_)
        {
            func_.reset();
            valid_ = (next_ != end_) ? func_(next_, end_, tok_) : false;
        }
    }

    TokenizerFunc func_;
    Iterator next_;
    Iterator end_;
    bool valid_;
    std::pair<Iterator, Iterator> tok_;
};

template<typename TokenizerFunc, typename Iterator>
RangeTokenIterator<TokenizerFunc, Iterator>
inline makeRangeTokenIterator(
    Iterator begin,
    Iterator end,
    const TokenizerFunc& func = TokenizerFunc()
)
{
    return RangeTokenIterator<TokenizerFunc, Iterator>(func, begin, end);
}

}} // namespace izenelib::util

#endif // INCLUDE_STRING_RANGE_TOKEN_ITERATOR_H
