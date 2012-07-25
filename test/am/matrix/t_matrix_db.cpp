#include <am/matrix/matrix_db.h>


#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

using namespace izenelib::am;
using namespace std;

namespace bfs = boost::filesystem;

const char* MATRIX_TEST_DIR_STR = "matrix";

BOOST_AUTO_TEST_SUITE(matrix_test)

BOOST_AUTO_TEST_CASE(update_elem)
{
    bfs::path matrix_path(MATRIX_TEST_DIR_STR);
    boost::filesystem::remove_all(matrix_path);
    bfs::create_directories(matrix_path);
    std::string matrix_path_str = matrix_path.string();

    MatrixDB<uint32_t, uint32_t> matrix(1024*2, matrix_path_str+"/test.db");
    size_t colNum = 100;
    size_t rowNum = 100;
    boost::mt19937 engine ;
    boost::uniform_int<> distribution(1, 10000) ;
    boost::variate_generator<boost::mt19937, boost::uniform_int<> > random(engine, distribution);

    uint32_t v = 0;
    for(size_t i = 0; i < rowNum; ++i)
        for(size_t j = 0; j < colNum; ++j)
            matrix.update_elem(i, j, v++);

    BOOST_TEST_MESSAGE(matrix);

    v = 0;
    for(size_t i = 0; i < rowNum; ++i)
        for(size_t j = 0; j < colNum; ++j)
        {
            uint32_t e = matrix.elem(i, j);
            BOOST_CHECK_EQUAL(e	,v++);
        }
}

BOOST_AUTO_TEST_CASE(dump)
{
    bfs::path matrix_path(MATRIX_TEST_DIR_STR);
    std::string matrix_path_str = matrix_path.string();

    MatrixDB<uint32_t, uint32_t> matrix(1024, matrix_path_str+"/test.db");
    size_t colNum = 100;
    size_t rowNum = 100;

    uint32_t v = 0;
    for(size_t i = 0; i < rowNum; ++i)
        for(size_t j = 0; j < colNum; ++j)
        {
            uint32_t e = matrix.elem(i, j);
            BOOST_CHECK_EQUAL(e	,v++);
        }

    BOOST_TEST_MESSAGE(matrix);
}

BOOST_AUTO_TEST_SUITE_END()
