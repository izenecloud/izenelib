//
// cclog
//
// Copyright (C) 2009 FURUHASHI Sadayuki
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
#ifndef CCLOG_H__
#define CCLOG_H__

#include <sstream>
#include <iostream>


#ifndef CCLOG_LEVEL

#ifdef NDEBUG
#define CCLOG_LEVEL 2
#else
#define CCLOG_LEVEL 0
#endif

#endif

#if CCLOG_LEVEL > 5
#define DISABLE_CCLOG
#endif

class cclog_initializer;

class cclog {
public:
	static void reset(cclog* lg);
	static void destroy();

public:
	static cclog& instance();

public:
	enum level {
		TRACE  = 0,
		DEBUG  = 1,
		INFO   = 2,
		WARN   = 3,
		ERROR  = 4,
		FATAL  = 5,
	};

	cclog(level runtime_level);
	virtual ~cclog();

#define CCLOG_IMPL_BEGIN \
	try { \
		if(lv < m_runtime_level) { return; } \
		std::stringstream s; \
		do { \
			char tmbuf[21]; \
			time_t ti = time(NULL); \
			struct tm t; localtime_r(&ti, &t); \
			s.write(tmbuf, strftime(tmbuf, sizeof(tmbuf), "%Y-%m-%d %H:%M:%S ", &t)); \
		} while(0)

#define CCLOG_IMPL_END \
		std::string str(s.str()); \
		log_impl(lv, str); \
	} catch (...) { \
		std::cerr << prefix << " log error" << std::endl; \
	}

	template <typename A0>
	void log(level lv, const char* prefix, A0 a0) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1>
	void log(level lv, const char* prefix, A0 a0, A1 a1) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13 << a14;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13 << a14 << a15;
		CCLOG_IMPL_END;
	}
	template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	void log(level lv, const char* prefix, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16) {
		CCLOG_IMPL_BEGIN;
		s << prefix << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13 << a14 << a15 << a16;
		CCLOG_IMPL_END;
	}

private:
	virtual void log_impl(level lv, std::string& str) = 0;

private:
	level m_runtime_level;

private:
	friend class cclog_initializer;
	static cclog* s_logger;

private:
	cclog();
	cclog(const cclog&);
};

inline cclog& cclog::instance()
{
	return *s_logger;
}


#include "cclog_null.h"

#ifndef DISABLE_CCLOG
static unsigned long cclog_initializer_counter = 0;

class cclog_initializer {
public:
	cclog_initializer()
	{
		if(0 == cclog_initializer_counter++) {
			if(cclog::s_logger == NULL) {
				cclog::reset(new cclog_null());
			}
		}
	}
	~cclog_initializer()
	{
		if(0 == --cclog_initializer_counter) {
			cclog::destroy();
		}
	}
private:
	void initialize();
};

static cclog_initializer cclog_initializer_;
#endif

#define CCLOG_XSTR(s) #s
#define CCLOG_XSTR_(x) CCLOG_XSTR(x)
#define CCLOG_LINE   CCLOG_XSTR_(__LINE__)

#ifndef CCLOG_PREFIX
#define CCLOG_PREFIX __FILE__ ":" CCLOG_LINE ": "
#endif

#ifndef CCLOG_PREFIX_VERBOSE
#define CCLOG_PREFIX_VERBOSE __FILE__ ":" CCLOG_LINE ":", __FUNCTION__, ": "
#endif

#ifndef CCLOG_PREFIX_TRACE
#define CCLOG_PREFIX_TRACE CCLOG_PREFIX_VERBOSE
#endif
#ifndef CCLOG_PREFIX_DEBUG
#define CCLOG_PREFIX_DEBUG CCLOG_PREFIX_VERBOSE
#endif
#ifndef CCLOG_PREFIX_INFO
#define CCLOG_PREFIX_INFO CCLOG_PREFIX
#endif
#ifndef CCLOG_PREFIX_WARN
#define CCLOG_PREFIX_WARN CCLOG_PREFIX
#endif
#ifndef CCLOG_PREFIX_ERROR
#define CCLOG_PREFIX_ERROR CCLOG_PREFIX
#endif
#ifndef CCLOG_PREFIX_FATAL
#define CCLOG_PREFIX_FATAL CCLOG_PREFIX
#endif

#if CCLOG_LEVEL <= 0
#define LOG_TRACE(...) \
	cclog::instance().log(cclog::TRACE, CCLOG_PREFIX_TRACE, __VA_ARGS__)
#else
#define LOG_TRACE(...) ((void)0)
#endif
#if CCLOG_LEVEL <= 1
#define LOG_DEBUG(...) \
	cclog::instance().log(cclog::DEBUG, CCLOG_PREFIX_DEBUG, __VA_ARGS__)
#else
#define LOG_DEBUG(...) ((void)0)
#endif
#if CCLOG_LEVEL <= 2
#define LOG_INFO(...) \
	cclog::instance().log(cclog::INFO, CCLOG_PREFIX_INFO, __VA_ARGS__)
#else
#define LOG_INFO(...) ((void)0)
#endif
#if CCLOG_LEVEL <= 3
#define LOG_WARN(...) \
	cclog::instance().log(cclog::WARN, CCLOG_PREFIX_WARN, __VA_ARGS__)
#else
#define LOG_WARN(...) ((void)0)
#endif
#if CCLOG_LEVEL <= 4
#define LOG_ERROR(...) \
	cclog::instance().log(cclog::ERROR, CCLOG_PREFIX_ERROR, __VA_ARGS__)
#else
#define LOG_ERROR(...) ((void)0)
#endif
#if CCLOG_LEVEL <= 5
#define LOG_FATAL(...) \
	cclog::instance().log(cclog::FATAL, CCLOG_PREFIX_FATAL, __VA_ARGS__)
#else
#define LOG_FATAL(...) ((void)0)
#endif

#endif /* cclog.h */
