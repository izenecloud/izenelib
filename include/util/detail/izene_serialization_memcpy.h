#ifndef IZENE_SERIALIZATION_MEMCPY_H_
#define IZENE_SERIALIZATION_MEMCPY_H_

#include "izene_type_traits.h"
#include <am/am.h>

NS_IZENELIB_UTIL_BEGIN

template<typename T>
inline void write_image_memcpy(const T& dat, char* &str, size_t& size);

template<typename T> inline void read_image_memcpy(T& dat, const char* str,
        const size_t size);

template<typename T> class izene_serialization_memcpy
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
            delete ptr_;
        ptr_ = 0;
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
        ptr = (char*)dat_.c_str();
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


#define MAKE_MEMCPY(TYPE) \
namespace izenelib { \
namespace util { \
\
template<> \
class izene_serialization_memcpy< TYPE > \
{ \
    const TYPE& dat_; \
public: \
    izene_serialization_memcpy(const TYPE& dat) \
        : dat_(dat) \
    {} \
    void write_image(char* &ptr, size_t& size) \
    { \
        ptr = (char*) &dat_; \
        size = sizeof(dat_); \
    } \
}; \
\
template<> class izene_deserialization_memcpy< TYPE > \
{ \
    const char* ptr_; \
    const size_t size_; \
public: \
    izene_deserialization_memcpy(const char* ptr, const size_t size) \
        : ptr_(ptr), size_(size) \
    {} \
    void read_image(TYPE& dat) \
    { \
        memcpy(&dat, ptr_, size_); \
    } \
}; \
\
} \
}

//MAKE_MEMECPY_TYPE = MAKE_MEMCPY + MAKE_MEMCPY_SERIALIZATION

#define MAKE_MEMCPY_TYPE(TYPE) \
namespace izenelib { \
namespace util{ \
\
template <> \
struct IsMemcpySerial< TYPE > \
{ \
    enum { yes = 1, no =! yes }; \
}; \
\
} \
} \
\
MAKE_MEMCPY(TYPE)

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
