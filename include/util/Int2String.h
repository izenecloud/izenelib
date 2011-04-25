#ifndef IZENELIB_UTIL_INT2STRING_H
#define IZENELIB_UTIL_INT2STRING_H

//#include <boost/spirit/include/karma.hpp>
#include <util/modp_numtoa.h>
#include <util/izene_serialization.h>

namespace izenelib{ namespace util{

struct Int2String
{
    char x[12];
    size_t len;
public:
    Int2String(int32_t integer)
    {
#if 0  
        ///I can not get boost::spirit::karma::generate work under boost1.38
        ///Perhaps it could work under higer boost version
        using namespace boost::spirit;
        using boost::spirit::karma::generate;
        char *p = x;
        generate(p, int_, integer);
        *p = 0;
#endif
        len = modp_itoa10(integer, x);
    }

    Int2String(uint32_t integer)
    {
#if 0  
        ///I can not get boost::spirit::karma::generate work under boost1.38
        ///Perhaps it could work under higer boost version
        using namespace boost::spirit;
        using boost::spirit::karma::generate;
        char *p = x;
        generate(p, int_, integer);
        *p = 0;
#endif
        len = modp_uitoa10(integer, x);
    }

};

template<> class izene_serialization_memcpy<Int2String> {
    char* x_;
    size_t len_;
public:
    izene_serialization_memcpy(const Int2String& dat)
    {
        x_ = (char*)dat.x;
        len_ = dat.len;
    }
    void write_image(char* &ptr, size_t& size) {
        ptr = x_;
        size = len_;
    }
};

template<> class izene_deserialization_memcpy<Int2String> {
    const char* ptr_;
    const size_t size_;
public:
    izene_deserialization_memcpy(const char* ptr, const size_t size) :
        ptr_(ptr), size_(size) {

    }
    void read_image(Int2String& dat) {
        memcpy(dat.x, ptr_, size_);
    }
};

}}

MAKE_MEMCPY_SERIALIZATION(Int2String);

#endif

