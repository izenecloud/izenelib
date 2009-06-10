#ifndef MAP_H
#define MAP_H

#include <map>

#include <types.h>
#include <3rdparty/boost/memory.hpp>

using namespace std;
using namespace boost::memory;

NS_IZENELIB_UTIL_BEGIN

template<class KeyT, class DataT, class PredT = std::less<KeyT>, class AllocT = scoped_alloc>
class Map : public std::map< KeyT, DataT, PredT, stl_allocator<DataT, AllocT> >
{
private:
	typedef stl_allocator<DataT, AllocT> StlAllocT;
	typedef std::map<KeyT, DataT, PredT, StlAllocT> Base;

public:
	explicit Map(AllocT& alloc, const PredT& pred = PredT())
		: Base(pred, alloc)
	{
	}

	template <class Iterator>
	Map(AllocT& alloc, Iterator first, Iterator last, const PredT& pred = PredT())
		: Base(first, last, pred, alloc)
	{
	}

	void copy(const Base& from) {
		Base::operator=(from);
	}
};


NS_IZENELIB_UTIL_END

#endif
