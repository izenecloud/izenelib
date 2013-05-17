#ifndef IZENE_SERIALIZATION_MEMCPY_H_
#define IZENE_SERIALIZATION_MEMCPY_H_

#include "izene_type_traits.h"
#include <am/am.h>

NS_IZENELIB_UTIL_BEGIN

template<typename T>
inline void write_image_memcpy(const T& dat, char* &str, size_t& size);

template<typename T>
inline void read_image_memcpy(T& dat, const char* str, const size_t size);

template<typename T>
class izene_serialization_memcpy
{
    char* ptr_;
    size_t size_;
public:
    izene_serialization_memcpy(const T& dat)
    {
        write_image_memcpy<T>(dat, ptr_, size_);
    }
    ~izene_serialization_memcpy()
    {
        if (ptr_)
        {
            delete ptr_;
            ptr_ = 0;
        }
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = ptr_;
        size = size_;
    }
};

template<typename T>
class izene_deserialization_memcpy
{
    T dat_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
    {
        read_image_memcpy<T>(dat_, ptr, size);
    }
    void read_image(T& dat)
    {
        dat = dat_;
    }
};

template<>
class izene_serialization_memcpy<std::string>
{
    const std::string& dat_;
public:
    izene_serialization_memcpy(const std::string& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = (char*) dat_.c_str();
        size = dat_.size();
    }
};

template<>
class izene_deserialization_memcpy<std::string>
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(std::string& dat)
    {
        dat = std::string(ptr_, size_);
    }
};

template<typename T>
class izene_serialization_memcpy<std::vector<T> >
{
    const std::vector<T>& dat_;
public:
    izene_serialization_memcpy(const std::vector<T>& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        if (dat_.empty())
        {
            ptr = (char*) &dat_;
            size = 0;
        }
        else
        {
            ptr = (char*) &dat_[0];
            size = dat_.size() * sizeof(T);
        }
    }
};

template<typename T>
class izene_deserialization_memcpy<std::vector<T> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(std::vector<T>& dat)
    {
        if (size_ > 0)
        {
            dat.resize(size_ / sizeof(T));
            memcpy(&dat[0], ptr_, size_);
        }
        else
        {
            dat.clear();
        }
    }
};

template<>
class izene_serialization_memcpy<izenelib::am::NullType>
{
    const char* ptr_;
public:
    izene_serialization_memcpy(const izenelib::am::NullType& dat)
        : ptr_(reinterpret_cast<const char*>(&dat))
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = const_cast<char*>(ptr_);
        size = 0;
    }
};

template<>
class izene_deserialization_memcpy<izenelib::am::NullType>
{
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
    {
    }
    void read_image(izenelib::am::NullType& dat)
    {
    }
};

template<typename T>
class izene_serialization_memcpy<std::pair<T, T> >
{
    const std::pair<T, T>& dat_;
public:
    izene_serialization_memcpy(const std::pair<T, T>& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = (char*) &dat_;
        size = sizeof(dat_);
    }
};

template<typename T>
class izene_deserialization_memcpy<std::pair<T, T> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(std::pair<T, T>& dat)
    {
        memcpy(&dat, ptr_, size_);
    }
};

template<typename T1, typename T2>
class izene_serialization_memcpy<std::pair<T1, T2> >
{
    const std::pair<T1, T2>& dat_;
public:
    izene_serialization_memcpy(const std::pair<T1, T2>& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        // if sizeof(T1) != sizeof(T2), we need clear padding data to avoid random padding data. 
        if (sizeof(T1) != sizeof(T2))
        {
            if ((char*)&(dat_.second) > (char*)&dat_ + sizeof(T1))
            {
                ::memset((char*)&dat_ + sizeof(T1), 0x00,
                    (char*)&(dat_.second) - ((char*)&dat_ + sizeof(T1)));
            }
            if ((char*)&dat_ + sizeof(dat_) > (char*)&(dat_.second) + sizeof(T2))
            {
                ::memset((char*)&(dat_.second) + sizeof(T2), 0x00,
                    (char*)&dat_ + sizeof(dat_) - ((char*)&(dat_.second) + sizeof(T2)));
            }
        }
        ptr = (char*) &dat_;
        size = sizeof(dat_);
    }
};

template<typename T1, typename T2>
class izene_deserialization_memcpy<std::pair<T1, T2> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(std::pair<T1, T2>& dat)
    {
        memcpy(&dat, ptr_, size_);
    }
};

template<typename T1>
class izene_serialization_memcpy<boost::tuple<T1> >
{
    const boost::tuple<T1>& dat_;
public:
    izene_serialization_memcpy(const boost::tuple<T1>& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = (char*) &dat_;
        size = sizeof(dat_);
    }
};
template<typename T1>
class izene_deserialization_memcpy<boost::tuple<T1> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(boost::tuple<T1>& dat)
    {
        memcpy(&dat, ptr_, size_);
    }
};

