//
// mpio sync
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
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
#ifndef MP_SYNC_H__
#define MP_SYNC_H__

#include "pthread.h"

namespace mp {


template <typename T>
class sync {
public:
	sync() : m_obj() { }
	template <typename A1>
	sync(A1 a1) : m_obj(a1) { }
	template <typename A1, typename A2>
	sync(A1 a1, A2 a2) : m_obj(a1, a2) { }
	template <typename A1, typename A2, typename A3>
	sync(A1 a1, A2 a2, A3 a3) : m_obj(a1, a2, a3) { }
	template <typename A1, typename A2, typename A3, typename A4>
	sync(A1 a1, A2 a2, A3 a3, A4 a4) : m_obj(a1, a2, a3, a4) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) : m_obj(a1, a2, a3, a4, a5) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) : m_obj(a1, a2, a3, a4, a5, a6) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) : m_obj(a1, a2, a3, a4, a5, a6, a7) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) { }
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	sync(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16) : m_obj(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) { }

	~sync() { }

	T& unsafe_ref() { return m_obj; }
	const T& unsafe_ref() const { return m_obj; }

	class ref {
	public:
		ref(sync<T>& obj) : m_ref(NULL)
		{
			obj.m_mutex.lock();
			m_ref = &obj;
		}
	
		ref() : m_ref(NULL) { }
	
		~ref() { reset(); }
	
		void reset()
		{
			if(m_ref) {
				m_ref->m_mutex.unlock();
				m_ref = NULL;
			}
		}
	
		void reset(sync<T>& obj)
		{
			reset();
			obj.m_mutex.lock();
			m_ref = &obj;
		}

		void swap(sync<T>& obj)
		{
			sync<T>* tmp = m_ref;
			m_ref = obj.m_ref;
			obj.m_ref = tmp;
		}
	
		T& operator*() { return m_ref->m_obj; }
		T* operator->() { return &operator*(); }
		const T& operator*() const { return m_ref->m_obj; }
		const T* operator->() const { return &operator*(); }

		operator bool() const { return m_ref != NULL; }

		pthread_mutex& get_mutex()
		{
			return m_ref->m_mutex;
		}
	
	protected:
		sync<T>* m_ref;
	
	private:
		ref(const ref&);
	};

	class auto_ref : public ref {
	public:
		auto_ref(sync<T>& obj) : ref(obj) { }
		auto_ref() { }
		~auto_ref() { }

		auto_ref(auto_ref& o)
		{
			ref::m_ref = o.m_ref;
			o.m_ref = NULL;
		}

		auto_ref& operator= (auto_ref& o)
		{
			auto_ref(o).swap(*this);
			return *this;
		}
	};

	auto_ref lock()
	{
		return auto_ref(*this);
	}

private:
	T m_obj;
	pthread_mutex m_mutex;
	friend class ref;

private:
	sync(const sync&);
};


}  // namespace mp

#endif /* mp/sync.h */

