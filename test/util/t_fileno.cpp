#include <boost/test/unit_test.hpp>

#include <util/fileno.hpp>
#include <unistd.h>
#include <fcntl.h>

BOOST_AUTO_TEST_SUITE(t_fileno_util)

BOOST_AUTO_TEST_CASE(normal_test)
{
    const char* TEST_FILE = "./test.out";
    std::ofstream file(TEST_FILE, std::ios::out);
    int fd = izenelib::util::fileno(file);
    posix_fadvise(fd, 0,0,POSIX_FADV_DONTNEED);	
    BOOST_CHECK(fd);
}


BOOST_AUTO_TEST_SUITE_END()