template<typename T1, typename T2>
class izene_serialization_memcpy<boost::tuple<T1, T2> >
{
    const boost::tuple<T1, T2>& dat_;
public:
    izene_serialization_memcpy(const boost::tuple<T1, T2>& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        if (sizeof(T1) != sizeof(T2))
        {
            if ((char*)&boost::get<1>(dat_) > (char*)&dat_ + sizeof(T1))
            {
                ::memset((char*)&dat_ + sizeof(T1), 0x00,
                    (char*)&boost::get<1>(dat_) - ((char*)&dat_ + sizeof(T1)));
            }
            if ((char*)&dat_ + sizeof(dat_) > (char*)&boost::get<1>(dat_) + sizeof(T2))
            {
                ::memset((char*)&boost::get<1>(dat_) + sizeof(T2), 0x00,
                    (char*)&dat_ + sizeof(dat_) - ((char*)&boost::get<1>(dat_) + sizeof(T2)));
            }
        }

        ptr = (char*) &dat_;
        size = sizeof(dat_);
    }
};

template<typename T1, typename T2>
class izene_deserialization_memcpy<boost::tuple<T1, T2> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(boost::tuple<T1, T2>& dat)
    {
        memcpy(&dat, ptr_, size_);
    }
};


template<typename T1, typename T2, typename T3>
class izene_serialization_memcpy<boost::tuple<T1, T2, T3> >
{
    const boost::tuple<T1, T2, T3 >& dat_;
public:
    izene_serialization_memcpy(const boost::tuple<T1, T2, T3 >& dat)
        : dat_(dat)
    {
    }
    void write_image(char* &ptr, size_t& size)
    {
        if (sizeof(T1) != sizeof(T2) || sizeof(T1) != sizeof(T3) || sizeof(T2) != sizeof(T3))
        {
            if ((char*)&boost::get<1>(dat_) > (char*)&dat_ + sizeof(T1))
            {
                ::memset((char*)&dat_ + sizeof(T1), 0x00,
                    (char*)&boost::get<1>(dat_) - ((char*)&dat_ + sizeof(T1)));
            }
            if ((char*)&boost::get<2>(dat_) > (char*)&boost::get<1>(dat_) + sizeof(T2))
            {
                ::memset((char*)&boost::get<1>(dat_) + sizeof(T2), 0x00,
                    (char*)&boost::get<2>(dat_) - ((char*)&boost::get<1>(dat_) + sizeof(T2)));
            }
            if ((char*)&dat_ + sizeof(dat_) > (char*)&boost::get<2>(dat_) + sizeof(T3))
            {
                ::memset((char*)&boost::get<2>(dat_) + sizeof(T3), 0x00,
                    (char*)&dat_ + sizeof(dat_) - ((char*)&boost::get<2>(dat_) + sizeof(T3)));
            }
        }

        ptr = (char*) &dat_;
        size = sizeof(dat_);
    }
};

template<typename T1, typename T2, typename T3>
class izene_deserialization_memcpy<boost::tuple<T1, T2, T3> >
{
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size)
        : ptr_(ptr), size_(size)
    {
    }
    void read_image(boost::tuple<T1, T2, T3 >& dat)
    {
        memcpy(&dat, ptr_, size_);
    }
};


NS_IZENELIB_UTIL_END


#define MAKE_MEMCPY(...) \
namespace izenelib { \
namespace util { \
\
template<> \
class izene_serialization_memcpy< __VA_ARGS__ > \
{ \
    const __VA_ARGS__& dat_; \
public: \
    izene_serialization_memcpy(const __VA_ARGS__& dat) \
        : dat_(dat) \
    {} \
    void write_image(char* &ptr, size_t& size) \
    { \
        ptr = (char*) &dat_; \
        size = sizeof(dat_); \
    } \
}; \
\
template<> class izene_deserialization_memcpy< __VA_ARGS__ > \
{ \
    const char* ptr_; \
    const size_t size_; \
public: \
    izene_deserialization_memcpy(const char* ptr, const size_t size) \
        : ptr_(ptr), size_(size) \
    {} \
    void read_image(__VA_ARGS__& dat) \
    { \
        memcpy(&dat, ptr_, size_); \
    } \
}; \
\
} \
}

//MAKE_MEMECPY_TYPE = MAKE_MEMCPY + MAKE_MEMCPY_SERIALIZATION

#define MAKE_MEMCPY_TYPE(...) \
MAKE_MEMCPY_SERIALIZATION(__VA_ARGS__) \
MAKE_MEMCPY(__VA_ARGS__)

MAKE_MEMCPY(bool)

MAKE_MEMCPY(char)

MAKE_MEMCPY(unsigned char)

MAKE_MEMCPY(wchar_t)

MAKE_MEMCPY(short)

MAKE_MEMCPY(unsigned short)

MAKE_MEMCPY(int)

MAKE_MEMCPY(unsigned int)

MAKE_MEMCPY(long)

MAKE_MEMCPY(unsigned long)

#if 1 // LONGLONG_EXISTS
MAKE_MEMCPY(signed long long)
MAKE_MEMCPY(unsigned long long)
#endif

#if !defined(WIN32) || defined(__MINGW32__)
MAKE_MEMCPY_TYPE(int128_t)

MAKE_MEMCPY_TYPE(uint128_t)
#endif

MAKE_MEMCPY(float)

MAKE_MEMCPY(double)

MAKE_MEMCPY(long double)

#endif /*IZENE_SERIALIZATION_MEMCPY_H_*/
