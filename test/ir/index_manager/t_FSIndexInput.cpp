#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include <ir/index_manager/store/FSIndexInput.h>

using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

// given a little endian value, return a high endian value.
int toHighEnd(int little)
{
    int high = 0;
    high |= (little & 0xFF) << 24;
    little >>= 8;
    high |= (little & 0xFF) << 16;
    little >>= 8;
    high |= (little & 0xFF) << 8;
    little >>= 8;
    high |= (little & 0xFF);

    return high;
}

BOOST_AUTO_TEST_SUITE(t_FSIndexInput)

BOOST_AUTO_TEST_CASE(read)
{
    const unsigned int size = 1024*10;
    int* buffer = new int[size];
    const unsigned int bufferSize = size*sizeof(int);
    for(unsigned int i=0; i<size; ++i)
        buffer[i] = i;

    const char* fileName = "fsindex.txt";
    ofstream ofs(fileName, ofstream::binary);
    ofs.write(reinterpret_cast<char*>(buffer), bufferSize);
    ofs.close();
    delete[] buffer;

    // call IndexInput::readIntBySmallEndian()
    FSIndexInput fsIndexInput(fileName);
    BOOST_CHECK_EQUAL(fsIndexInput.length(), bufferSize);
    int readIntCount = size / 3;
    for(int i=0; i<readIntCount; ++i)
        BOOST_CHECK_EQUAL(fsIndexInput.readIntBySmallEndian(), i);
    BOOST_CHECK_EQUAL(fsIndexInput.getFilePointer(), static_cast<int64_t>(readIntCount * sizeof(int)));

    // call IndexInput::read()
    const int newBufferSize = readIntCount * sizeof(int);
    int* newBuffer = new int[newBufferSize];
    fsIndexInput.read(reinterpret_cast<char*>(newBuffer), newBufferSize);
    for(int i=0; i<readIntCount; ++i)
        BOOST_CHECK_EQUAL(newBuffer[i], readIntCount + i);
    delete[] newBuffer;
    BOOST_CHECK_EQUAL(fsIndexInput.getFilePointer(), static_cast<int64_t>(2 * readIntCount * sizeof(int)));

    // call IndexInput::readInt()
    for(int i=0; i<static_cast<int>(size-2*readIntCount); ++i)
        BOOST_CHECK_EQUAL(fsIndexInput.readInt(), toHighEnd(2*readIntCount + i));
    BOOST_CHECK_EQUAL(fsIndexInput.getFilePointer(), static_cast<int64_t>(bufferSize));
}

BOOST_AUTO_TEST_SUITE_END()
