#ifndef MSGPACK_TYPE_USTRING_HPP__
#define MSGPACK_TYPE_USTRING_HPP__

#include "../object.hpp"
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
    o.via.raw.ptr = v.data();
    o.via.raw.size = (uint32_t)v.size();
}

}

#endif /* MSGPACK_TYPE_USTRING_HPP__ */
