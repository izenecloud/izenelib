//
// MessagePack for C++ memory pool
//
// Copyright (C) 2008-2010 FURUHASHI Sadayuki
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
#ifndef MSGPACK_ZONE_HPP__
#define MSGPACK_ZONE_HPP__

#include "auto_zone.hpp"
#include "zone.h"
#include <cstdlib>
#include <memory>
#include <vector>


namespace msgpack {


class zone : public msgpack_zone {
private:
    zone(size_t chunk_size = MSGPACK_ZONE_CHUNK_SIZE);
    ~zone();

public:
    void* malloc(size_t size);
    void* malloc_no_align(size_t size);

    void push_finalizer(void (*func)(void*), void* data);

    template <typename T>
    void push_finalizer(std::auto_ptr<T> obj);

    void clear();

    void swap(zone& o);

    template <typename T, typename... Types>
    T* allocate(Types... args);

private:
    void undo_malloc(size_t size);

    template <typename T>
    static void object_destructor(void* obj);

    typedef msgpack_zone base;

private:
    zone(const zone&);
};



inline zone::zone(size_t chunk_size)
{
    msgpack_zone_init(this, chunk_size);
}

inline zone::~zone()
{
    msgpack_zone_destroy(this);
}

inline void* zone::malloc(size_t size)
{
    void* ptr = msgpack_zone_malloc(this, size);
    if(!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
}

inline void* zone::malloc_no_align(size_t size)
{
    void* ptr = msgpack_zone_malloc_no_align(this, size);
    if(!ptr) {
        throw std::bad_alloc();
    }
    return ptr;
}

inline void zone::push_finalizer(void (*func)(void*), void* data)
{
    if(!msgpack_zone_push_finalizer(this, func, data)) {
        throw std::bad_alloc();
    }
}

template <typename T>
inline void zone::push_finalizer(std::auto_ptr<T> obj)
{
    if(!msgpack_zone_push_finalizer(this, &zone::object_destructor<T>, obj.get())) {
        throw std::bad_alloc();
    }
    obj.release();
}

inline void zone::clear()
{
    msgpack_zone_clear(this);
}

inline void zone::swap(zone& o)
{
    msgpack_zone_swap(this, &o);
}

template <typename T>
void zone::object_destructor(void* obj)
{
    reinterpret_cast<T*>(obj)->~T();
}

inline void zone::undo_malloc(size_t size)
{
    base::chunk_list.ptr  -= size;
    base::chunk_list.free += size;
}

template <typename T, typename... Types>
T* zone::allocate(Types... args)
{
    void* x = malloc(sizeof(T));
    if(!msgpack_zone_push_finalizer(this, &zone::object_destructor<T>, x)) {
        undo_malloc(sizeof(T));
        throw std::bad_alloc();
    }
    try {
        return new (x) T(args...);
    } catch (...) {
        --base::finalizer_array.tail;
        undo_malloc(sizeof(T));
        throw;
    }
}

typedef msgpack::auto_ptr<msgpack_zone> auto_zone;

}  // namespace msgpack

#endif /* msgpack/zone.hpp */
