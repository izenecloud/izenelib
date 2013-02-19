#ifndef _OSACA_CUSTOM_UINT_HPP
#define _OSACA_CUSTOM_UINT_HPP

#include <types.h>
#include <string>
#include <limits>
#include "custom_int.hpp"


namespace izenelib{ namespace am{ namespace succinct{
namespace osaca
{

#pragma pack(push,1)
class uint40_t
{
    uint8_t upper8; //usigned
    uint32_t lower32; //unsigned
public:
    uint40_t()
    {
        upper8 = 0;
        lower32 = 0;
    }
    uint40_t(const uint40_t& o)
    {
        upper8 = o.upper8;
        lower32 = o.lower32;
    }
    uint40_t(uint64_t in_trunc)
    {
        upper8 = (uint8_t)(in_trunc >> 32);
        lower32 = (uint32_t)in_trunc;
    }

    operator uint64_t() const //!Implicit, to make use of the type similarly automatic to that of builtin ints
    {
        return (uint64_t)lower32 | (uint64_t)upper8 << 32;
    }

    uint40_t& operator ++() //prefix
    {
        *this = (uint64_t)(*this) + 1;
        return *this;
    }
    uint40_t operator ++(int) //suffix
    {
        uint40_t tmp = *this;
        ++(*this);
        return tmp;
    }
    uint40_t& operator --() //prefix
    {
        *this = (uint64_t)(*this) - 1;
        return *this;
    }
    uint40_t operator --(int) //suffix
    {
        uint40_t tmp = *this;
        --(*this);
        return tmp;
    }
    uint40_t& operator =(const uint40_t& i)
    {
        upper8 = i.upper8;
        lower32 = i.lower32;
        return *this;
    }
    uint40_t& operator =(const uint64_t i)
    {
        upper8 = (uint8_t)(i >> 32);
        lower32 = (uint32_t)i;
        return *this;
    }
    bool operator >(const uint40_t& b) const
    {
        return (uint64_t)(*this) > (uint64_t)b;
    }
    template<class UIntType>
    bool operator >(UIntType b) const
    {
        return (uint64_t)(*this) > b;
    }
    bool operator >=(const uint40_t& b) const
    {
        return (uint64_t)(*this) >= (uint64_t)b;
    }
    template<class UIntType>
    bool operator >=(UIntType b) const
    {
        return (uint64_t)(*this) >= b;
    }
    bool operator <(const uint40_t& b) const
    {
        return (uint64_t)(*this) < (uint64_t)b;
    }
    template<class UIntType>
    bool operator <(UIntType b) const
    {
        return (uint64_t)(*this) < b;
    }
    bool operator <=(const uint40_t& b) const
    {
        return (uint64_t)(*this) <= (uint64_t)b;
    }
    template<class UIntType>
    bool operator <=(UIntType b) const
    {
        return (uint64_t)(*this) <= b;
    }
    bool operator ==(const uint40_t& b) const
    {
        return (uint64_t)(*this) == (uint64_t)b;
    }
    template<class UIntType>
    bool operator ==(UIntType b) const
    {
        return (uint64_t)(*this) == b;
    }
    bool operator !=(const uint40_t& b) const
    {
        return (uint64_t)(*this) != (uint64_t)b;
    }
    template<class UIntType>
    bool operator !=(UIntType b) const
    {
        return (uint64_t)(*this) != b;
    }
    uint40_t operator +(const uint40_t &x) const
    {
        return (uint64_t)(*this) + (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator +(UIntType n) const
    {
        return (uint64_t)(*this) + n;
    }

    uint40_t operator -(const uint40_t &x) const
    {
        return (uint64_t)(*this) - (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator -(UIntType n) const
    {
        return (uint64_t)(*this) - n;
    }
    uint40_t operator *(const uint40_t &x) const
    {
        return (uint64_t)(*this) * (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator *(UIntType n) const
    {
        return (uint64_t)(*this) * n;
    }
    uint40_t operator /(const uint40_t &x) const
    {
        return (uint64_t)(*this) / (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator /(UIntType n) const
    {
        return (uint64_t)(*this) / n;
    }
    uint40_t operator %(const uint40_t &x) const
    {
        return (uint64_t)(*this) % (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator %(UIntType n) const
    {
        return (uint64_t)(*this) % n;
    }
    uint40_t operator &(const uint40_t &x) const
    {
        return (uint64_t)(*this) & (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator &(UIntType n) const
    {
        return (uint64_t)(*this) & n;
    }
    uint40_t operator |(const uint40_t &x) const
    {
        return (uint64_t)(*this) | (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator |(UIntType n) const
    {
        return (uint64_t)(*this) | n;
    }
    uint40_t operator ^(const uint40_t &x) const
    {
        return (uint64_t)(*this) ^ (uint64_t)x;
    }
    template<class UIntType>
    uint40_t operator ^(UIntType n) const
    {
        return (uint64_t)(*this) ^ n;
    }
    uint40_t operator >>( const uint40_t &b ) const
    {
        return uint40_t( (uint64_t)*this >> (uint64_t)b );
    }
    template<class UIntType>
    uint40_t operator >>( const UIntType b ) const
    {
        return uint40_t( (uint64_t)*this >> b );
    }
    uint40_t operator <<( const uint40_t &b ) const
    {
        return uint40_t( (uint64_t)*this << (uint64_t)b );
    }
    template<class UIntType>
    uint40_t operator <<( const UIntType b ) const
    {
        return uint40_t( (uint64_t)*this << b );
    }
    uint40_t& operator +=(const uint40_t& b)
    {
        *this = (*this) + b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator +=(UIntType b)
    {
        *this = (uint64_t)(*this) + b;
        return *this;
    }
    uint40_t& operator -=(const uint40_t& b)
    {
        *this = (*this) - b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator -=(UIntType b)
    {
        *this = (uint64_t)(*this) - b;
        return *this;
    }
    uint40_t& operator *=(const uint40_t& b)
    {
        *this = (*this) * b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator *=(UIntType b)
    {
        *this = (uint64_t)(*this) * b;
        return *this;
    }
    uint40_t& operator /=(const uint40_t& b)
    {
        *this = (*this) / b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator /=(UIntType b)
    {
        *this = (uint64_t)(*this) / b;
        return *this;
    }
    uint40_t& operator %=(const uint40_t& b)
    {
        *this = (*this) % b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator %=(UIntType b)
    {
        *this = (uint64_t)(*this) % b;
        return *this;
    }
    uint40_t& operator &=(const uint40_t& b)
    {
        *this = (*this) & b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator &=(UIntType b)
    {
        *this = (uint64_t)(*this) & b;
        return *this;
    }
    uint40_t& operator |=(const uint40_t& b)
    {
        *this = (*this) | b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator |=(UIntType b)
    {
        *this = (uint64_t)(*this) | b;
        return *this;
    }
    uint40_t& operator ^=(const uint40_t& b)
    {
        *this = (*this) ^ b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator ^=(UIntType b)
    {
        *this = (uint64_t)(*this) ^ b;
        return *this;
    }
    uint40_t& operator <<=(const uint40_t& b)
    {
        *this = (*this) << b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator <<=(UIntType b)
    {
        *this = (uint64_t)(*this) << (unsigned)b;
        return *this;
    }
    uint40_t& operator >>=(const uint40_t& b)
    {
        *this = (*this) >> b;
        return *this;
    }
    template<class UIntType>
    uint40_t& operator >>=(UIntType b)
    {
        *this = (uint64_t)(*this) >> b;
        return *this;
    }
    bool operator !() const
    {
        return !((uint64_t)*this);
    }
    uint40_t operator -() const
    {
        return -((uint64_t)*this);
    }
    uint40_t operator ~() const
    {
        return ~(uint64_t)(*this);
    }
};
#pragma pack(pop)

}
}

}}

namespace std
{
using izenelib::am::succinct::osaca::uint40_t;
template<>
class numeric_limits<uint40_t>
{
public:
  static uint40_t min() throw()
  {
      return uint40_t(0);
  }
  static uint40_t max() throw()
  {
      return uint40_t(0xFFFFFFFFFF);
  }
};
}

#endif
