//
// mpio object_callback
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
#ifndef MP_OBJECT_CALLBACK_H__
#define MP_OBJECT_CALLBACK_H__

#include "memory.h"
#include "object_delete.h"

namespace mp {


template <typename Signature>
struct object_callback;

template <typename R, typename... Types>
struct object_callback<R(Types...)>
{
    template <typename T, R (T::*MemFun)(Types... args)>
    static R mem_fun(void* obj, Types... args)
    {
        return (reinterpret_cast<T*>(obj)->*MemFun)(args...);
    }

    template <typename T, R (T::*MemFun)(Types... args)>
    static R const_mem_fun(const void* obj, Types... args)
    {
        return (reinterpret_cast<const T*>(obj)->*MemFun)(args);
    }

    template <typename T, R (T::*MemFun)(Types... args)>
    static R shared_fun(shared_ptr<T> obj, Types... args)
    {
        return (obj.get()->*MemFun)(args...);
    }
};


}  // namespace mp

#endif /* mp/object_callback.h */
