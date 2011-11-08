#ifndef IZENELIB_AM_ITERATOR_H
#define IZENELIB_AM_ITERATOR_H
/**
 * @file AmIterator.h
 *
 * usage:
 *   typedef int KeyType;
 *   typedef string ValueType;
 *   typedef izenelib::am::tc::BTree<KeyType, ValueType> AMType;
 *   typedef izenelib::am::AmForwardIterator<AMType> AMIterator;
 *
 *   AMType container;
 *   AMIterator begin = AMIterator(container);
 *   AMIterator end = AMIterator();
 *
 *   for (AMIterator it = begin; it != end; ++it)
 *   {
 *      const KeyType& key = it->first;
 *      const ValueType& value = it->second;
 *   }
 */
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <iostream>
namespace izenelib
{
namespace am
{

template<typename AM>
class AMIterator;
template<typename AM>
class AMReverseIterator;

namespace detail
{
template<typename AM>
struct AMForwardIteratorBase
{
    typedef std::pair<typename AM::key_type, typename AM::value_type> data_type;
    typedef boost::iterator_facade<AMIterator<AM> , data_type,
    boost::forward_traversal_tag, const data_type&> type;
};

template<typename AM>
struct AMReverseIteratorBase
{
    typedef std::pair<typename AM::key_type, typename AM::value_type> data_type;
    typedef boost::iterator_facade<AMReverseIterator<AM> , data_type,
    boost::forward_traversal_tag, const data_type&> type;
};

} // namespace detail

/**
 * @brief forward iterator wrapper of Cursor used in AccessMethod
 * iteration.
 */
template<typename AM>
class AMIterator: public detail::AMForwardIteratorBase<AM>::type
{
    friend class boost::iterator_core_access;
    typedef typename detail::AMForwardIteratorBase<AM>::type super;
    typedef typename super::value_type::first_type key_type;

public:
    /**
     * @brief constructs a end sentry
     * @return iterator as end
     */
    AMIterator() :
            am_(0), cur_(), data_()
    {
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting Cursor
     * @return iterator as begin
     */
    explicit AMIterator(AM& am) :
            am_(&am), cur_(am.begin()), data_()
    {
        increment();
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting Cursor
     * @return iterator as begin
     */
    explicit AMIterator(AM& am, const key_type& key) :
            am_(&am), cur_(am.begin(key)), data_()
    {
        increment();
    }

private:
    typename super::reference dereference() const
    {
        return data_;
    }

    void increment()
    {
        if (am_)
        {
            if (am_->fetch(cur_, data_.first, data_.second))
            {
                am_->iterNext(cur_);
            }
            else
            {
                am_ = 0;
            }
        }
    }

    bool equal(const AMIterator<AM>& rhs) const
    {
        return (this == &rhs) || (am_ == 0 && rhs.am_ == 0)
               || (am_ == rhs.am_ && cur_ == rhs.cur_);
    }

    AM* am_;
    typename AM::cursor_type cur_;
    typename super::value_type data_;
};

template<typename AM>
class AMReverseIterator: public detail::AMReverseIteratorBase<AM>::type
{
    friend class boost::iterator_core_access;
    typedef typename detail::AMReverseIteratorBase<AM>::type super;
    typedef typename super::value_type::first_type key_type;

public:
    /**
     * @brief constructs a end sentry
     * @return iterator as end
     */
    AMReverseIterator() :
            am_(0), cur_(), data_()
    {
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting Cursor
     * @return iterator as begin
     */
    explicit AMReverseIterator(AM& am) :
            am_(&am), cur_(am.rbegin()), data_()
    {
        increment();
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting Cursor
     * @return iterator as begin
     */
    explicit AMReverseIterator(AM& am, const key_type& key) :
            am_(&am), cur_(am.begin(key)), data_()
    {
        increment();
    }

private:
    typename super::reference dereference() const
    {
        return data_;
    }

    void increment()
    {
        if (am_)
        {
            if (am_->fetch(cur_, data_.first, data_.second))
            {
                am_->iterPrev(cur_);
            }
            else
            {
                am_ = 0;
            }
        }
    }

    bool equal(const AMReverseIterator<AM>& rhs) const
    {
        return (this == &rhs) || (am_ == 0 && rhs.am_ == 0)
               || (am_ == rhs.am_ && cur_ == rhs.cur_);
    }

    AM* am_;
    typename AM::cursor_type cur_;
    typename super::value_type data_;
};

}
}
#endif // IZENELIB_AM_ITERATOR_H
