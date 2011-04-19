// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_PROCESSOR_SET_H
#define YIELD_PLATFORM_PROCESSOR_SET_H

#include "yield/platform/platform_config.h"


namespace YIELD
{
	class ProcessorSet
	{
	public:
		ProcessorSet();
		~ProcessorSet();

		void set( unsigned short processor_i );
		bool isset( unsigned short processor_i );
		void clear( unsigned short processor_i );

	private:
		friend class Process;
		friend class Thread;

#if defined(_WIN32)
		unsigned long mask;
#elif defined(__linux)
		void* cpu_set;
#elif defined(__sun)
		int psetid;
#endif
	};
};

#endif
