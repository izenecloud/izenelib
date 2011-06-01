#include <string>
#include <memory>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <cstdio>

#include <boost/memory.hpp>

using namespace std;
using boost::stl_allocator;

void testAutoAlloc()
{
	boost::auto_alloc alloc;
	int* intObj = BOOST_NEW(alloc, int);
	int* intObjWithArg = BOOST_NEW(alloc, int)(10);
	int* intArray = BOOST_NEW_ARRAY(alloc, int, 100);
	int* intBuf = BOOST_ALLOC(alloc, int);
	int* intArrayBuf = BOOST_ALLOC_ARRAY(alloc, int, 100);

	boost::auto_alloc* subAlloc = BOOST_NEW(alloc, boost::auto_alloc);
	int* e = BOOST_NEW(*subAlloc, int);
}

void testScopedAlloc()
{
	NS_BOOST_MEMORY::block_pool recycle;
	boost::scoped_alloc alloc(recycle);

	int* intObj = BOOST_NEW(alloc, int);
	int* intObjWithArg = BOOST_NEW(alloc, int)(10);
	int* intArray = BOOST_NEW_ARRAY(alloc, int, 100);
	int* intBuf = BOOST_ALLOC(alloc, int);
	int* intArrayBuf = BOOST_ALLOC_ARRAY(alloc, int, 100);

	boost::scoped_alloc* suballoc = BOOST_NEW(alloc, boost::scoped_alloc)(alloc);
	int* e = BOOST_NEW(*suballoc, int);
}

void testTlsScopedAlloc()
{
	boost::scoped_alloc alloc;
	// same as: boost::scoped_alloc(boost::tls_block_pool::instance());

	int* intObj = BOOST_NEW(alloc, int);
	int* intObjWithArg = BOOST_NEW(alloc, int)(10);
	int* intArray = BOOST_NEW_ARRAY(alloc, int, 100);
	int* intBuf = BOOST_ALLOC(alloc, int);
	int* intArrayBuf = BOOST_ALLOC_ARRAY(alloc, int, 100);

	boost::scoped_alloc* suballoc = BOOST_NEW(alloc, boost::scoped_alloc);
	int* e = BOOST_NEW(*suballoc, int);
}

void simpleExamples()
{
	testAutoAlloc();
	testScopedAlloc();
	testTlsScopedAlloc();
}
enum { Count = 100000 };

void testDeque()
{
	printf("\n===== Deque (scoped_alloc) =====\n");
	boost::scoped_alloc alloc;
	std::deque<int, stl_allocator<int> > s(alloc);
	for (int i = 0; i < Count; ++i)
		s.push_back(i);
}

void testList()
{
	printf("\n===== List (scoped_alloc) =====\n");
	boost::scoped_alloc alloc;
	std::list<int, stl_allocator<int> > s(alloc);
	for (int i = 0; i < Count; ++i)
		s.push_back(i);
}

void testSet()
{
	printf("\n===== Set (scoped_alloc) =====\n");
	boost::scoped_alloc alloc;
	std::set<int, std::less<int>, stl_allocator<int> > s(std::less<int>(), alloc);
	for (int i = 0; i < Count; ++i)
		s.insert(i);
}

void testMap()
{
	printf("\n===== Map (scoped_alloc) =====\n");
	boost::scoped_alloc alloc;
	std::map<int, int, std::less<int>, stl_allocator<int> > s(std::less<int>(), alloc);
	for (int i = 0; i < Count; ++i)
		s.insert(std::pair<int, int>(i, i));
}

void testStlContainers()
{
	testDeque();
	testList();
	testSet();
	testMap();
}

int main()
{
	testStlContainers();
	simpleExamples();

    return 0;
}

