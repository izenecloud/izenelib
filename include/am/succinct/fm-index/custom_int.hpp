#ifndef _FM_INDEX_CUSTOM_INT_HPP
#define _FM_INDEX_CUSTOM_INT_HPP

#include <types.h>
#include <string>
#include <limits>

NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace fm_index
{

#pragma pack(push,1)
class int48_t
{
public:
    int16_t upper16; 	//signed
    uint32_t lower32;	//unsigned

public:
    int48_t()
    {
        upper16 = 0;
        lower32 = 0;
    }
    int48_t(const int48_t& o)
    {
        upper16 = o.upper16;
        lower32 = o.lower32;
    }
    int48_t(int64_t in_trunc)
    {
        upper16 = int16_t(in_trunc >> 32);
        lower32 = (uint32_t)in_trunc; //the bit-clearing may not be really necessary
    }

    operator int64_t() const  	//!Implicit, to make use of the type similarly automatic to that of builtin ints
    {
        return (int64_t)lower32 | (int64_t)upper16 << 32;
    }
    int48_t& operator ++()   //prefix
    {
        *this = (int64_t)(*this) + 1;
//      if (!(++lower32)) ++upper16;
        return *this;
    }
    int48_t operator ++(int)  	//suffix, int dummy param
    {
        int48_t tmp = *this;
        ++(*this);
        return tmp;
    }

    int48_t& operator --()   //prefix
    {
        *this = (int64_t)(*this) - 1;
//      if (!(lower32--)) --upper16;
        return *this;
    }
    int48_t operator --(int)  	//suffix, int dummy param
    {
        int48_t tmp = *this;
        --(*this);
        return tmp;
    }
    int48_t& operator =(const int48_t& i)
    {
        upper16 = i.upper16;
        lower32 = i.lower32;
        return *this;
    }
    int48_t& operator =(const int64_t i)
    {
        upper16 = int16_t(i >> 32);
        lower32 = (uint32_t)i;
        return *this;
    }
    bool operator >(const int48_t& b) const
    {
        return (int64_t)(*this) > (int64_t)b;
    }
    template<class IntType>
    bool operator >(IntType b) const
    {
        return (int64_t)(*this) > b;
    }
    bool operator >=(const int48_t& b) const
    {
        return (int64_t)(*this) >= (int64_t)b;
    }
    template<class IntType>
    bool operator >=(IntType b) const
    {
        return (int64_t)(*this) >= b;
    }
    bool operator <(const int48_t& b) const
    {
        return (int64_t)(*this) < (int64_t)b;
    }
    template<class IntType>
    bool operator <(IntType b) const
    {
        return (int64_t)(*this) < b;
    }
    bool operator <=(const int48_t& b) const
    {
        return (int64_t)(*this) <= (int64_t)b;
    }
    template<class IntType>
    bool operator <=(IntType b) const
    {
        return (int64_t)(*this) <= b;
    }
    bool operator ==(const int48_t& b) const
    {
        return (int64_t)(*this) == (int64_t)b;
    }
    template<class IntType>
    bool operator ==(IntType b) const
    {
        return (int64_t)(*this) == b;
    }
    bool operator !=(const int48_t& b) const
    {
        return (int64_t)(*this) != (int64_t)b;
    }
    template<class IntType>
    bool operator !=(IntType b) const
    {
        return (int64_t)(*this) != b;
    }
    int48_t operator +(const int48_t &x) const
    {
        return (int64_t)(*this) + (int64_t)x;
    }
    template<class IntType>
    int48_t operator +(IntType n) const
    {
        return (int64_t)(*this) + n;
    }
    int48_t operator -(const int48_t &x) const
    {
        return (int64_t)(*this) - (int64_t)x;
    }
    template<class IntType>
    int48_t operator -(IntType n) const
    {
        return (int64_t)(*this) - n;
    }
    int48_t operator *(const int48_t &x) const
    {
        return (int64_t)(*this) * (int64_t)x;
    }
    template<class IntType>
    int48_t operator *(IntType n) const
    {
        return (int64_t)(*this) * n;
    }
    int48_t operator /(const int48_t &x) const
    {
        return (int64_t)(*this) / (int64_t)x;
    }
    template<class IntType>
    int48_t operator /(IntType n) const
    {
        return (int64_t)(*this) / n;
    }
    int48_t operator %(const int48_t &x) const
    {
        return (int64_t)(*this) % (int64_t)x;
    }
    template<class IntType>
    int48_t operator %(IntType n) const
    {
        return (int64_t)(*this) % n;
    }
    int48_t operator &(const int48_t &x) const
    {
        return (int64_t)(*this) & (int64_t)x;
    }
    template<class IntType>
    int48_t operator &(IntType n) const
    {
        return (int64_t)(*this) & n;
    }
    int48_t operator |(const int48_t &x) const
    {
        return (int64_t)(*this) | (int64_t)x;
    }
    template<class IntType>
    int48_t operator |(IntType n) const
    {
        return (int64_t)(*this) | n;
    }
    int48_t operator ^(const int48_t &x) const
    {
        return (int64_t)(*this) ^ (int64_t)x;
    }
    template<class IntType>
    int48_t operator ^(IntType n) const
    {
        return (int64_t)(*this) ^ n;
    }
    int48_t operator >>( const int48_t &b ) const
    {
        return int48_t( (int64_t)*this >> (int64_t)b );
    }
    template<class IntType>
    int48_t operator >>( const IntType b ) const
    {
        return int48_t( (int64_t)*this >> b );
    }
    int48_t operator <<( const int48_t &b ) const
    {
        return int48_t( (int64_t)*this << (int64_t)b );
    }
    template<class IntType>
    int48_t operator <<( const IntType b ) const
    {
        return int48_t( (int64_t)*this >> b );
    }
    int48_t& operator +=(const int48_t& b)
    {
        *this = (*this) + b;
        return *this;
    }
    int48_t& operator -=(const int48_t& b)
    {
        *this = (*this) - b;
        return *this;
    }
    int48_t& operator *=(const int48_t& b)
    {
        *this = (*this) * b;
        return *this;
    }
    int48_t& operator /=(const int48_t& b)
    {
        *this = (*this) / b;
        return *this;
    }
    int48_t& operator %=(const int48_t& b)
    {
        *this = (*this) % b;
        return *this;
    }
    int48_t& operator &=(const int48_t& b)
    {
        *this = (*this) & b;
        return *this;
    }
    int48_t& operator |=(const int48_t& b)
    {
        *this = (*this) | b;
        return *this;
    }
    int48_t& operator ^=(const int48_t& b)
    {
        *this = (*this) ^ b;
        return *this;
    }
    int48_t& operator <<=(const int48_t& b)
    {
        *this = (*this) << b;
        return *this;
    }
    int48_t& operator >>=(const int48_t& b)
    {
        *this = (*this) >> b;
        return *this;
    }
    bool operator !() const
    {
        return !((int64_t)*this);
    }
    int48_t operator -() const
    {
        return -((int64_t)*this);
    }
    int48_t operator ~() const
    {
        return ~(int64_t)(*this);
    }
};
#pragma pack(pop)

}
}

NS_IZENELIB_AM_END

namespace std
{
using izenelib::am::succinct::fm_index::int48_t;
template<>
class numeric_limits<int48_t>
{
public:
  static int48_t min() throw()
  {
      return int48_t(-0x7FFFFFFFFFFF - 1);
  }
  static int48_t max() throw()
  {
      return int48_t(0x7FFFFFFFFFFF);
  }
};
}

#endif
