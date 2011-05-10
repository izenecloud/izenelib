#ifndef AM_RAW_BUFFER_H
#define AM_RAW_BUFFER_H
/**
 * @file am/raw/Buffer.h
 * @author Ian Yang
 * @date Created <2009-09-04 14:05:33>
 * @date Updated <2009-09-06 23:18:06>
 * @brief Wrapper of the memory buffer. A deleter can be specified to delegate
 * the destruction work to Buffer.
 */

#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include <boost/iterator/reverse_iterator.hpp>

// #include <util/izene_serialization.h>

namespace izenelib {
namespace util {
template <typename T> class izene_serialization;
}} // namespace izenelib::util

namespace izenelib {
namespace am {
namespace raw {

class Buffer
{
public:
    // type definitions
    typedef char           value_type;
    typedef char*          iterator;
    typedef const char*    const_iterator;
    typedef char&          reference;
    typedef const char&    const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    typedef boost::reverse_iterator<iterator> reverse_iterator;
    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef void (*deleter_type)(void*);

    Buffer()
    : data_(0), size_(0), deleter_(0)
    {}

    explicit Buffer(size_type size)
    : data_(static_cast<char*>(std::malloc(size)))
    , size_(size)
    , deleter_(&std::free)
    {
        if (!data_)
        {
            size_ = 0;
        }
    }

    Buffer(char* data,
           size_type size,
           deleter_type deleter = 0)
    : data_(data)
    , size_(size)
    , deleter_(deleter)
    {
        if (!data_)
        {
            size_ = 0;
        }
    }

    Buffer(const Buffer& rhs)
    : data_(0)
    , size_(0)
    , deleter_(0)
    {
        if (rhs.data_ && rhs.size_ > 0)
        {
            data_ = static_cast<char*>(std::malloc(rhs.size_));
            if (data_)
            {
                size_ = rhs.size_;
                deleter_ = &std::free;
                std::memcpy(data_, rhs.data_, size_);
            }
        }
    }

    Buffer& operator=(const Buffer& rhs)
    {
        if (this != &rhs)
        {
            if (data_ == rhs.data_)
            {
                deleter_ = 0;
            }

            Buffer tmp(rhs);
            swap(tmp);
        }

        return *this;
    }

    ~Buffer()
    {
        if (deleter_)
        {
            (*deleter_)(data_);
        }
    }

    void attach()
    {
        data_ = const_cast<char*>(str_.data());
        size_ = str_.size();
        deleter_ = 0;
    }

    void attach(char* data,
                size_type size,
                deleter_type deleter = 0)
    {
        if (data_ != data && deleter_)
        {
            (*deleter_)(data_);
        }

        data_ = data;
        size_ = size;
        deleter_ = deleter;
    }

    void copyAttach(const char* data,
                    size_type size)
    {

        if (data_ != data && deleter_)
        {
            (*deleter_)(data_);
        }

        size_ = 0;
        deleter_ = 0;
        data_ = static_cast<char*>(std::malloc(size));
        if (data_)
        {
            std::memcpy(data_, data, size);
            size_ = size;
            deleter_ = &std::free;
        }
    }

    iterator begin()
    {
        return data_;
    }
    const_iterator begin() const
    {
        return data_;
    }
    iterator end()
    {
        return data_ + size_;
    }
    const_iterator end() const
    {
        return data_ + size_;
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    reference operator[](size_type i)
    {
        return data_[i];
    }
    const_reference operator[](size_type i) const
    {
        return data_[i];
    }
    reference at(size_type i)
    {
        rangecheck_(i);
        return data_[i];
    }
    const_reference at(size_type i) const
    {
        rangecheck_(i);
        return data_[i];
    }

    reference front()
    {
        return data_[0];
    }
    const_reference front() const
    {
        return data_[0];
    }
    reference back()
    {
        return data_[size_ - 1];
    }
    const_reference back() const
    {
        return data_[size_ - 1];
    }

    size_type size() const
    {
        return size_;
    }
    bool empty()
    {
        return size_ == 0;
    }
    size_type max_size()
    {
        return size_;
    }

    void swap(Buffer& rhs)
    {
        using std::swap;
        swap(data_, rhs.data_);
        swap(size_, rhs.size_);
        swap(deleter_, rhs.deleter_);
    }

    const char* data() const
    {
        return data_;
    }
    char* data()
    {
        return data_;
    }

    std::string& strbuffer()
    {
        return str_;
    }

    void assign(char value)
    {
        if (data_)
        {
            std::memset(data_, value, size_);
        }
    }

private:
    void rangecheck_(size_type i) const
    {
        if (i >= size())
        {
            throw std::out_of_range(
                "raw::Buffer: index out of range"
            );
        }
    }

    char* data_;
    size_type size_;
    deleter_type deleter_;

    std::string str_; ///for the usage of attach std::string

    template<typename T>
    friend void write_image(
        izenelib::util::izene_serialization<T>& izs,
        Buffer& buf
    );
};

}}} // namespace izenelib::am::raw

#endif // AM_RAW_BUFFER_H
