#ifndef MSGPACK_TYPE_USTRING_HPP__
#define MSGPACK_TYPE_USTRING_HPP__

#include "../object.hpp"
#include "int.hpp"
#include <string>
#include <util/ustring/UString.h>



namespace msgpack {

inline izenelib::util::UString& operator>> (object o, izenelib::util::UString& uv)
{
    if(o.type != type::RAW) { throw type_error(); }
    uv.assign(o.via.raw.size / sizeof(uint16_t), (uint16_t *)o.via.raw.ptr);
    return uv;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const izenelib::util::UString& uv)
{
    o.pack_raw(uv.size());
    o.pack_raw_body((char *)uv.data(), uv.size());
    return o;
}

inline void operator<< (object::with_zone& o, const izenelib::util::UString& uv)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_USTRING;
    char* ptr = (char*)o.zone->malloc(uv.size());
    o.via.raw.ptr = ptr;
    o.via.raw.size = (uint32_t)uv.size();
    memcpy(ptr, uv.data(), uv.size());
}

inline void operator<< (object& o, const izenelib::util::UString& uv)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_USTRING;
    o.via.raw.ptr = (char *)uv.data();
    o.via.raw.size = (uint32_t)uv.size();
}


inline izenelib::util::UString::EncodingType& operator>>(object o, izenelib::util::UString::EncodingType& v)
{
    signed int iv = type::detail::convert_integer<signed int>(o);
    v = static_cast<izenelib::util::UString::EncodingType>(iv);
    return v;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const izenelib::util::UString::EncodingType& v)
    { o.pack_int(static_cast<int>(v)); return o; }

inline void operator<< (object& o, izenelib::util::UString::EncodingType v)
    { o.type = type::POSITIVE_INTEGER, o.via.u64 = static_cast<int>(v); }

inline void operator<< (object::with_zone& o, izenelib::util::UString::EncodingType v)
    { static_cast<object&>(o) << static_cast<int>(v); }

}

#endif /* MSGPACK_TYPE_USTRING_HPP__ */
