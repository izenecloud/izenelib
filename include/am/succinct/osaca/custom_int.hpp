#ifndef _OSACA_CUSTOM_INT_HPP
#define _OSACA_CUSTOM_INT_HPP

#include <types.h>
#include <string>
#include <limits>
#include "custom_uint.hpp"

namespace izenelib{ namespace am{ namespace succinct{
namespace osaca
{

#pragma pack(push,1)
class int40_t
{
    char upper8; //signed
    uint32_t lower32; //unsigned
public:
    int40_t()
    {
        upper8 = 0;
        lower32 = 0;
    }
    int40_t(const int40_t& o)
    {
        upper8 = o.upper8;
        lower32 = o.lower32;
    }
    int40_t(int64_t in_trunc)
    {
        upper8 = char(in_trunc >> 32);
        lower32 = (uint32_t)in_trunc;
    }

    int40_t(const uint40_t& o)
    {
        upper8 = char(o >> 32);
        lower32 = (uint32_t)o;
    }
    operator int64_t() const //!Implicit, to make use of the type similarly automatic to that of builtin ints
    {
        return (int64_t)lower32 | (int64_t)upper8 << 32;
    }
    int40_t& operator ++() //prefix
    {
        *this = (int64_t)(*this) + 1;
        return *this;
    }
    int40_t operator ++(int) //suffix
    {
        int40_t tmp = *this;
        ++(*this);
        return tmp;
    }
    int40_t& operator --() //prefix
    {
        *this = (int64_t)(*this) - 1;
        return *this;
    }
    int40_t operator --(int) //suffix
    {
        int40_t tmp = *this;
        --(*this);
        return tmp;
    }
    int40_t& operator =(const int40_t& i)
    {
        upper8 = i.upper8;
        lower32 = i.lower32;
        return *this;
    }
    int40_t& operator =(const int64_t i)
    {
        upper8 = char(i >> 32);
        lower32 = (uint32_t)i;
        return *this;
    }
    int40_t& operator =(const osaca::uint40_t i)
    {
        upper8 = char(i >> 32);
        lower32 = (uint32_t)i;
        return *this;
    }
    bool operator >(const int40_t& b) const
    {
        return (int64_t)(*this) > (int64_t)b;
    }
    template<class IntType>
    bool operator >(IntType b) const
    {
        return (int64_t)(*this) > b;
    }
    bool operator >=(const int40_t& b) const
    {
        return (int64_t)(*this) >= (int64_t)b;
    }
    template<class IntType>
    bool operator >=(IntType b) const
    {
        return (int64_t)(*this) >= b;
    }
    bool operator <(const int40_t& b) const
    {
        return (int64_t)(*this) < (int64_t)b;
    }
    template<class IntType>
    bool operator <(IntType b) const
    {
        return (int64_t)(*this) < b;
    }
    bool operator <=(const int40_t& b) const
    {
        return (int64_t)(*this) <= (int64_t)b;
    }
    template<class IntType>
    bool operator <=(IntType b) const
    {
        return (int64_t)(*this) <= b;
    }
    bool operator ==(const int40_t& b) const
    {
        return (int64_t)(*this) == (int64_t)b;
    }
    template<class IntType>
    bool operator ==(IntType b) const
    {
        return (int64_t)(*this) == b;
    }
    bool operator !=(const int40_t& b) const
    {
        return (int64_t)(*this) != (int64_t)b;
    }
    template<class IntType>
    bool operator !=(IntType b) const
    {
        return (int64_t)(*this) != b;
    }
    int40_t operator +(const int40_t &x) const
    {
        return (int64_t)(*this) + (int64_t)x;
    }
    template<class IntType>
    int40_t operator +(IntType n) const
    {
        return (int64_t)(*this) + n;
    }
    int40_t operator -(const int40_t &x) const
    {
        return (int64_t)(*this) - (int64_t)x;
    }
    template<class IntType>
    int40_t operator -(IntType n) const
    {
        return (int64_t)(*this) - n;
    }
    int40_t operator *(const int40_t &x) const
    {
        return (int64_t)(*this) * (int64_t)x;
    }
    template<class IntType>
    int40_t operator *(IntType n) const
    {
        return (int64_t)(*this) * n;
    }
    int40_t operator /(const int40_t &x) const
    {
        return (int64_t)(*this) / (int64_t)x;
    }
    template<class IntType>
    int40_t operator /(IntType n) const
    {
        return (int64_t)(*this) / n;
    }
    int40_t operator %(const int40_t &x) const
    {
        return (int64_t)(*this) % (int64_t)x;
    }
    template<class IntType>
    int40_t operator %(IntType n) const
    {
        return (int64_t)(*this) % n;
    }
    int40_t operator &(const int40_t &x) const
    {
        return (int64_t)(*this) & (int64_t)x;
    }
    template<class IntType>
    int40_t operator &(IntType n) const
    {
        return (int64_t)(*this) & n;
    }
    int40_t operator |(const int40_t &x) const
    {
        return (int64_t)(*this) | (int64_t)x;
    }
    template<class IntType>
    int40_t operator |(IntType n) const
    {
        return (int64_t)(*this) | n;
    }
    int40_t operator ^(const int40_t &x) const
    {
        return (int64_t)(*this) ^ (int64_t)x;
    }
    template<class IntType>
    int40_t operator ^(IntType n) const
    {
        return (int64_t)(*this) ^ n;
    }
    int40_t operator >>( const int40_t &b ) const
    {
        return int40_t( (int64_t)*this >> (int64_t)b );
    }
    template<class IntType>
    int40_t operator >>( const IntType b ) const
    {
        return int40_t( (int64_t)*this >> b );
    }
    int40_t operator <<( const int40_t &b ) const
    {
        return int40_t( (int64_t)*this << (int64_t)b );
    }
    template<class IntType>
    int40_t operator <<( const IntType b ) const
    {
        return int40_t( (int64_t)*this << b );
    }
    int40_t& operator +=(const int40_t& b)
    {
        *this = (*this) + b;
        return *this;
    }
    template<class IntType>
    int40_t& operator +=(IntType b)
    {
        *this = (int64_t)(*this) + b;
        return *this;
    }
    int40_t& operator -=(const int40_t& b)
    {
        *this = (*this) - b;
        return *this;
    }
    template<class IntType>
    int40_t& operator -=(IntType b)
    {
        *this = (int64_t)(*this) - b;
        return *this;
    }
    int40_t& operator *=(const int40_t& b)
    {
        *this = (*this) * b;
        return *this;
    }
    template<class IntType>
    int40_t& operator *=(IntType b)
    {
        *this = (int64_t)(*this) * b;
        return *this;
    }
    int40_t& operator /=(const int40_t& b)
    {
        *this = (*this) / b;
        return *this;
    }
    template<class IntType>
    int40_t& operator /=(IntType b)
    {
        *this = (int64_t)(*this) / b;
        return *this;
    }
    int40_t& operator %=(const int40_t& b)
    {
        *this = (*this) % b;
        return *this;
    }
    template<class IntType>
    int40_t& operator %=(IntType b)
    {
        *this = (int64_t)(*this) % b;
        return *this;
    }
    int40_t& operator &=(const int40_t& b)
    {
        *this = (*this) & b;
        return *this;
    }
    template<class IntType>
    int40_t& operator &=(IntType b)
    {
        *this = (int64_t)(*this) & b;
        return *this;
    }
    int40_t& operator |=(const int40_t& b)
    {
        *this = (*this) | b;
        return *this;
    }
    template<class IntType>
    int40_t& operator |=(IntType b)
    {
        *this = (int64_t)(*this) | b;
        return *this;
    }
    int40_t& operator ^=(const int40_t& b)
    {
        *this = (*this) ^ b;
        return *this;
    }
    template<class IntType>
    int40_t& operator ^=(IntType b)
    {
        *this = (int64_t)(*this) ^ b;
        return *this;
    }
    int40_t& operator <<=(const int40_t& b)
    {
        *this = (*this) << b;
        return *this;
    }
    template<class IntType>
    int40_t& operator <<=(IntType b)
    {
        *this = (int64_t)(*this) << (unsigned)b;
        return *this;
    }
    int40_t& operator >>=(const int40_t& b)
    {
        *this = (*this) >> b;
        return *this;
    }
    template<class IntType>
    int40_t& operator >>=(IntType b)
    {
        *this = (int64_t)(*this) >> b;
        return *this;
    }
    bool operator !() const
    {
        return !((int64_t)*this);
    }
    int40_t operator -() const
    {
        return -((int64_t)*this);
    }
    int40_t operator ~() const
    {
        return ~(int64_t)(*this);
    }
};
#pragma pack(pop)

}
}

}}

#endif
