#ifndef AM_RANGE_GET_NEXT_RANGE_H
#define AM_RANGE_GET_NEXT_RANGE_H
/**
 * @file am/range/GetNextRange.h
 * @author Ian Yang
 * @date Created <2009-09-08 10:58:28>
 * @date Updated <2009-09-08 11:11:16>
 */

#include <boost/assert.hpp>

namespace izenelib {
namespace am {

template<typename AM>
class GetNextRange
{
public:
    typedef typename AM::data_type data_type;
    typedef typename AM::key_type key_type;
    typedef typename AM::value_type value_type;

    GetNextRange()
    : am_(0), data_()
    {}

    explicit GetNextRange(AM& am)
    : am_(&am), data_()
    {
        if (!am_->getFirst(data_))
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
        if (!am_->getFirst(data_))
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
        if (!am_->getNext(data_))
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

#endif // AM_RANGE_GET_NEXT_RANGE_H
