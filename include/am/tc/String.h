#ifndef AM_TC_STRING_H
#define AM_TC_STRING_H
/**
 * @file am/tc/String.h
 * @author Ian Yang
 * @date Created <2009-09-06 00:15:24>
 * @date Updated <2009-09-07 14:20:01>
 * @brief Tokyocabinet extensible string wrapper
 */
#include <algorithm>
#include <stdexcept>

#include <boost/iterator/reverse_iterator.hpp>

#include <tcutil.h>

#include <am/raw/Buffer.h>

namespace izenelib {
namespace am {
namespace tc {

namespace detail {
struct StringAccessor;
}

class String
{
public:
    typedef char            value_type;
    typedef int             size_type;
    typedef int             difference_type;
    typedef const char&     const_reference;
    typedef const_reference reference;
    typedef const char*     const_pointer;
    typedef const_pointer   pointer;

    typedef const_pointer  const_iterator;
    typedef const_iterator iterator;

    typedef boost::reverse_iterator<const_iterator>	const_reverse_iterator;
    typedef boost::reverse_iterator<iterator>       reverse_iterator;

    String()
    : handle_(::tcxstrnew())
    {
        BOOST_ASSERT(handle_);
    }

    /**
     * @pre str != 0
     * @warning implicitly conversion from const char* is allowed
     */
    String(const char* str)
    : handle_(::tcxstrnew2(str))
    {
        BOOST_ASSERT(handle_);
    }

    /**
     * @pre str != 0
     */
    String(const char* str, int size)
    : handle_(::tcxstrnew3(size + 1))
    {
        BOOST_ASSERT(handle_);
        append(str, size);
    }

    /**
     * @param size preserves so many memories
     */
    explicit String(int size)
    : handle_(::tcxstrnew3(size))
    {
        BOOST_ASSERT(handle_);
    }

    String(const String& rhs)
    : handle_(::tcxstrdup(rhs.handle_))
    {
        BOOST_ASSERT(handle_);
    }

    String& operator=(String& rhs)
    {
        if (this != &rhs)
        {
            String tmp(rhs);
            swap(tmp);
        }

        return *this;
    }

    ~String()
    {
        if (handle_)
        {
            ::tcxstrdel(handle_);
        }
    }

    String& append(const char* str)
    {
        if (str)
        {
            ::tcxstrcat2(handle_, str);
        }

        return *this;
    }

    String& append(const char* str, int size)
    {
        BOOST_ASSERT(str);
        if (str)
        {
            ::tcxstrcat(handle_, str, size);
        }
        return *this;
    }

    String& append(const String& str)
    {
        ::tcxstrcat(handle_, str.data(), str.size());
        return *this;
    }

    String& operator+=(const char* str)
    {
        return append(str);
    }

    String& operator+=(const String& str)
    {
        return append(str);
    }

    int size() const
    {
        return ::tcxstrsize(handle_);
    }

    bool empty() const
    {
        return size() == 0;
    }

    void clear()
    {
        ::tcxstrclear(handle_);
    }

    void swap(String& rhs)
    {
        ::TCXSTR* rememberThis = handle_;
        handle_ = rhs.handle_;
        rhs.handle_ = rememberThis;
    }

    const char* data() const
    {
        return static_cast<const char*>(::tcxstrptr(handle_));
    }

    const char* c_str() const
    {
        return data();
    }

    const_iterator begin() const
    {
        return data();
    }
    const_iterator end() const
    {
        return data() + size();
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    const_reference operator[](int i) const
    {
        BOOST_ASSERT(i >= 0);
        return data()[i];
    }
    const_reference at(int i) const
    {
        if (i < 0 || i >= size())
        {
            throw std::out_of_range(
                "tc::String: index out of range"
            );
        }
        return data()[i];
    }

    bool equalTo(const String& rhs) const
    {
        if (size() == rhs.size())
        {
            return std::equal(begin(), end(), rhs.begin());
        }

        return false;
    }

    bool lessThan(const String& rhs) const
    {
        return std::lexicographical_compare(
            begin(), end(),
            rhs.begin(), rhs.end()
        );
    }

    /**
     * @brief transfers string internal buffer to @a buf, this string becomes
     * empty.
     * @post this->empty()
     */
    bool toBuffer(raw::Buffer& buf)
    {
        size_type s = size();
        void* data = ::tcxstrtomalloc(handle_);
        if (data)
        {
            buf.attach(static_cast<char*>(data), s, &::tcfree);
        }
        handle_ = ::tcxstrnew();
        BOOST_ASSERT(handle_);

        return data != 0;
    }

private:
    ::TCXSTR* handle_;
    friend struct detail::StringAccessor;
};

inline String operator+(const String& a, const String& b)
{
    String result(a);
    result += b;

    return result;
}

inline bool operator<(const String& a, const String& b)
{
    return a.lessThan(b);
}
inline bool operator==(const String& a, const String& b)
{
    return a.equalTo(b);
}
inline bool operator<=(const String& a, const String& b)
{
    return !(b < a);
}
inline bool operator>(const String& a, const String& b)
{
    return b < a;
}
inline bool operator>=(const String& a, const String& b)
{
    return !(a < b);
}
inline bool operator!=(const String& a, const String& b)
{
    return !(a == b);
}

namespace detail {
struct StringAccessor
{
    inline static ::TCXSTR* handle(String& str)
    {
        return str.handle_;
    }
    inline static const ::TCXSTR* handle(const String& str)
    {
        return str.handle_;
    }
};
} // namespace detail

}}} // namespace izenelib::am::tc

#endif // AM_TC_STRING_H
