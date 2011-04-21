// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_HASH_MAP_H
#define YIELD_PLATFORM_HASH_MAP_H

#include "yield/platform/string_hash.h"

#include <utility> // For std::make_pair

#if defined(_WIN32)
#include <hash_map>
#elif defined(__GNUC__)
#if !defined(__sun) && ( __GNUC__ >= 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 3 ) )
#include <tr1/unordered_map>
#else
#include <ext/hash_map>
#endif
#else
#error
#endif


namespace YIELD
{
	template <class ValueType>
	class HashMap
	{
	public:
#if defined(_WIN32)
		typedef typename stdext::hash_map<uint32_t, ValueType>::iterator iterator;
#elif defined(__GNUC__)
#if !defined(__sun) && ( __GNUC__ >= 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 3 ) )
		typedef typename std::tr1::unordered_map<uint32_t, ValueType>::iterator iterator;
#else
		typedef typename __gnu_cxx::hash_map<uint32_t, ValueType>::iterator iterator;
#endif
#endif


		size_t size() { return std_hash_map.size(); }
		iterator begin() { return std_hash_map.begin(); }
		iterator end() { return std_hash_map.end(); }

		void insert( const char* key, ValueType value ) { insert( string_hash( key ), value ); }
		ValueType find( const char* key ) { return find( string_hash( key ) ); }
		ValueType find( const char* key, ValueType default_value ) { ValueType value = find( string_hash( key ) ); return value != 0 ? value : default_value; }
		ValueType remove( const char* key ) { return remove( string_hash( key ) ); }

		void insert( uint32_t key, ValueType value )
		{
			std_hash_map.insert( std::make_pair( key, value ) );
		}

		ValueType find( uint32_t key )
		{
			iterator i = std_hash_map.find( key );
			if ( i != std_hash_map.end() )
				return i->second;
			else
				return 0;
		}

		ValueType remove( uint32_t key )
		{
			iterator i = std_hash_map.find( key );
			if ( i != std_hash_map.end() )
			{
				ValueType value = i->second;
				std_hash_map.erase( i );
				return value;
			}
			else
				return 0;
		}

	protected:
#if defined(_WIN32)
		stdext::hash_map<uint32_t, ValueType> std_hash_map;
#elif defined(__GNUC__)
#if !defined(__sun) && ( __GNUC__ >= 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 3 ) )
		std::tr1::unordered_map<uint32_t, ValueType> std_hash_map;
#else
		__gnu_cxx::hash_map<uint32_t, ValueType> std_hash_map;
#endif
#endif
	};
};

#endif
