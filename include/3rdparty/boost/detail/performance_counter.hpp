//
//  boost/detail/performance_counter.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_PERFORMANCE_COUNTER_HPP
#define BOOST_DETAIL_PERFORMANCE_COUNTER_HPP

#ifndef BOOST_DETAIL_WINAPI_WINBASE_H
#include <boost/detail/winapi/winbase.h>
#endif

#ifndef BOOST_DETAIL_LOG_HPP
#include "log.hpp"
#endif

#ifndef BOOST_DETAIL_DEBUG_HPP
#include "debug.hpp"
#endif

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------

template <class UIntT, class CharT>
inline CharT* BOOST_DETAIL_CALL _ui2estr(UIntT val, CharT* estr)
{
	*estr = '\0';
	do {
		*--estr = '0' + val % 10;
		val /= 10;
	}
	while (val);
	return estr;
}

// -------------------------------------------------------------------------
// class performance_counter

class performance_counter
{
public:
	typedef UINT64 value_type;

private:
	value_type m_tick;
	
private:
	static value_type BOOST_DETAIL_CALL _get_freq()
	{
		value_type _freq = 0;
		QueryPerformanceFrequency(&(LARGE_INTEGER&)_freq);
		if (_freq == 0)
		{
			BOOST_DETAIL_ASSERT(!"QueryPerformanceFrequency is unsupported\n");
			_freq = 1;
		}
		return _freq;
	}
	
public:
	performance_counter()
	{
		freq();
		start();
	}

	static value_type BOOST_DETAIL_CALL freq()
	{
		static value_type s_freq = _get_freq();
		return s_freq;
	}

	void BOOST_DETAIL_CALL start()
	{
		QueryPerformanceCounter(&(LARGE_INTEGER&)m_tick);
	}

	__forceinline value_type BOOST_DETAIL_CALL duration() const
	{
		value_type tickNow;
		QueryPerformanceCounter(&(LARGE_INTEGER&)tickNow);
		return tickNow - m_tick;
	}

public:
	template <class LogT>
	static void BOOST_DETAIL_CALL trace(LogT& log, const value_type& ticks)
	{
		double msVal = (INT64)ticks * 1000.0 / (INT64)freq();
		char szTicks[32];
		log.trace(
			"---> Elapse %s ticks (%.2lf ms) (%.2lf min) ...\n",
			_ui2estr(ticks, szTicks + sizeof(szTicks)), msVal, msVal/60000.0
			);
	}

	template <class LogT>
	value_type BOOST_DETAIL_CALL trace(LogT& log)
	{
		value_type dur = duration();
		trace(log, dur);
		return dur;
	}
};

// -------------------------------------------------------------------------
// class accumulator

template <class CounterT = performance_counter>
class accumulator_imp
{
public:
	typedef typename CounterT::value_type value_type;
	typedef size_t size_type;
	
private:
	value_type m_acc;
	size_type m_count;

public:
	accumulator_imp() {
		m_acc = 0;
		m_count = 0;
	}
	
	void BOOST_DETAIL_CALL start() {
		m_acc = 0;
		m_count = 0;
	}
	
	size_type BOOST_DETAIL_CALL count() const {
		return m_count;
	}
	
	const value_type& BOOST_DETAIL_CALL accumulate() const {
		return m_acc;
	}

	void BOOST_DETAIL_CALL accumulate(const value_type& val) {
		m_acc += val;
		++m_count;
	}
	
	__forceinline void BOOST_DETAIL_CALL accumulate(const CounterT& counter) {
		m_acc += counter.duration();
		++m_count;
	}
	
	template <class LogT>
	void BOOST_DETAIL_CALL trace_avg(LogT& log) {
		log.trace("Average: ");
		CounterT::trace(log, m_acc/m_count);
	}

	template <class LogT>
	void BOOST_DETAIL_CALL trace(LogT& log) {
		CounterT::trace(log, m_acc);
	}
};

typedef accumulator_imp<> accumulator;

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_PERFORMANCE_COUNTER_HPP */
