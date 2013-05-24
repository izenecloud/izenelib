/*
 * =====================================================================================
 *
 *       Filename:  thread.hpp
 *
 *    Description:  It should be inherited by class which has operator function "void *operator()(void *)" that runs as a thread.
 *
 *        Version:  1.0
 *        Created:  05/03/2011 03:16:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_THREAD_THREAD_HPP_
#define _IZENELIB_THREAD_THREAD_HPP_

#include <types.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <pthread.h>
#include "mutex.h"
#include "condition.h"

namespace izenelib{

static inline int32_t
__attribute__ ((__unused__))
__atomic_exchanged_add(int32_t* __mem, int32_t __val)
{
	register int32_t __result;
	__asm__ __volatile__ ("lock; xaddl %0,%2"
				: "=r" (__result)
				: "0" (__val), "m" (*__mem)
				: "memory");
	return __result;
}

template<class ThreadClass>
struct _threadInfo
{   
	ThreadClass *_thread_ptr;   
	void *_data;   
};   

template<class ThreadClass>
class ThreadCall{   
	public:   
		static void *Fun(void *data)
		{   
			_threadInfo<ThreadClass> *info = (_threadInfo<ThreadClass> *)data;   
			return info->_thread_ptr->operator ()(info->_data);   
		}   
};  

template<
class THREAD_TYPE//It should implement operator()
>
class Thread
{
	typedef Thread<THREAD_TYPE> SelfT;
	pthread_t _pid;
	_threadInfo<THREAD_TYPE> _info; 

	protected:
	private:
	Thread(const SelfT& ){}
	const SelfT& operator = (const SelfT& ){return *this;}

	int run_(THREAD_TYPE* ptr, void* data)
	{
		_info._thread_ptr = ptr;
		_info._data = data;
		return pthread_create(&_pid,NULL,&ThreadCall<THREAD_TYPE>::Fun, &_info);
	}

	public:

	explicit Thread(THREAD_TYPE* ptr, void* data = NULL)
	{
		_pid = 0;
		if (run_(ptr, data)!=0)
		  throw std::runtime_error("Fail to create thread.");
	}

	~Thread()
	{
		join();
	}


	void join()
	{
		if (_pid > 0)
		  pthread_join(_pid, NULL);
		_pid = 0;
	}

	int pid()const
	{
		return _pid;
	}
};

}
#endif

