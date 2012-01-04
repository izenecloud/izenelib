//
// MessagePack for C++ static resolution routine
//
// Copyright (C) 2008-2009 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_TYPE_INT128_HPP__
#define MSGPACK_TYPE_INT128_HPP__

#include "../object.hpp"
#include <types.h>

namespace msgpack {


inline int128_t& operator>> (object o, int128_t& v)
{
    if (o.type != type::RAW) { throw type_error(); }
    memcpy(&v, o.via.raw.ptr, sizeof(int128_t));
    return v;
}

inline uint128_t& operator>> (object o, uint128_t& v)
{
    if (o.type != type::RAW) { throw type_error(); }
    memcpy(&v, o.via.raw.ptr, sizeof(uint128_t));
    return v;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const int128_t& v)
{
    o.pack_raw(sizeof(int128_t));
    o.pack_raw_body((char *) &v, sizeof(int128_t));
    return o;
}

template <typename Stream>
inline packer<Stream>& operator<< (packer<Stream>& o, const uint128_t& v)
{
    o.pack_raw(sizeof(uint128_t));
    o.pack_raw_body((char *) &v, sizeof(uint128_t));
    return o;
}

inline void operator<< (object& o, int128_t v)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_INT128;
    o.via.raw.ptr = (char *) &v;
    o.via.raw.size = sizeof(int128_t);
}

inline void operator<< (object& o, uint128_t v)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_UINT128;
    o.via.raw.ptr = (char *) &v;
    o.via.raw.size = sizeof(uint128_t);
}

inline void operator<< (object::with_zone& o, int128_t v)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_INT128;
    char* ptr = (char*)o.zone->malloc(sizeof(int128_t));
    o.via.raw.ptr = ptr;
    o.via.raw.size = sizeof(int128_t);
    memcpy(ptr, &v, sizeof(int128_t));
}

inline void operator<< (object::with_zone& o, uint128_t v)
{
    o.type = type::RAW;
    o.raw_type = type::RAW_UINT128;
    char* ptr = (char*)o.zone->malloc(sizeof(uint128_t));
    o.via.raw.ptr = ptr;
    o.via.raw.size = sizeof(uint128_t);
    memcpy(ptr, &v, sizeof(uint128_t));
}

}  // namespace msgpack

#endif /* msgpack/type/int.hpp */
