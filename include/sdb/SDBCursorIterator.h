#ifndef SDB_CURSOR_ITERATOR_H
#define SDB_CURSOR_ITERATOR_H
/**
 * @file SDBCursorIterator.h
 *
 * @author Ian Yang
 * @date Created <2009-08-25 11:08:15>
 * @brief iterator wrapper of SDBCursor
 *
 * @date Updated (Jun Jiang) <2011-04-19>
 * @brief change data_type to a pair of key, value, also change increment() for SDB usage.
 *
 * usage:
 *   typedef int KeyType;
 *   typedef string ValueType;
 *   typedef izenelib::sdb::unordered_sdb_tc<KeyType, ValueType> SDBType;
 *   typedef izenelib::sdb::SDBCursorIterator<SDBType> SDBIterator;
 *
 *   SDBType container;
 *   SDBIterator begin = SDBIterator(container);
 *   SDBIterator end = SDBIterator();
 *
 *   for (SDBIterator it = begin; it != end; ++it)
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

namespace sdb
{

template<typename AM, typename AMTraits>
class SDBCursorIterator;

namespace detail
{
template<typename AM, typename AMTraits>
struct SDBCursorIteratorBase
{
    typedef std::pair<typename AM::SDBKeyType, typename AM::SDBValueType> data_type;

    typedef boost::iterator_facade<SDBCursorIterator<AM, AMTraits> , data_type,
    boost::forward_traversal_tag, const data_type&> type;
};

template<typename AM>
struct SDBCursorIteratorBase<AM, typename boost::enable_if<boost::mpl::bool_<AM::AMWrapperType> >::type>
{
    typedef std::pair<typename AM::key_type, typename AM::value_type> data_type;

    typedef boost::iterator_facade<SDBCursorIterator<AM,  typename boost::enable_if<boost::mpl::bool_<AM::AMWrapperType> >::type > ,
    data_type,
    boost::forward_traversal_tag, const data_type&> type;
};

} // namespace detail

/**
 * @brief forward iterator wrapper of SDBCursor used in AccessMethod
 * iteration.
 */
template<typename AM, typename AMTraits=void>
class SDBCursorIterator: public detail::SDBCursorIteratorBase<AM, AMTraits>::type
{
    friend class boost::iterator_core_access;
    typedef typename detail::SDBCursorIteratorBase<AM, AMTraits>::type super;

public:
    /**
     * @brief constructs a end sentry
     * @return iterator as end
     */
    SDBCursorIterator() :
            am_(0), cur_(), data_()
    {
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting SDBCursor
     * @return iterator as begin
     */
    explicit SDBCursorIterator(AM& am) :
            am_(&am), cur_(am.get_first_locn()), data_()
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
            if (am_->get(cur_, data_.first, data_.second))
            {
                am_->seq(cur_);
            }
            else
            {
                am_ = 0;
            }
        }
    }

    bool equal(const SDBCursorIterator<AM>& rhs) const
    {
        return (this == &rhs) || (am_ == 0 && rhs.am_ == 0)
               || (am_ == rhs.am_ && cur_ == rhs.cur_);
    }

    AM* am_;
    typename AM::SDBCursor cur_;
    typename super::value_type data_;
};

template<typename AM>
class SDBCursorIterator<AM, typename boost::enable_if<boost::mpl::bool_<AM::AMWrapperType> >::type>
            : public detail::SDBCursorIteratorBase<AM,typename boost::enable_if<boost::mpl::bool_<AM::AMWrapperType> >::type>::type
{
    friend class boost::iterator_core_access;
    typedef typename detail::SDBCursorIteratorBase<AM,typename boost::enable_if<boost::mpl::bool_<AM::AMWrapperType> >::type>::type super;

public:
    /**
     * @brief constructs a end sentry
     * @return iterator as end
     */
    SDBCursorIterator() :
            am_(0), data_()
    {
    }

    /**
     * @brief begin iterator
     * @param am AccessMethod object supporting SDBCursor
     * @return iterator as begin
     */
    explicit SDBCursorIterator(AM& am)
    {
        attach(am);
    }

private:
    typename super::reference dereference() const
    {
        return data_;
    }

    void attach(AM& am)
    {
        am_ = &am;
        if (! (am_->iterInit() && am_->iterNext(data_.first, data_.second)))
        {
            am_ = 0;
        }
    }

    void increment()
    {
        if (am_)
        {
            if (! am_->iterNext(data_.first, data_.second))
            {
                am_ = 0;
            }
        }
    }

    bool equal(const SDBCursorIterator<AM>& rhs) const
    {
        return (this == &rhs) || (am_ == 0 && rhs.am_ == 0)
               || (am_ == rhs.am_ );
    }

    AM* am_;
    typename super::value_type data_;
};

} // namespace sdb
} // namespace izenelib

#endif // SDB_CURSOR_ITERATOR_H
