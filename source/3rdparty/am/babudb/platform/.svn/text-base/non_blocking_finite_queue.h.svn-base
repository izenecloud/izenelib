// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_NON_BLOCKING_FINITE_QUEUE_H
#define YIELD_PLATFORM_NON_BLOCKING_FINITE_QUEUE_H

#include "yield/platform/atomic_ops.h"


namespace YIELD
{
#if defined(__LLP64__)
	typedef unsigned long long ptr_as_ulong;
#else
	typedef unsigned long ptr_as_ulong;
#endif

#if defined(__LLP64__) || defined(__LP64__)
#define PTR_HIGH_BIT 0x8000000000000000
#define PTR_LOW_BITS 0x7fffffffffffffff
	template <class ElementType> inline ElementType atomic_cas( volatile ElementType* current_value, ElementType new_value, ElementType old_value ) { return ( ElementType )atomic_cas( ( volatile uint64_t* )current_value, ( uint64_t )new_value, ( uint64_t )old_value ); }
#else
#define PTR_HIGH_BIT 0x80000000
#define PTR_LOW_BITS 0x7fffffff
	template <class ElementType> inline ElementType atomic_cas( volatile ElementType* current_value, ElementType new_value, ElementType old_value ) { return ( ElementType )atomic_cas( ( volatile uint32_t* )current_value, ( uint32_t )new_value, ( uint32_t )old_value ); }
#endif


	template <class ElementType, size_t QueueLength>
	class NonBlockingFiniteQueue
	{
	public:
		NonBlockingFiniteQueue()
		{
			head = 0;
			tail = 1;

			for ( size_t element_i = 0; element_i < QueueLength+2; element_i++ )
				elements[element_i] = ( ElementType )0;

			elements[0] = ( ElementType )1;
		}

		bool enqueue( ElementType element )
		{
//			if ( ( ptr_as_ulong )element & 0x1 ) DebugBreak();
			element = ( ElementType )( ( ptr_as_ulong )element >> 1 );
//			if ( ( ptr_as_ulong )element & PTR_HIGH_BIT ) DebugBreak();

			unsigned long copied_tail, last_try_pos, try_pos; // te, ate, temp
			ElementType try_element;

			while ( true )
			{
				copied_tail = tail;
				last_try_pos = copied_tail;
				try_element = ( ElementType )elements[last_try_pos];
				try_pos = ( last_try_pos + 1 ) % ( QueueLength + 2 );

				while ( try_element != ( ElementType )0 && try_element != ( ElementType )1 )
				{
					if ( copied_tail != tail )
						break;

					if ( try_pos == head )
						break;

					try_element = ( ElementType )elements[try_pos];
					last_try_pos = try_pos;
					try_pos = ( last_try_pos + 1 ) % ( QueueLength + 2 );
				}

				if ( copied_tail != tail ) // Someone changed tail while we were looping
					continue;

				if ( try_pos == head )
				{
					last_try_pos = ( try_pos + 1 ) % ( QueueLength + 2 );
					try_element = ( ElementType )elements[last_try_pos];

					if ( try_element != ( ElementType )0 && try_element != ( ElementType )1 )
						return false;						// Queue is full

					atomic_cas( &head, last_try_pos, try_pos );

					continue;
				}

				if ( copied_tail != tail )
					continue;

				// diff next line
				if ( atomic_cas( &elements[last_try_pos], ( try_element == ( ElementType )1 ) ? ( ElementType )( ( ptr_as_ulong )element | PTR_HIGH_BIT ) : element, try_element ) == try_element )
				{
					if ( try_pos % 2 == 0 )
						atomic_cas( &tail, try_pos, copied_tail );

					return true;
				}
			}
		}

		ElementType try_dequeue()
		{
			unsigned long copied_head, try_pos;
			ElementType try_element;

			while ( true )
			{
				copied_head = head;
				try_pos = ( copied_head + 1 ) % ( QueueLength + 2 );
				try_element = ( ElementType )elements[try_pos];

				while ( try_element == ( ElementType )0 || try_element == ( ElementType )1 )
				{
					if ( copied_head != head )
						break;

					if ( try_pos == tail )
						return 0;

					try_pos = ( try_pos + 1 ) % ( QueueLength + 2 );

					try_element = ( ElementType )elements[try_pos];
				}

				if ( copied_head != head )
					continue;

				if ( try_pos == tail )
				{
					atomic_cas( &tail, ( try_pos + 1 ) % ( QueueLength + 2 ), try_pos );
					continue;
				}

				if ( copied_head != head )
					continue;

				if ( atomic_cas( &elements[try_pos], ( ( ptr_as_ulong )try_element & ( ptr_as_ulong )0x80000000 ) ? ( ElementType )1 : ( ElementType )0, try_element ) == try_element )
				{
					if ( try_pos % 2 == 0 )
						atomic_cas( &head, try_pos, copied_head );

					return ( ElementType )( ( ( ptr_as_ulong )try_element & PTR_LOW_BITS ) << 1 );
				}
			}
			return 0;
		}

	private:
		volatile ElementType elements[QueueLength+2]; // extra 2 for sentinels
		volatile unsigned long head, tail;
	};
};

#endif
