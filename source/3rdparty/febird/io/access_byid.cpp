/* vim: set tabstop=4 : */
#include <febird/io/access_byid.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace febird {

	id_generator::id_generator(long maxID)
		: id_list(maxID+1)
	{
		chain(0);
		m_nUsed = 0;
	}
	id_generator::~id_generator()
	{
	}

	void id_generator::chain(long newHead)
	{
		for (long i = newHead; i != static_cast<long>(id_list.size()-1); ++i)
			id_list[i] = i + 1; // id_list[i] link to id_list[i+1]
		id_list.back() = 0; // set 0 as tail
	}

	long id_generator::alloc_id()
	{
		if (0 == id_list[0]) // out of space, enlarge id_list
		{
			long newHead = id_list[0] = id_list.size();
			id_list.resize(id_list.size()*2);
			chain(newHead);
		}
		++m_nUsed;
		long retid = id_list[0];
		id_list[0] = id_list[retid];
		return retid;
	}

	void id_generator::free_id(long id)
	{
		assert(id >= 1);
		assert(id <= long(id_list.size()));
		if (long(id_list.size()) < id || id <= 0)
		{
			throw std::invalid_argument("void id_generator::free_id(long id)");
		}
		id_list[id] = id_list[0];
		id_list[0] = id;
		--m_nUsed;
	}

	long id_generator::add_val(uintptr_t val)
	{
		long id = alloc_id();
		id_list[id] = val;
		return id;
	}

	void id_generator::get_used_id(std::vector<uintptr_t>& used_id) const
	{
		used_id.resize(0);
		used_id.reserve(id_list.size());
		std::vector<long> free_subscript;
		free_subscript.reserve(id_list.size());
		free_subscript.push_back(0);
		for (uintptr_t h = id_list[0]; h != 0; h = id_list[h])
			free_subscript.push_back(h);
		std::sort(free_subscript.begin(), free_subscript.end());
		free_subscript.push_back(id_list.size());
		for (long i = 0; i != static_cast<long>(free_subscript.size()-1); ++i)
		{
			long d1 = free_subscript[i+0]+1;
			long d2 = free_subscript[i+1];
			for (long j = d1; j < d2; ++j)
				used_id.push_back(j);
		}
	}

void* access_byid<void*>::get_ptr_imp(long id, const char* func) const
{
	if (this->is_valid(id))
	{
		assert(this->get_val(id) > 1024);
		return (void*)(this->get_val(id));
	}
	std::ostringstream oss;
	oss << func << ": id too large";
	throw std::invalid_argument(oss.str());
}

void access_byid<void*>::on_destroy(void* vp)
{
	::free(vp);
}

void access_byid<void*>::destroy()
{
	std::vector<uintptr_t> used_id;
	get_used_id(used_id);
	for (long i = 0; i != static_cast<long>(used_id.size()); ++i)
	{
		void* x = (void*)this->get_val(used_id[i]);
		on_destroy(x);
	}
	this->clear();
}

long AccessByNameID<void*>::add_ptr(void* x, const std::string& name, void** existed)
{
	assert(0 != x);
	void*& y = m_byname[name];
	if (y)
	{
		*existed = y;
		return 0;
	}
	else
	{
		*existed = 0;
		y = x;
		long id = m_byid.add_ptr(x);
		return id;
	}
}

void* AccessByNameID<void*>::get_byname(const std::string& name) const
{
	std::map<std::string, void*>::const_iterator iter = m_byname.find(name);
	if (m_byname.end() != iter)
	{
		return iter->second;
	}
	return 0;
}

void AccessByNameID<void*>::on_destroy(void* vp)
{
	::free(vp);
}

	//! delete all object in list, and clear self
void AccessByNameID<void*>::destroy()
{
	std::vector<uintptr_t> used_id;
	m_byid.get_used_id(used_id);
	std::vector<uintptr_t> bynamep(m_byname.size());
	std::map<std::string, void*>::iterator iter = m_byname.begin();
	for (long i = 0; iter != m_byname.end(); ++iter)
		bynamep[i++] = (uintptr_t)iter->second;
	for (long i = 0; i != static_cast<long>(used_id.size()); ++i)
		used_id[i] = m_byid.get_val(used_id[i]);
	std::sort(used_id.begin(), used_id.end());
	std::sort(bynamep.begin(), bynamep.end());
	long n = std::set_union(used_id.begin(), used_id.end(), bynamep.begin(), bynamep.end(), used_id.begin()) - used_id.begin();
	assert(static_cast<long>(used_id.size()) == n);
	for (long i = 0; i != static_cast<long>(used_id.size()); ++i)
	{
		on_destroy((void*)used_id[i]);
	}
	m_byid.clear();
	m_byname.clear();
}

void AccessByNameID<void*>::remove(long id, const std::string& name)
{
	void* p = m_byid.get_ptr(id);
	m_byid.free_id(id);
	m_byname.erase(name);
	on_destroy(p);
}

void AccessByNameID<void*>::remove(long id)
{
	void* p = m_byid.get_ptr(id);
	m_byid.free_id(id);
	on_destroy(p);
}

bool AccessByNameID<void*>::check_id(long id, const char* szClassName, std::string& err) const
{
	if (0 == id)
	{
		assert(0);
	}
	if (!this->is_valid(id))
	{
		std::ostringstream oss;
		oss << "can not find " << szClassName << " object[id=" << id << "]"
			;
		err = oss.str();
		return false;
	}
	return true;
}

} // namespace febird

