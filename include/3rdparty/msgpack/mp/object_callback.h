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

template <typename R>
struct object_callback<R ()>
{
	template <typename T, R (T::*MemFun)()>
	static R mem_fun(void* obj)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)();
	}

	template <typename T, R (T::*MemFun)()>
	static R const_mem_fun(const void* obj)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)();
	}

	template <typename T, R (T::*MemFun)()>
	static R shared_fun(shared_ptr<T> obj)
	{
		return (obj.get()->*MemFun)();
	}
};

template <typename R, typename A1>
struct object_callback<R (A1 a1)>
{
	template <typename T, R (T::*MemFun)(A1 a1)>
	static R mem_fun(void* obj, A1 a1)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1);
	}

	template <typename T, R (T::*MemFun)(A1 a1)>
	static R const_mem_fun(const void* obj, A1 a1)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1);
	}

	template <typename T, R (T::*MemFun)(A1 a1)>
	static R shared_fun(shared_ptr<T> obj, A1 a1)
	{
		return (obj.get()->*MemFun)(a1);
	}
};

template <typename R, typename A1, typename A2>
struct object_callback<R (A1 a1, A2 a2)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2)>
	static R mem_fun(void* obj, A1 a1, A2 a2)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2)
	{
		return (obj.get()->*MemFun)(a1, a2);
	}
};

template <typename R, typename A1, typename A2, typename A3>
struct object_callback<R (A1 a1, A2 a2, A3 a3)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3)
	{
		return (obj.get()->*MemFun)(a1, a2, a3);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
	}
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
struct object_callback<R (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)>
{
	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)>
	static R mem_fun(void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
	{
		return (reinterpret_cast<T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)>
	static R const_mem_fun(const void* obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
	{
		return (reinterpret_cast<const T*>(obj)->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
	}

	template <typename T, R (T::*MemFun)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)>
	static R shared_fun(shared_ptr<T> obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
	{
		return (obj.get()->*MemFun)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
	}
};



}  // namespace mp

#endif /* mp/object_callback.h */

