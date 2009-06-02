/* vim: set tabstop=4 : */
#ifndef refcount_h__
#define refcount_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "stdtypes.h"
#include <assert.h>
#include <boost/detail/atomic_count.hpp>

namespace febird {

//! for add ref count ability to your class..
//!
//! has no virtual functions, so no v-table,
//! but if Clazz is inherited from 'RefCounter', it will be virtual...
template<class Clazz>
class RefCountable : public Clazz
{
	DECLARE_NONE_COPYABLE_CLASS(RefCountable)
protected:
	boost::detail::atomic_count nRef;
public:
	explicit RefCountable(long nInitRef = 0) : nRef(nInitRef) { }
	long get_refcount() const  { return nRef; }
	friend void intrusive_ptr_add_ref(RefCountable* p) { ++p->nRef; }
	friend void intrusive_ptr_release(RefCountable* p) { if (0 == --p->nRef) delete p; }
};

/**
 @brief 一般 RefCounter，使用虚函数实现，用于 boost::febird_ptr
 */
class FEBIRD_DLL_EXPORT RefCounter
{
	DECLARE_NONE_COPYABLE_CLASS(RefCounter)
	boost::detail::atomic_count nRef;

public:
	explicit RefCounter(long nInitRef = 0) : nRef(nInitRef) { }
	virtual ~RefCounter() {}

	long get_refcount() const  { assert(this); return nRef; }
	void add_ref() { assert(this); ++nRef; }
	void release() { assert(this); if (0 == --nRef) delete this; }

	friend void intrusive_ptr_add_ref(RefCounter* p) { ++p->nRef; }
	friend void intrusive_ptr_release(RefCounter* p) { if (0 == --p->nRef) delete p; }
};

template<class T>
class febird_ptr_ref_cnt_base : public RefCounter
{
public:
	T* p;
	explicit febird_ptr_ref_cnt_base(T* p) : RefCounter(1), p(p) {}
};

template<class T>
class febird_ptr_ref_cnt_auto : public febird_ptr_ref_cnt_base<T>
{
public:
	explicit febird_ptr_ref_cnt_auto(T* p) : febird_ptr_ref_cnt_base<T>(p) {}
	virtual ~febird_ptr_ref_cnt_auto() { delete this->p; }
};

template<class T, class Del>
class febird_ptr_ref_cnt_user : public febird_ptr_ref_cnt_base<T>
{
	Del del;
public:
	febird_ptr_ref_cnt_user(T* p, const Del& del)
		: febird_ptr_ref_cnt_base<T>(p), del(del) {}
	virtual ~febird_ptr_ref_cnt_user() { del(this->p); }
};

template<class T> class febird_ptr
{
private:
    typedef febird_ptr this_type;

	febird_ptr_ref_cnt_base<T>* p;

public:
    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;

	typedef febird_ptr_ref_cnt_base<T>* internal_ptr;

	internal_ptr get_internal_p() const { return p; }

	T* get() const  { return p->p; }

	febird_ptr() : p(0) {}

	template<class Y>
	explicit febird_ptr(Y* y) : p(y ? new febird_ptr_ref_cnt_auto<T>(y) : 0) {}
	explicit febird_ptr(T* x) : p(x ? new febird_ptr_ref_cnt_auto<T>(x) : 0) {}

	template<class Y, class Del>
	febird_ptr(Y* y, const Del& del)
		: p(y ? new febird_ptr_ref_cnt_user<T,Del>(y, del) : 0) {}

	template<class Del>
	febird_ptr(T* x, const Del& del)
		: p(x ? new febird_ptr_ref_cnt_user<T,Del>(x, del) : 0) {}

	febird_ptr(const febird_ptr<T>& x) : p(x.p)
	{
		if (x.p) x.p->add_ref();
	}

	template<class Y>
	febird_ptr(const febird_ptr<Y>& y)
		: p(reinterpret_cast<febird_ptr_ref_cnt_auto<T>*>(y.get_internal_p()))
	{
		// reinterpret_cast is not safe, so check it
		// if not convertible, this line will raise a compile error
		T* check_convertible = (Y*)0;
		if (p) p->add_ref();
	}

	~febird_ptr() {	if (p) p->release(); }

	T* operator->() const { assert(p); return  p->p; }
	T& operator* () const { assert(p); return *p->p; }

	template<class Y>
	void reset(Y* y) { febird_ptr<T>(y).swap(*this); }
	void reset(T* y) { febird_ptr<T>(y).swap(*this); }

	void swap(febird_ptr<T>& y)
	{
		T* t = y.p;
		y.p = this->p;
		this->p = t;
	}
	const febird_ptr<T>& operator=(const febird_ptr<T>& y) 
	{
		febird_ptr<T>(y).swap(*this);
		return *this;
	}
	template<class Y>
	const febird_ptr<T>& operator=(const febird_ptr<Y>& y) 
	{
		febird_ptr<T>(y).swap(*this);
		return *this;
	}
/*
#if defined(__SUNPRO_CC) && BOOST_WORKAROUND(__SUNPRO_CC, <= 0x530)

    operator bool () const
    {
        return p != 0;
    }

#elif defined(__MWERKS__) && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    typedef T * (this_type::*unspecified_bool_type)() const;

    operator unspecified_bool_type() const // never throws
    {
        return p == 0? 0: &this_type::get;
    }

#else 
*/
    typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type () const
    {
        return p == 0? 0: &this_type::p;
    }

//#endif

    // operator! is a Borland-specific workaround
    bool operator! () const
    {
        return p == 0;
    }

	//!{@ caution!!!
	void add_ref() { assert(p); p->add_ref(); }
	void release() { assert(p); p->release(); }
	//@}

	long get_refcount() const { return p ? p->get_refcount() : 0; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const febird_ptr<T>& x)
	{
		assert(x.p);
		dio << *x.p->p;
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, febird_ptr<T>& x)
	{
		x.reset(new T);
		dio >> *x.p->p;
	}
};

template<class T, class U> inline bool operator==(febird_ptr<T> const & a, febird_ptr<U> const & b)
{
    return a.get_internal_p() == b.get_internal_p();
}

template<class T, class U> inline bool operator!=(febird_ptr<T> const & a, febird_ptr<U> const & b)
{
    return a.get_internal_p() != b.get_internal_p();
}

template<class T, class U> inline bool operator==(febird_ptr<T> const & a, U * b)
{
    return a.get_internal_p() == b;
}

template<class T, class U> inline bool operator!=(febird_ptr<T> const & a, U * b)
{
    return a.get_internal_p() != b;
}

template<class T, class U> inline bool operator==(T * a, febird_ptr<U> const & b)
{
    return a == b.get_internal_p();
}

template<class T, class U> inline bool operator!=(T * a, febird_ptr<U> const & b)
{
    return a != b.get_internal_p();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator!=(febird_ptr<T> const & a, febird_ptr<T> const & b)
{
    return a.get_internal_p() != b.get_internal_p();
}

#endif

template<class T> inline bool operator<(febird_ptr<T> const & a, febird_ptr<T> const & b)
{
    return a.get_internal_p() < b.get_internal_p();
}

template<class T> void swap(febird_ptr<T> & lhs, febird_ptr<T> & rhs)
{
    lhs.swap(rhs);
}

// mem_fn support

template<class T> T * get_pointer(febird_ptr<T> const & p)
{
    return p.get();
}

template<class T, class U> febird_ptr<T> static_pointer_cast(febird_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> febird_ptr<T> const_pointer_cast(febird_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}

template<class T, class U> febird_ptr<T> dynamic_pointer_cast(febird_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

} // name space febird

#endif // refcount_h__
