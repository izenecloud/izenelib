#ifndef AM_RANGE_ITER_NEXT_RANGE_H
#define AM_RANGE_ITER_NEXT_RANGE_H
/**
 * @file am/range/IterNextRange.h
 * @author Ian Yang
 * @date Created <2009-09-08 10:58:49>
 * @date Updated <2009-09-08 11:11:05>
 */

#include <boost/assert.hpp>

namespace izenelib {
namespace am {

template<typename AM>
class IterNextRange
{
public:
    typedef typename AM::data_type data_type;
    typedef typename AM::key_type key_type;
    typedef typename AM::value_type value_type;

    IterNextRange()
    : am_(0), data_()
    {}

    explicit IterNextRange(AM& am)
    : am_(&am), data_()
    {
        if (! (am_->iterInit() && am_->iterNext(data_)))
        {
            am_ = 0;
        }
    }

    void clear()
    {
        am_ = 0;
    }
    void attach(AM& am)
    {
        am_ = &am;
        if (! (am_->iterInit() && am_->iterNext(data_)))
        {
            am_ = 0;
        }
    }

    bool empty() const
    {
        return am_ == 0;
    }
    void popFront()
    {
        BOOST_ASSERT(am_);
        if (! (am_ && am_->iterNext(data_)))
        {
            am_ = 0;
        }
    }
    const data_type& front() const
    {
        BOOST_ASSERT(am_);
        return data_;
    }
    const key_type& frontKey() const
    {
        BOOST_ASSERT(am_);
        return data_.get_key();
    }
    const value_type& frontValue() const
    {
        BOOST_ASSERT(am_);
        return data_.get_value();
    }
    bool updateFront(const value_type& value)
    {
        BOOST_ASSERT(am_);
        data_.get_value() = value;
        return am_->update(data_);
    }
    bool delFront()
    {
        BOOST_ASSERT(am_);
        key_type key = data_.get_key();
        popFront();
        return am_->del(key);
    }

private:
    AM* am_;
    data_type data_;
};

}} // namespace izenelib::am

#endif // AM_RANGE_ITER_NEXT_RANGE_H
