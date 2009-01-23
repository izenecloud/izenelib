/**
 * @file SmartPtr.h
 * @brief The header file of SmartPtr.h.
 *
 * This file defines class RefCount.
 */

#ifndef REFCOUNT_H
#define REFCOUNT_H

#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>

#include "ThreadModel.h"

NS_IZENELIB_UTIL_BEGIN


template<class LockType> struct RefCount {
	int refCount;
	RefCount() :
		refCount(0) {
	}
	void refer() {
		++refCount;
	}
	void unrefer() {
		if (--refCount == 0)
			delete this;
	}
	virtual ~RefCount() {
	}
};

template<> struct RefCount<ReadWriteLock> {
	boost::detail::atomic_count refCount;
	//int refCount;
	RefCount() :
		refCount(0) {
	}
	void refer() {
		++refCount;
	}
	void unrefer() {
		if (--refCount == 0)
			delete this;
	}
	virtual ~RefCount() {
	}
};

inline void intrusive_ptr_add_ref(RefCount<NullLock> * p) {
	if(p)p->refer();
}
inline void intrusive_ptr_release(RefCount<NullLock> * p) {
	if(p)p->unrefer();
}
inline void intrusive_ptr_add_ref(RefCount<ReadWriteLock> * p) {
	if(p)p->refer();
}
inline void intrusive_ptr_release(RefCount<ReadWriteLock> * p) {
	if(p)p->unrefer();
}

NS_IZENELIB_UTIL_END
#endif

