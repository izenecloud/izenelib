/**
   @file UString.h
   @author Kevin Hu
   @date  2009.11.24
 */
#ifndef IZENELIB_UTIL_USTRING_H
#define IZENELIB_UTIL_USTRING_H

#include <util/izene_serialization.h>
#include "vector_string.hpp"

/* namespace wiselib { */
/*   typedef istring UString; */

/* } */


MAKE_FEBIRD_SERIALIZATION(izenelib::util::UString);

//NS_IZENELIB_UTIL_BEGIN
/*
template<>
inline void write_image_memcpy< izenelib::util::UString>(const izenelib::util::UString& dat, char* &str, size_t& size) {

  size = sizeof(izenelib::util::UString::size_t) + dat.length()*sizeof(izenelib::util::UString::value_type);
    str = new char[size];
    char* p=str;
    izenelib::util::UString::size_t length = dat.length();

    memcpy(p, &length, sizeof(izenelib::util::UString::size_t));

    p += sizeof(izenelib::util::UString::size_t);
    memcpy(p, dat.str_, length*sizeof(izenelib::util::UString::value_type));

}

template<>
inline void read_image_memcpy< izenelib::util::UString >(izenelib::util::UString& dat, const char* str,
        const size_t size) {
    char* p = (char*)str;
    izenelib::util::UString::size_t length;// = *(izenelib::util::UString::size_t*)str;

    memcpy(&length, p, sizeof(izenelib::util::UString::size_t));
    p += sizeof(izenelib::util::UString::size_t);
    dat.setBuffer(length, length*sizeof(izenelib::util::UString::value_type), p);
}*/


//template<>
//class izene_serialization_memcpy<izenelib::util::UString>
//{
//    const izenelib::util::UString& dat_;
//public:
//    izene_serialization_memcpy(const izenelib::util::UString& dat)
//        : dat_(dat)
//    {
//    }
//    void write_image(char* &ptr, size_t& size)
//    {
//        ptr = (char*) dat_.c_str();
//        size = dat_.size();
//    }
//};
//
//template<>
//class izene_deserialization_memcpy<izenelib::util::UString>
//{
//    const char* ptr_;
//    const size_t size_;
//public:
//    izene_deserialization_memcpy(const char* ptr, const size_t size)
//        : ptr_(ptr), size_(size)
//    {
//    }
//    void read_image(izenelib::util::UString& dat)
//    {
//        dat.assign(std::string(ptr_, size_), izenelib::util::UString::UTF_8);
//    }
//};
//
//
//NS_IZENELIB_UTIL_END


#endif
