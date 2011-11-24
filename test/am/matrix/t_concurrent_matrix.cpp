///
/// @file t_concurrent_matrix.cpp
/// @brief test functions in ConcurrentMatrix
/// @author Jun Jiang
/// @date Created 2011-11-24
///

#include <am/matrix/concurrent_matrix.h>
#include <3rdparty/am/google/sparse_hash_map>

#include <boost/test/unit_test.hpp>

#include <map>

namespace
{

template<typename MatrixType>
void checkMatrix()
{
    typedef typename MatrixType::key_type KeyType;
    typedef typename MatrixType::row_type RowType;
    typedef typename RowType::mapped_type ValueType;
    typedef boost::shared_ptr<RowType> SharedRowType;

    const int elemSize = MatrixType::ELEM_SIZE;
    const int rowSize = MatrixType::ROW_SIZE;
    BOOST_TEST_MESSAGE("elem size: " << elemSize);
    BOOST_TEST_MESSAGE("row size: " << rowSize);

    MatrixType matrix;
    const ValueType MATRIX_VALUE = 123.456;

    {
        BOOST_TEST_MESSAGE("check empty...");
        BOOST_CHECK_EQUAL(matrix.size(), 0U);
        BOOST_CHECK(matrix.empty());
        BOOST_CHECK_EQUAL(matrix.elem_count(), 0U);
        BOOST_CHECK(matrix.get(1) == false);
    }

    {
        BOOST_TEST_MESSAGE("check insert...");
        SharedRowType row1(new RowType);
        (*row1)[1] = MATRIX_VALUE;
        (*row1)[2] = MATRIX_VALUE;
        BOOST_CHECK(matrix.insert(1, row1));
        BOOST_CHECK(matrix.insert(1, row1) == false);

        SharedRowType row2(new RowType);
        (*row2)[10] = MATRIX_VALUE;
        (*row2)[20] = MATRIX_VALUE;
        BOOST_CHECK(matrix.insert(2, row2));
        BOOST_CHECK(matrix.insert(2, row2) == false);

        // matrix = [1][2], [2][2]
        BOOST_CHECK_EQUAL(matrix.size(), 2U);
        BOOST_CHECK(matrix.empty() == false);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 4U);
        BOOST_CHECK(matrix.get(1) == row1);
        BOOST_CHECK(matrix.get(2) == row2);
    }

    {
        BOOST_TEST_MESSAGE("check update...");
        SharedRowType row1(new RowType);
        (*row1)[1] = MATRIX_VALUE;
        (*row1)[3] = MATRIX_VALUE;
        (*row1)[5] = MATRIX_VALUE;
        matrix.update(1, row1);

        SharedRowType row3(new RowType);
        (*row3)[30] = MATRIX_VALUE;
        matrix.update(3, row3);

        // matrix = [1][3], [2][2], [3][1]
        BOOST_CHECK_EQUAL(matrix.size(), 3U);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 6U);
        BOOST_CHECK(matrix.get(1) == row1);
        BOOST_CHECK(matrix.get(3) == row3);
    }

    {
        BOOST_TEST_MESSAGE("check erase...");
        SharedRowType row1 = matrix.erase(1);
        BOOST_CHECK_EQUAL((*row1)[1], MATRIX_VALUE);
        BOOST_CHECK_EQUAL((*row1)[3], MATRIX_VALUE);
        BOOST_CHECK_EQUAL((*row1)[5], MATRIX_VALUE);
        BOOST_CHECK(matrix.get(1) == false);

        SharedRowType row4 = matrix.erase(4);
        BOOST_CHECK(row4 == false);

        // matrix = [2][2], [3][1]
        BOOST_CHECK_EQUAL(matrix.size(), 2U);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 3U);
    }

    {
        BOOST_TEST_MESSAGE("check incre_elem_count...");
        SharedRowType row2 = matrix.get(2);
        (*row2)[30] = MATRIX_VALUE;
        matrix.incre_elem_count();

        // matrix = [2][3], [3][1]
        BOOST_CHECK_EQUAL(matrix.size(), 2U);
        BOOST_CHECK(matrix.empty() == false);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 4U);
    }

    {
        BOOST_TEST_MESSAGE("check clear...");
        matrix.clear();

        BOOST_CHECK_EQUAL(matrix.size(), 0U);
        BOOST_CHECK(matrix.empty());
        BOOST_CHECK_EQUAL(matrix.elem_count(), 0U);
        BOOST_CHECK(matrix.get(1) == false);
    }
}

}

BOOST_AUTO_TEST_SUITE(ConcurrentMatrixTest)

BOOST_AUTO_TEST_CASE(testMatrixFunc)
{
    typedef int KeyType;
    typedef ::google::sparse_hash_map<int, float> RowType;

    BOOST_TEST_MESSAGE("=> check default container type...");
    typedef izenelib::am::ConcurrentMatrix<KeyType, RowType> DefaultMatrix;
    checkMatrix<DefaultMatrix>();

    BOOST_TEST_MESSAGE("=> check std::map as container type...");
    typedef boost::shared_ptr<RowType> SharedRowType;
    typedef std::map<KeyType, SharedRowType> StdContainer;
    typedef izenelib::am::ConcurrentMatrix<KeyType, RowType, StdContainer> StdMatrix;
    checkMatrix<StdMatrix>();
}

BOOST_AUTO_TEST_SUITE_END() 
