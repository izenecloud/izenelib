#ifndef MSGPACK_TYPE_USTRING_HPP__
#define MSGPACK_TYPE_USTRING_HPP__

#include "../object.hpp"
#include "int.hpp"
#include <string>
#include <util/ustring/UString.h>



namespace msgpack {

inline izenelib::util::UString& operator>> (object o, izenelib::util::UString& uv)
{
    std::string v;
    if(o.type != type::RAW) { throw type_error(); }
    v.assign(o.via.raw.ptr, o.via.raw.size);

    uv.assign(v, izenelib::util::UString::UTF_8); // Fix encoding, by define new object type
    return uv;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const izenelib::util::UString& uv)
{
    std::string v;
    uv.convertString(v, izenelib::util::UString::UTF_8);

    o.pack_raw(v.size());
    o.pack_raw_body(v.data(), v.size());
    return o;
}

inline void operator<< (object::with_zone& o, const izenelib::util::UString& uv)
{
    std::string v;
    uv.convertString(v, izenelib::util::UString::UTF_8);

    o.type = type::RAW;
    o.raw_type = type::RAW_USTRING;
    char* ptr = (char*)o.zone->malloc(v.size());
    o.via.raw.ptr = ptr;
    o.via.raw.size = (uint32_t)v.size();
    memcpy(ptr, v.data(), v.size());
}

inline void operator<< (object& o, const izenelib::util::UString& uv)
{
    std::string v;
    uv.convertString(v, izenelib::util::UString::UTF_8);

    o.type = type::RAW;
    o.raw_type = type::RAW_USTRING;
    o.via.raw.ptr = v.data();
    o.via.raw.size = (uint32_t)v.size();
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
