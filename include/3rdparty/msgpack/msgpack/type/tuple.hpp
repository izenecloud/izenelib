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
#ifndef MSGPACK_TYPE_TUPLE_HPP__
#define MSGPACK_TYPE_TUPLE_HPP__

#include "../object.hpp"

namespace msgpack {

namespace type {

// FIXME operator==
// FIXME operator!=


template <typename... Types>
struct tuple;

template <size_t N, typename Tuple>
struct tuple_element;

template <size_t N, typename Tuple>
struct const_tuple_element;

template <typename T>
struct tuple_type
{
    typedef T type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef const T& transparent_reference;
};

template <typename T>
struct tuple_type<T&>
{
    typedef T type;
    typedef T& value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T& transparent_reference;
};

template <typename T>
struct tuple_type<const T&>
{
    typedef T type;
    typedef T& value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef const T& transparent_reference;
};

template <size_t N, typename First, typename... Rest>
struct tuple_element<N, tuple<First, Rest...> > : tuple_element<N - 1, tuple<Rest...> >
{
    tuple_element(tuple<First, Rest...>& x) : tuple_element<N - 1, tuple<Rest...> >(static_cast<tuple<Rest...>&>(x)) {}
};

template <typename First, typename... Rest>
struct tuple_element<0, tuple<First, Rest...> > : tuple_type<First>
{
    tuple_element(tuple<First, Rest...>& x) : _x(x.elem) {}
    typename tuple_type<First>::reference get() { return _x; }
    typename tuple_type<First>::const_reference get() const { return _x; }
private:
    typename tuple_type<First>::reference _x;
};

template <size_t N, typename First, typename... Rest>
struct const_tuple_element<N, tuple<First, Rest...> > : const_tuple_element<N - 1, tuple<Rest...> >
{
    const_tuple_element(const tuple<First, Rest...>& x) : const_tuple_element<N - 1, tuple<Rest...> >(static_cast<const tuple<Rest...>&>(x)) {}
};

template <typename First, typename... Rest>
struct const_tuple_element<0, tuple<First, Rest...> > : tuple_type<First>
{
    const_tuple_element(const tuple<First, Rest...>& x) : _x(x.elem) {}
    typename tuple_type<First>::const_reference get() const { return _x; }
private:
    typename tuple_type<First>::const_reference _x;
};

template <>
struct tuple<>
{
    tuple() {}
    tuple(object o) { o.convert(this); }
    typedef tuple<> value_type;
};

template <typename First, typename... Rest>
struct tuple<First, Rest...> : tuple<Rest...>
{
    typedef tuple<First, Rest...> value_type;
    tuple() {}
    tuple(typename tuple_type<First>::transparent_reference _first, typename tuple_type<Rest>::transparent_reference... _rest) :
        tuple<Rest...>(_rest...), elem(_first) {}
    tuple(object o) { o.convert(this); }

    template <size_t N> typename tuple_element<N, value_type>::reference get()
    { return tuple_element<N, value_type>(*this).get(); }
    template <size_t N> typename const_tuple_element<N, value_type>::const_reference get() const
    { return const_tuple_element<N, value_type>(*this).get(); }

    First elem;
};

template <typename... Types>
tuple<Types...> make_tuple(typename tuple_type<Types>::transparent_reference... args)
{
    return tuple<Types...>(args...);
}


}  // namespace type


namespace {

template <typename... Types>
void read_tuple_impl(object*, type::tuple<Types...>&);

template <>
void read_tuple_impl(object* ptr, type::tuple<>& v)
{
}

template <typename First, typename... Rest>
void read_tuple_impl(object* ptr, type::tuple<First, Rest...>& v)
{
    ptr->convert<typename type::tuple_type<First>::type>(&v.template get<0>());
    read_tuple_impl(ptr + 1, static_cast<type::tuple<Rest...>&>(v));
}

template <typename Stream, typename... Types>
void packer_write_tuple_impl(packer<Stream>&, const type::tuple<Types...>&);

template <typename Stream>
void packer_write_tuple_impl(packer<Stream>& o, const type::tuple<>& v)
{
}

template <typename Stream, typename First, typename... Rest>
void packer_write_tuple_impl(packer<Stream>& o, const type::tuple<First, Rest...>& v)
{
    o.pack(v.template get<0>());
    packer_write_tuple_impl(o, static_cast<const type::tuple<Rest...>&>(v));
}

template <typename... Types>
void write_tuple_impl(object*, zone*, const type::tuple<Types...>&);

template <>
void write_tuple_impl(object* ptr, zone* z, const type::tuple<>& v)
{
}

template <typename First, typename... Rest>
void write_tuple_impl(object* ptr, zone* z, const type::tuple<First, Rest...>& v)
{
    *ptr = object(v.template get<0>(), z);
    write_tuple_impl(ptr + 1, z, static_cast<const type::tuple<Rest...>&>(v));
}

}

inline type::tuple<>& operator>>(object o, type::tuple<>& v)
{
    if (o.type != type::ARRAY) { throw type_error(); }
    return v;
}

template <typename First, typename... Rest>
type::tuple<First, Rest...>& operator>>(object o, type::tuple<First, Rest...>& v)
{
    if (o.type != type::ARRAY) { throw type_error(); }
    if (o.via.array.size < sizeof...(Rest) + 1) { throw type_error(); }

    read_tuple_impl(o.via.array.ptr, v);
    return v;
}

template <typename Stream>
const packer<Stream>& operator<<(packer<Stream>& o, const type::tuple<>& v)
{
    o.pack_array(0);
    return o;
}

template <typename Stream, typename First, typename... Rest>
const packer<Stream>& operator<<(packer<Stream>& o, const type::tuple<First, Rest...>& v)
{
    o.pack_array(sizeof...(Rest) + 1);

    packer_write_tuple_impl(o, v);
    return o;
}

inline void operator<<(object::with_zone& o, const type::tuple<>& v)
{
    o.type = type::ARRAY;
    o.via.array.ptr = NULL;
    o.via.array.size = 0;
}

template <typename First, typename... Rest>
void operator<<(object::with_zone& o, const type::tuple<First, Rest...>& v)
{
    o.type = type::ARRAY;
    o.via.array.ptr = (object*)o.zone->malloc(sizeof(object)*(sizeof...(Rest) + 1));
    o.via.array.size = sizeof...(Rest) + 1;

    write_tuple_impl(o.via.array.ptr, o.zone, v);
}


}  // namespace msgpack


#endif /* msgpack/type/tuple.hpp */
