#ifndef AM_TOKYO_CABINET_RAW_HASH_FAST_RANGE_H
#define AM_TOKYO_CABINET_RAW_HASH_FAST_RANGE_H
/**
 * @file am/tokyo_cabinet/RawHashFastRange.h
 * @author Ian Yang
 * @date Created <2009-09-02 17:06:35>
 * @date Updated <2009-09-02 17:43:33>
 */

namespace izenelib {
namespace am {
namespace tc {

class RawHashFastConstRange
    : boost::noncopyable
{
public:
    RawHashFastConstRange(TCHDB* hdb)
    : empty_(true)
    , hdb_(hdb)
    , cur_()
    , key_(::tcxstrnew())
    , value_(::tcxstrnew())
    {
        BOOST_ASSERT(key_);
        BOOST_ASSERT(value_);
        if (hdb && key_ && value_ && ::tchdbiterinit(hdb))
        {
            get_();
        }
    }

    ~RawHashFastConstRange()
    {
        if (key_)
        {
            ::tcxstrdel(key_);
        }
        if (value_)
        {
            ::tcxstrdel(value_);
        }
    }

    bool empty() const
    {
        return empty_;
    }

    void popFront()
    {
        BOOST_ASSERT(!empty_);
        if (hdb && key_ && value_)
        {
            get_();
        }
    }

    const Key& frontKey()
    {
        BOOST_ASSERT(!empty_);
        return cur_.get_key();
    }
    const Value& frontValue()
    {
        BOOST_ASSERT(!empty_);
        return cur_.get_value();
    }
    const DataType<Key, Value>& front()
    {
        BOOST_ASSERT(!empty_);
        return cur_;
    }

protected:
    void get_()
    {
        empty_ = true;
        if (::tchdbiternext3(hdb_, key_, value_))
        {
            cur_.get_key().attach(
                ::tcxstrptr(key_),
                static_cast<std::size_t>(::tcxstrsize(key_))
            );
            cur_.get_value().attach(
                ::tcxstrptr(value),
                static_cast<std::size_t>(::tcxstrsize(value))
            );

            empty_ = false;
        }
    }

    bool empty_;
    TCHDB* hdb_;
    DataType<Buffer, Buffer> cur_;
    TCXSTR* key_;
    TCXSTR* value_;
};

class RawHashFastRange
    : RawHashFastConstRange
{
public:
    RawHashFastRange(TCHDB* hdb)
    : RawHashFastConstRange(hdb)
    {}

    bool updateFront(const Buffer& value)
    {
        BOOST_ASSERT(!empty_);
        return hdb_ && ::tchdbput(
            hdb_,
            cur_.get_key().data(),
            cur_.get_key().size(),
            value.data(),
            value.size()
        );
    }

    bool delFront()
    {
        BOOST_ASSERT(!empty_);
        return hdb_ && ::tchdbout(
            hdb_,
            cur_.get_key().data(),
            cur_.get_key().size()
        );
    }
};

}}} // namespace izenelib::am::tc

#endif // AM_TOKYO_CABINET_RAW_HASH_FAST_RANGE_H
