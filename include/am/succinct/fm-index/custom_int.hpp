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
protected:
    int16_t upper16; 	//signed
    uint32_t lower32;	//unsigned
public:
    int48_t()
    {
        upper16 = 0;
        lower32 = 0;
    }
    int48_t(int64_t in_trunc)
    {
        upper16 = int16_t( in_trunc >> 32 );
        lower32  = (uint32_t) ( in_trunc bitand ( 0xFFFFFFFF) ); //the bit-clearing may not be really necessary
    }

    operator std::string()
    {
        std::ostringstream s;
        s << "`int48_t 48`";
        s << to64bits();
        return s.str();
    }

    int64_t to64bits() const
    {
        int64_t tmp = 0; //int64 is the smallest standard type capable of holding 48bits
        tmp |= lower32;	//will it sign-extend? It seems sign-extension is NOT performed on the unsigned operand
        tmp |= (uint64_t(upper16) << 32);
        return tmp;
    }

    operator int64_t() const  	//!Implicit, to make use of the type similarly automatic to that of builtin ints
    {
        return to64bits();
    }

    int48_t& operator ++()   //prefix
    {
        *this  = (*this).to64bits() + 1;
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
        *this  = (*this).to64bits() - 1;
        return *this;
    }

    int48_t operator --(int)  	//suffix, int dummy param
    {
        int48_t tmp = *this;
        --(*this);
        return tmp;
    }

    int48_t& operator +=(const int48_t& b)
    {
        *this = (*this).to64bits() + b.to64bits();
        return *this;
    }
    int48_t& operator -=(const int48_t& b)
    {
        *this = (*this).to64bits() - b.to64bits();
        return *this;
    }
    int48_t& operator *=(const int48_t& b)
    {
        *this = (*this).to64bits() * b.to64bits();
        return *this;
    }
    int48_t& operator /=(const int48_t& b)
    {
        *this = (*this).to64bits() / b.to64bits();
        return *this;
    }
    int48_t& operator %=(const int48_t& b)
    {
        *this = (*this).to64bits() % b.to64bits();
        return *this;
    }
    int48_t& operator &=(const int48_t& b)
    {
        *this = (*this).to64bits() bitand b.to64bits();
        return *this;
    }
    int48_t& operator |=(const int48_t& b)
    {
        *this = (*this).to64bits() bitor b.to64bits();
        return *this;
    }
    int48_t& operator ^=(const int48_t& b)
    {
        *this = (*this).to64bits() xor b.to64bits();
        return *this;
    }
    int48_t& operator <<=(const int48_t& b)
    {
        *this = (*this).to64bits() << b.to64bits();
        return *this;
    }
    int48_t& operator >>=(const int48_t& b)
    {
        *this = (*this).to64bits() >> b.to64bits();
        return *this;
    }
    int48_t operator +(const int48_t &x) const
    {
        return (*this).to64bits() + x.to64bits();
    }
    template<class IntType>
    int48_t operator +(IntType n) const
    {
        return (*this).to64bits() + n;
    }
    int48_t operator -(const int48_t &x) const
    {
        return (*this).to64bits() - x.to64bits();    
    }
    template<class IntType>
    int48_t operator -(IntType n) const
    {
        return (*this).to64bits() - n;
    }
    int48_t operator *(const int48_t &x) const
    {
        return (*this).to64bits() * x.to64bits();    
    }
    template<class IntType>	
    int48_t operator *(IntType n) const
    {
        return (*this).to64bits() * n;
    }
    int48_t operator /(const int48_t &x) const
    {
        return (*this).to64bits() / x.to64bits();    
    }
    template<class IntType>
    int48_t operator /(IntType n) const
    {
        return (*this).to64bits() / n;
    }
    int48_t operator %(const int48_t &x) const
    {
        return (*this).to64bits() % x.to64bits();    
    }
    template<class IntType>
    int48_t operator %(IntType n) const
    {
        return (*this).to64bits() % n;
    }
    int48_t operator -() const
    {
        return - (*this).to64bits();
    }
    int48_t& operator ~()
    {
        *this = not (*this).to64bits();
        return *this;
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
      return int48_t(0xFFFFFFFFFFFF);
  }
  static int48_t max() throw()
  {
      return int48_t(0x7FFFFFFFFFFF);
  }
};
}
#endif

