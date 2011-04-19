// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_SHARED_OBJECT_H
#define YIELD_PLATFORM_SHARED_OBJECT_H

#include "yield/platform/atomic_ops.h"


namespace YIELD
{
	class SharedObject
	{
	public:
		SharedObject() : refcnt( 1 )
		{ }

		inline long getRefcnt() const { return refcnt; }

		template <class SharedObjectType>
		static inline SharedObjectType& incRef( SharedObjectType& shared_object )
		{
			atomic_inc( &shared_object.refcnt );
			return shared_object;
		}

		template <class SharedObjectType>
		static inline SharedObjectType* incRef( SharedObjectType* shared_object )
		{
			if ( shared_object )
				atomic_inc( &shared_object->refcnt );
			return shared_object;
		}

		inline SharedObject& incRef()
		{
			atomic_inc( &this->refcnt );
			return *this;
		}

		static inline void decRef( SharedObject& shared_object )
		{
			if ( atomic_dec( &shared_object.refcnt ) == 0 )
			{
				delete &shared_object;
			}
		}

		static inline void decRef( SharedObject* shared_object )
		{
			if ( shared_object )
				SharedObject::decRef( *shared_object );
		}

	protected:
		virtual ~SharedObject()
		{ }

	private:
		volatile uint32_t refcnt;
	};


	template <class SharedObjectType>
	class auto_SharedObject // Like auto_ptr, but using SharedObject::decRef instead of delete; an operator delete( void* ) on SharedObject doesn't work, because the object is destructed before that call
	{
	public:
		auto_SharedObject( SharedObjectType* shared_object = 0 ) : shared_object( shared_object ) { }
		~auto_SharedObject() { if ( shared_object ) SharedObject::decRef( *shared_object ); }
		auto_SharedObject( const auto_SharedObject<SharedObjectType>& other ) { shared_object = SharedObject::incRef( other.shared_object ); }
		auto_SharedObject& operator=( const auto_SharedObject<SharedObjectType>& other ) { SharedObject::decRef( this->shared_object ); shared_object = SharedObject::incRef( other.shared_object ); return *this; }
		auto_SharedObject& operator=( SharedObjectType* shared_object ) { SharedObject::decRef( this->shared_object ); this->shared_object = SharedObject::incRef( shared_object ); return *this; }

		inline SharedObjectType* get() const { return shared_object; }
		inline SharedObjectType* release() { SharedObjectType* temp_shared_object = shared_object; shared_object = 0; return temp_shared_object; }
		inline void reset( SharedObjectType* shared_object ) { SharedObject::decRef( this->shared_object ); this->shared_object = shared_object; }

	private:
		SharedObjectType* shared_object;
	};
};

#endif
