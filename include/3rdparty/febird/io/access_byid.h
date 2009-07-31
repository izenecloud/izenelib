/* vim: set tabstop=4 : */
#ifndef __febird_io_id_generator_h__
#define __febird_io_id_generator_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../config.h"

#include <assert.h>
#include <vector>
#include <map>

#if defined(__GNUC__)
# include <stdint.h>
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
# include <linux/types.h>
#else
# include <sys/types.h>
#endif

#include <boost/current_function.hpp>
#include <boost/intrusive_ptr.hpp>

namespace febird {

class FEBIRD_DLL_EXPORT id_generator
{
	// use free-list structure, same as memory management
	// id may not be 0
	std::vector<uintptr_t> id_list; // id_list[0] is linked list head
	long m_nUsed;

	void chain(long newHead);

public:
	void clear() { id_list.clear(); }

	explicit id_generator(long maxID);
	virtual ~id_generator();

	long alloc_id();	
	void free_id(long id);

	uintptr_t get_val(long id) const
	{
		assert(id >= 1);
		assert(id <= long(id_list.size()-1));
		return id_list[id];
	}

	long add_val(uintptr_t val);	

	bool is_valid(long id) const
	{
		return id >= 1 && id <= long(id_list.size()-1);
	}

	long maxid() const { return id_list.size()-1; }
	long size() const { return m_nUsed; }

	void get_used_id(std::vector<uintptr_t>& used_id) const;
};

// do not allow T to be non-pointer type
template<class T> class access_byid;

template<> class FEBIRD_DLL_EXPORT access_byid<void*> : public id_generator
{
protected:
	virtual void on_destroy(void* vp);
	void* get_ptr_imp(long id, const char* func) const;
	using id_generator::add_val; // hide it

public:
	access_byid(long maxID = 3) : id_generator(maxID) { }

	//! delete all object in list, and clear self
	void destroy();	

	void* get_ptr(long id) const { return access_byid<void*>::get_ptr_imp(id, BOOST_CURRENT_FUNCTION); }
	long  add_ptr(void* x) { return this->add_val((uintptr_t)(x)); }
};

template<class T> class access_byid<T*> : public access_byid<void*>
{
	virtual void on_destroy(void* vp) { delete (T*)vp; }
public:
	access_byid(long maxID = 3) : access_byid<void*>(maxID) { }

	T* get_ptr(long id) const { return access_byid<void*>::get_ptr_imp(id, BOOST_CURRENT_FUNCTION); }
	long add_ptr(T* x) { return this->add_val((uintptr_t)(x)); }
};

template<class T> class AccessByNameID;

template<> class FEBIRD_DLL_EXPORT AccessByNameID<void*>
{
protected:
	access_byid<void*> m_byid;
	std::map<std::string, void*> m_byname;
	virtual void on_destroy(void* vp);
public:
	virtual ~AccessByNameID() {}	
	long add_ptr(void* x, const std::string& name, void** existed);	
	long add_ptr(void* x) { return m_byid.add_ptr(x); }
	void* get_byid(long id) const {	return m_byid.get_ptr(id); }
	void* get_byname(const std::string& name) const;
	bool is_valid(long id) const { return m_byid.is_valid(id); }
	void destroy();

	void remove(long id, const std::string& name);
	void remove(long id);
	long size() const { return m_byid.size(); }
	bool check_id(long id, const char* szClassName, std::string& err) const;
};

template<class T> class AccessByNameID<T*> : public AccessByNameID<void*>
{
	virtual void on_destroy(void* vp)
	{
		delete (T*)vp;
	}
public:
	long add_ptr(T* x, const std::string& name, T** existed)
	{
		return AccessByNameID<void*>::add_ptr(x, name, (void**)existed);
	}
	long add_ptr(T* x) // add without name
	{
		assert(0 != x);
		return m_byid.add_ptr(x);
	}
	T* get_byid(long id) const { return (T*) m_byid.get_ptr(id); }
	T* get_byname(const std::string& name) const
	{
		return (T*)AccessByNameID<void*>::get_byname(name);
	}
};

template<class T>
class AccessByNameID<boost::intrusive_ptr<T> > : public AccessByNameID<void*>
{
	virtual void on_destroy(void* vp)
	{
		intrusive_ptr_release((T*)vp);
	}
public:
	long add_ptr(boost::intrusive_ptr<T> x, const std::string& name, boost::intrusive_ptr<T>* existed)
	{
		assert(0 != x.get());
		T* vpExisted;
		long id = AccessByNameID<void*>::add_ptr(x.get(), name, (void**)&vpExisted);
		if (0 == vpExisted)
		{
			intrusive_ptr_add_ref(x.get());
			*existed = vpExisted;
		}
		return id;
	}
	long add_ptr(boost::intrusive_ptr<T> x) // add without name
	{
		assert(0 != x.get());
		intrusive_ptr_add_ref(x.get());
		return m_byid.add_ptr(x.get());
	}
	long add_ptr(T* x, const std::string& name, T** existed)
	{
		assert(0 != x);
		assert(0 != existed);
		long id = AccessByNameID<void*>::add_ptr(x, name, (void**)existed);
		if (0 == *existed)
			intrusive_ptr_add_ref(x);
		return id;
	}
	long add_ptr(T* x) // add without name
	{
		assert(0 != x);
		intrusive_ptr_add_ref(x);
		return m_byid.add_ptr(x);
	}
	boost::intrusive_ptr<T> get_byid(long id) const { return (T*)m_byid.get_ptr(id); }
	boost::intrusive_ptr<T> get_byname(const std::string& name) const
	{
		return (T*)AccessByNameID<void*>::get_byname(name);
	}
	T* get_rawptr_byid(long id) const
	{
		return (T*)m_byid.get_ptr(id);
	}
	T* get_rawptr_byname(const std::string& name) const
	{
		return (T*)AccessByNameID<void*>::get_byname(name);
	}
	virtual ~AccessByNameID() { destroy(); }
};

} // namespace febird

#endif // __febird_io_id_generator_h__
