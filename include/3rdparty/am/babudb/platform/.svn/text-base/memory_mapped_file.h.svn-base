// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_MEMORY_MAPPED_FILE_H
#define YIELD_PLATFORM_MEMORY_MAPPED_FILE_H

#include "yield/platform/file.h"


namespace YIELD
{
	class MemoryMappedFile : public File
	{
	public:
		MemoryMappedFile( const Path& path, size_t minimum_size = 0, unsigned long flags = O_RDWR|O_SYNC );
		virtual ~MemoryMappedFile() { close(); }

		void resize( size_t );
		inline char* getRegionStart() { return start; }
		inline char* getRegionEnd() { return start + size; }
		inline size_t getRegionSize() { return size; }

		virtual void writeBack();
		virtual void writeBack( size_t offset, size_t length );
		virtual void writeBack( void* ptr, size_t length );

		// File
		bool close();

	private:
		size_t size;
		char* start;
#ifdef _WIN32
		void* mapping;
#endif
	};
};

#endif
