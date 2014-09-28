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
#ifndef MSGPACK_TYPE_DEFINE_HPP__
#define MSGPACK_TYPE_DEFINE_HPP__

#define MSGPACK_DEFINE(...) \
	template <typename Packer> \
	void msgpack_pack(Packer& pk) const \
	{ \
		msgpack::type::make_define(__VA_ARGS__).msgpack_pack(pk); \
	} \
	void msgpack_unpack(msgpack::object o) \
	{ \
		msgpack::type::make_define(__VA_ARGS__).msgpack_unpack(o); \
	}\
	template <typename MSGPACK_OBJECT> \
	void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone* z) const \
	{ \
		msgpack::type::make_define(__VA_ARGS__).msgpack_object(o, z); \
	}

// MSGPACK_ADD_ENUM must be used in the global namespace.
#define MSGPACK_ADD_ENUM(enum) \
  namespace msgpack { \
    template <> \
    inline enum& operator>> (object o, enum& v) \
    { \
      int tmp; \
      o >> tmp; \
      v = static_cast<enum>(tmp); \
      return v; \
    } \
    template <> \
    inline void operator<< (object::with_zone& o, const enum& v) \
    { \
      o << static_cast<int>(v); \
    } \
    namespace detail { \
      template <typename Stream> \
      struct packer_serializer<Stream, enum> { \
        static packer<Stream>& pack(packer<Stream>& o, const enum& v) { \
          return o << static_cast<int>(v); \
        } \
      }; \
    } \
  }

namespace msgpack {
namespace type {



template <typename... Types>
struct define;


template <>
struct define<>
{
    typedef define<> value_type;
    typedef tuple<> tuple_type;

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack_array(0);
    }

    template <typename Packer>
    void msgpack_pack_impl(Packer& pk) const
    {
    }

    void msgpack_unpack(msgpack::object o)
    {
        if (o.type != type::ARRAY) { throw type_error(); }
    }

    void msgpack_unpack_impl(msgpack::object* ptr, msgpack::object* ptr_end)
    {
    }

    void msgpack_object(msgpack::object* o, msgpack::zone* z) const
    {
        o->type = type::ARRAY;
        o->via.array.ptr = NULL;
        o->via.array.size = 0;
    }

    void msgpack_object_impl(msgpack::object* ptr, msgpack::zone* z) const
    {
    }
};

template <typename First, typename... Rest>
struct define<First, Rest...> : define<Rest...>
{
    typedef define<First, Rest...> value_type;
    typedef tuple<First, Rest...> tuple_type;

    define(First& _first, Rest&... _rest) :
        define<Rest...>(_rest...), elem(_first) {}

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        pk.pack_array(sizeof...(Rest) + 1);

        msgpack_pack_impl(pk);
    }

    template <typename Packer>
    void msgpack_pack_impl(Packer& pk) const
    {
        pk.pack(elem);
        define<Rest...>::msgpack_pack_impl(pk);
    }

    void msgpack_unpack(msgpack::object o)
    {
        if (o.type != type::ARRAY) { throw type_error(); }
        msgpack::object *ptr = o.via.array.ptr;
        msgpack::object *ptr_end = o.via.array.ptr + o.via.array.size;
        msgpack_unpack_impl(ptr, ptr_end);
    }

    void msgpack_unpack_impl(msgpack::object* ptr, msgpack::object* ptr_end)
    {
        if (ptr == ptr_end) return;
        ptr->convert(&elem);
        define<Rest...>::msgpack_unpack_impl(ptr + 1, ptr_end);
    }

    void msgpack_object(msgpack::object* o, msgpack::zone* z) const
    {
        o->type = type::ARRAY;
        o->via.array.ptr = (object*)z->malloc(sizeof(object)*(sizeof...(Rest) + 1));
        o->via.array.size = sizeof...(Rest) + 1;

        msgpack_object_impl(o->via.array.ptr, z);
    }

    void msgpack_object_impl(msgpack::object* ptr, msgpack::zone* z) const
    {
        *ptr = object(elem, z);
        define<Rest...>::msgpack_object_impl(ptr + 1, z);
    }

    First& elem;
};


template <typename... Types>
inline define<Types...> make_define(Types&... args)
{
    return define<Types...>(args...);
}


}  // namespace type
}  // namespace msgpack


#endif /* msgpack/type/define.hpp */
