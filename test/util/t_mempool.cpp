#include <util/MemPool.h>

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include <cstdlib>   // for rand()
#include <cctype>    // for isalnum()   
#include <algorithm> // for back_inserter

#include <boost/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;


BOOST_AUTO_TEST_SUITE( t_mempool )

BOOST_AUTO_TEST_CASE(allocate)
{
/*
    izenelib::util::MemPoolAllocator<4096,4096>& alloc = *(izenelib::util::MemPoolAllocator<4096,4096>::get());
    for(int i = 0; i < 1000000; i ++)
    {
        void * p = alloc.malloc(16);
        alloc.free(p,16);
    }
    alloc.release_memory();
*/	
}

BOOST_AUTO_TEST_SUITE_END()

