///
/// @file t_matrix_cache.cpp
/// @brief test functions in MatrixCache
/// @author Jun Jiang
/// @date Created 2011-11-24
///

#include <am/matrix/matrix_cache.h>
#include <3rdparty/am/google/sparse_hash_map>

#include <boost/test/unit_test.hpp>

#include <map>

namespace
{

template<typename RowType>
void checkRow(const RowType& row, const RowType& gold)
{
    BOOST_REQUIRE_EQUAL(row.size(), gold.size());

    typename RowType::const_iterator rowIt = row.begin();
    typename RowType::const_iterator goldIt = gold.begin();
    typename RowType::const_iterator rowEnd = row.end();
    typename RowType::const_iterator goldEnd = gold.end();

    for (; rowIt != rowEnd && goldIt != goldEnd; ++rowIt, ++goldIt)
    {
        BOOST_CHECK_EQUAL(rowIt->first, goldIt->first);
        BOOST_CHECK_EQUAL(rowIt->second, goldIt->second);
    }

    BOOST_CHECK(rowIt == rowEnd);
    BOOST_CHECK(goldIt == goldEnd);
}

template<typename RowType>
class UpdateRowFunc
{
public:
    void operator() (RowType& row)
    {
        for (typename RowType::iterator it = row.begin();
            it != row.end(); ++it)
        {
            it->second *= 3;
        }
    }
};

template<typename RowType>
class CheckRowFunc
{
public:
    CheckRowFunc(const RowType& gold)
        : gold_(gold)
    {}

    void operator() (const RowType& row)
    {
        checkRow(row, gold_);
    }

private:
    const RowType& gold_;
};

template<typename KeyType, typename RowType>
class CheckResetRowFunc
{
public:
    CheckResetRowFunc(KeyType goldKey, const RowType& goldRow)
        : goldKey_(goldKey), goldRow_(goldRow)
    {}

    void operator() (KeyType key, const RowType& row)
    {
        BOOST_CHECK_EQUAL(key, goldKey_);
        checkRow(row, goldRow_);
    }

private:
    const KeyType goldKey_;
    const RowType& goldRow_;
};

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
        BOOST_TEST_MESSAGE("check update_elem...");
        BOOST_CHECK(matrix.update_elem(1, 1, 2*MATRIX_VALUE));
        BOOST_CHECK(matrix.update_elem(1, 2, 2*MATRIX_VALUE));

        BOOST_CHECK(matrix.update_elem(4, 1, 2*MATRIX_VALUE) == false);
    }

    {
        BOOST_TEST_MESSAGE("check update_with_func...");
        UpdateRowFunc<RowType> updateFunc;
        BOOST_CHECK(matrix.update_with_func(1, updateFunc));
    }

    RowType goldRow1;
    goldRow1[1] = MATRIX_VALUE*2*3;
    goldRow1[2] = MATRIX_VALUE*2*3;
    goldRow1[3] = MATRIX_VALUE*3;
    goldRow1[5] = MATRIX_VALUE*3;

    {
        BOOST_TEST_MESSAGE("check read_with_func...");
        CheckRowFunc<RowType> readFunc(goldRow1);
        BOOST_CHECK(matrix.read_with_func(1, readFunc));
    }

    {
        BOOST_TEST_MESSAGE("check erase...");
        bool isDirty = false;
        SharedRowType row1 = matrix.erase(1, isDirty);
        BOOST_CHECK(isDirty); // as updated
        checkRow(*row1, goldRow1);
        BOOST_CHECK(matrix.get(1) == false);

        SharedRowType row4 = matrix.erase(4, isDirty);
        BOOST_CHECK(row4.get() == NULL);

        // matrix = [2][2], [3][1]
        BOOST_CHECK_EQUAL(matrix.size(), 2U);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 3U);
    }

    {
        BOOST_TEST_MESSAGE("check reset_dirty_flag...");
        RowType goldRow3;
        goldRow3[30] = MATRIX_VALUE;

        CheckResetRowFunc<KeyType, RowType> resetFunc(3, goldRow3);

        BOOST_CHECK(matrix.reset_dirty_flag(resetFunc)); // as updated
        BOOST_CHECK(matrix.reset_dirty_flag(resetFunc) == false); // as no dirty flag
    }

    BOOST_TEST_MESSAGE(matrix);

    {
        BOOST_TEST_MESSAGE("check clear...");
        matrix.clear();

        BOOST_CHECK_EQUAL(matrix.size(), 0U);
        BOOST_CHECK(matrix.empty());
        BOOST_CHECK_EQUAL(matrix.occupy_size(), 0U);
        BOOST_CHECK_EQUAL(matrix.elem_count(), 0U);
        BOOST_CHECK(matrix.get(1) == false);
    }
}

}

BOOST_AUTO_TEST_SUITE(MatrixCacheTest)

BOOST_AUTO_TEST_CASE(testMatrixFunc)
{
    typedef int KeyType;
    typedef ::google::sparse_hash_map<int, float> RowType;

    BOOST_TEST_MESSAGE("=> check default container type...");
    typedef izenelib::am::MatrixCache<KeyType, RowType> DefaultMatrix;
    checkMatrix<DefaultMatrix>();

    BOOST_TEST_MESSAGE("=> check std::map as container type...");
    typedef boost::shared_ptr<RowType> SharedRowType;
    typedef std::map<KeyType, SharedRowType> StdContainer;
    typedef izenelib::am::MatrixCache<KeyType, RowType, StdContainer> StdMatrix;
    checkMatrix<StdMatrix>();
}

BOOST_AUTO_TEST_SUITE_END() 
