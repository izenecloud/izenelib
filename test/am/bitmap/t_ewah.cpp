#include <am/bitmap/ewah.h>
#include <ir/index_manager/utility/Bitset.h>
#include <util/ClockTimer.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace izenelib::am;
using namespace izenelib::ir::indexmanager;
namespace bfs = boost::filesystem;

static uint32_t *int_data;
static uint32_t * copy_data;
void init_data(int data_size, int& copy_length)
{
    std::cout<<"generating data... "<<std::endl;
    int MAXID = 20000;//50000000;

    int_data = new unsigned[data_size];
    srand( (unsigned)time(NULL) );

    for (int i = 0; i < data_size; ++i) {
        int_data[i] = rand() % MAXID;
    }
    std::sort(int_data, int_data+ data_size);
    uint32_t* it = std::unique(int_data, int_data+ data_size);

    copy_length = it - int_data;
    cout<<"real_size = "<<copy_length<<endl;
    copy_data = new unsigned[copy_length];
    std::memcpy(copy_data, int_data, copy_length * sizeof(uint32_t));

    std::cout<<"done!\n";
}

const char* BITMAP_TEST_DIR_STR = "bitmap";

template<class uword>
bool testRunningLengthWord() {
    cout << "[testing RunningLengthWord]" << endl;
    bool isOk(true);
    uword somenumber(0xABCD);
    RunningLengthWord<uword> rlw(somenumber);
    rlw.setRunningBit(true);
    if (rlw.getRunningBit() != true) {
        cout << "failed to set the running bit " << sizeof (uword) << endl;
        isOk = false;
    }
    for (uword myrl = 0; myrl <= RunningLengthWord<uword>::largestrunninglengthcount; myrl+=RunningLengthWord<uword>::largestrunninglengthcount/10) {
        rlw.setRunningLength(myrl);
        if (rlw.getRunningBit() != true) {
            cout << "failed to set the running bit (2) " << sizeof (uword) << endl;
            isOk = false;
        }
        if (rlw.getRunningLength() != myrl) {
            cout << "failed to set the running length " << sizeof (uword) << endl;
            isOk = false;
        }
    }
    rlw.setRunningLength(12);
    for (uword mylw = 0; mylw <= RunningLengthWord<uword>::largestliteralcount; mylw+=RunningLengthWord<uword>::largestliteralcount/10) {
        rlw.setNumberOfLiteralWords(mylw);
        if (rlw.getRunningBit() != true) {
            cout << "failed to set the running bit (3) " << sizeof (uword) << endl;
            isOk = false;
        }
        if (rlw.getRunningLength() != 12) {
            cout << "failed to set the running length (2) " << sizeof (uword) << endl;
            isOk = false;
        }
        if (rlw.getNumberOfLiteralWords() != mylw) {
            cout << "failed to set the LiteralWords " <<mylw << " "<<sizeof (uword)<<" "<<rlw.getNumberOfLiteralWords() << endl;
            isOk = false;
        }
    }
    rlw.setNumberOfLiteralWords(43);
    rlw.setRunningBit(false);
    if (rlw.getRunningBit() != false) {
        cout << "failed to set the running bit (4) " << sizeof (uword) << endl;
        isOk = false;
    }
    if (rlw.getRunningLength() != 12) {
        cout << "failed to set the running length (3) " << sizeof (uword) << endl;
        isOk = false;
    }
    if (rlw.getNumberOfLiteralWords() != 43) {
        cout << "failed to set the LiteralWords (2) " << sizeof (uword) << endl;
        isOk = false;
    }
    return isOk;
}


template <class uword>
bool testEWAHBoolArrayAppend() {
    cout << "[testing EWAHBoolArrayAppend]" << endl;
    bool isOk(true);
    uword zero = 0;
    uword specialval = 1UL + (1UL << 4)+(static_cast<uword>(1) << (sizeof(uword)*8-1));
    uword notzero = ~zero;
    EWAHBoolArray<uword> myarray1;
    BoolArray<uword> ba1;
    myarray1.addWord(zero);
    ba1.addWord(zero);
    myarray1.addWord(zero);
    ba1.addWord(zero);
    myarray1.addWord(zero);
    ba1.addWord(zero);
    myarray1.addWord(specialval);
    ba1.addWord(specialval);
    myarray1.addWord(specialval);
    ba1.addWord(specialval);
    myarray1.addWord(notzero);
    ba1.addWord(notzero);
    myarray1.addWord(zero);
    ba1.addWord(zero);
    EWAHBoolArray<uword> myarray2;
    BoolArray<uword> ba2;
    myarray2.addWord(notzero);
    ba2.addWord(notzero);
    myarray2.addWord(zero);
    ba2.addWord(zero);
    myarray2.addWord(notzero);
    ba2.addWord(notzero);
    myarray2.addWord(specialval);
    ba2.addWord(specialval);
    myarray2.addWord(specialval);
    ba2.addWord(specialval);
    myarray2.addWord(notzero);
    ba2.addWord(notzero);
    BoolArray<uword> aggregate1(ba1);
    BoolArray<uword> aggregate2(ba2);
    aggregate1.append(ba2);
    aggregate2.append(ba1);
    EWAHBoolArray<uword> caggregate1;
    caggregate1.append(myarray1);
    EWAHBoolArray<uword> caggregate2;
    caggregate2.append(myarray2);
    caggregate1.append(myarray2);
    caggregate2.append(myarray1);
    BoolArray<uword> aggregate1a = caggregate1.toBoolArray();
    if (aggregate1a != aggregate1) {
        cout << "aggregate 1 failed" << endl;
        isOk = false;
    }
    BoolArray<uword> aggregate2a = caggregate2.toBoolArray();
    if (aggregate2a != aggregate2) {
        cout << "aggregate 2 failed" << endl;
        isOk = false;
    }
    return isOk;
}

template<class uword>
bool testEWAHBoolArray() {
    cout << "[testing EWAHBoolArray]" << endl;
    bool isOk(true);
    EWAHBoolArray<uword> myarray;
    BoolArray<uword> ba(10 * sizeof (uword) * 8);
    uword zero = 0;
    uword notzero = ~zero;
    myarray.addWord(zero);
    ba.setWord(0, zero);
    myarray.addWord(zero);
    ba.setWord(1, zero);
    myarray.addWord(zero);
    ba.setWord(2, zero);
    uword specialval = 1UL + (1UL << 4)+(static_cast<uword>(1) << (sizeof(uword)*8-1));
    myarray.addWord(specialval);
    ba.setWord(3, specialval);
    myarray.addWord(notzero);
    ba.setWord(4, notzero);
    myarray.addWord(notzero);
    ba.setWord(5, notzero);
    myarray.addWord(notzero);
    ba.setWord(6, notzero);
    myarray.addWord(notzero);
    ba.setWord(7, notzero);
    myarray.addWord(specialval);
    ba.setWord(8, specialval);
    myarray.addWord(zero);
    ba.setWord(9, zero);
    if (myarray.sizeInBits() != 10 * sizeof (uword) * 8) {
        cout << "expected " << 10 * sizeof (uword) * 8 << " bits but found " << myarray.sizeInBits() << endl;
        isOk = false;
    }

    bfs::path bitmapDir(BITMAP_TEST_DIR_STR);
    boost::filesystem::remove_all(bitmapDir);
    bfs::create_directories(bitmapDir);
    std::string bitmap_dir_str = bitmapDir.string();
    std::string indexfile(bitmap_dir_str+"/testingewahboolarray.bin");

    ofstream out(indexfile.c_str(), ios::out | ios::binary);
    myarray.write(out);
    out.close();
    EWAHBoolArray<uword> lmyarray;
    ifstream in(indexfile.c_str());
    lmyarray.read(in);
    in.close();
    BOOST_CHECK(myarray == lmyarray);

    {
    izene_serialization_febird<EWAHBoolArray<uword> > isb(myarray);
    char* ptr;
    size_t sz;
    isb.write_image(ptr, sz);

    EWAHBoolArray<uword> newarray;
    izene_deserialization_febird<EWAHBoolArray<uword> > idb(ptr, sz);
    idb.read_image(newarray);
    BOOST_CHECK(myarray == newarray);
    }

    EWAHBoolArrayIterator<uword> i = myarray.uncompress();
    EWAHBoolArrayIterator<uword> j = lmyarray.uncompress();
    uint k = 0;
    while (i.hasNext()) {
        if (!j.hasNext()) {
            cout<<"the two arrays don't have the same size?"<<endl;
            isOk = false;
            break;
        }
        uword val = i.next();
        uword val2 = j.next();
        uword valref = ba.getWord(k);
        if (val != valref) {
            cout<<"the two arrays differ from uncompressed array at "<<k<<" "<< val<< " "<< val2<<" " <<valref << endl;
            isOk = false;
        }
        if (val != val2) {
            cout<<"the two arrays differ at "<<k<<" "<< val<< " "<< val2<<" " <<valref << endl;
            isOk = false;
        }
        ++k;
    }
    return isOk;
}



template<class uword>
bool testEWAHBoolArrayLogical() {
    cout << "[testing EWAHBoolArrayLogical]" << endl;
    bool isOk(true);
    EWAHBoolArray<uword> myarray1;
    EWAHBoolArray<uword> myarray2;
    const uint N=15;
    uword allones = static_cast<uword>(~0L);
    uword x1[N] ={1,54,24,145,0,0,0,allones,allones,allones,43,0,0,0,1};
    uword x2[N] ={allones,0,0,0,0,0,0,0,allones,allones,allones,0,4,0,0};
    uword xand[N];
    uword xxor[N];
    for(uint k = 0; k < N; ++k) {
        myarray1.addWord(x1[k]);
        myarray2.addWord(x2[k]);
        xand[k] = x1[k] & x2[k];
        xxor[k] = x1[k] | x2[k];
    }
    EWAHBoolArray<uword> myand;
    EWAHBoolArray<uword> myor;
    myarray1.logicaland(myarray2,myand);
    myarray1.logicalor(myarray2,myor);
    EWAHBoolArrayIterator<uword> i = myand.uncompress();
    EWAHBoolArrayIterator<uword> j = myor.uncompress();
    EWAHBoolArrayIterator<uword> it1 = myarray1.uncompress();
    EWAHBoolArrayIterator<uword> it2 = myarray2.uncompress();
    for(uint k = 0; k <N;++k) {
        const uword m1 = it1.next();
        const uword m2 = it2.next();
        if(!i.hasNext()) {
            cout<<"type 1 error"<<endl;
            isOk=false; break;
        }
        if(!j.hasNext()) {
            cout<<"type 3 error"<<endl;
            isOk=false; break;
        }
        if(i.next()!= xand[k]){
            cout<<"type 4 error"<<endl;
            isOk=false; break;
        }
        const uword jor = j.next();
        if(jor!= xxor[k]){
            cout<<m1<<" or "<< m2<<" = "<< xxor[k] << " but got "<<jor <<endl;
            cout<<"type 6 error OR"<<endl;
            isOk=false; break;
        }
    }
    return isOk;
}

template <typename word_t>
void checkCompressBitVector(const Bitset& bitVector)
{
    BOOST_TEST_MESSAGE("origin: " << bitVector);

    EWAHBoolArray<word_t> ewahBoolArray;
    bitVector.compress(ewahBoolArray);

    Bitset uncompress;
    uncompress.decompress(ewahBoolArray);
    BOOST_TEST_MESSAGE("uncomp: " << uncompress);

    const std::size_t bitNum = bitVector.size();
    //BOOST_CHECK_LE(uncompress.size(), bitNum);

    for (std::size_t i = 0; i < bitNum; ++i)
    {
        BOOST_CHECK_EQUAL(uncompress.test(i), bitVector.test(i));
    }
}

void setBitVector(Bitset& bitVector, std::size_t setBitNum)
{
    std::size_t bitNum = bitVector.size();
    for (std::size_t i = 0; i < setBitNum; ++i)
    {
        int k = std::rand() % bitNum;
        bitVector.set(k);
    }
}

template <typename word_t>
void testCompressBitVector(std::size_t bitNum)
{
    BOOST_TEST_MESSAGE("testCompressBitVector, sizeof(word_t): " << sizeof(word_t)
                       << ", bitNum: " << bitNum);

    Bitset bitVector(bitNum);

    bitVector.set();
    checkCompressBitVector<word_t>(bitVector);

    bitVector.reset();
    checkCompressBitVector<word_t>(bitVector);

    const std::size_t setBitNum = bitNum / 2;
    setBitVector(bitVector, setBitNum);
    checkCompressBitVector<word_t>(bitVector);

    bitVector.flip();
    checkCompressBitVector<word_t>(bitVector);
}

template <typename word_t>
void runCompressToEWAHBoolArray(const Bitset& bitVector, std::size_t runNum)
{
    izenelib::util::ClockTimer timer;

    for (std::size_t i = 0; i < runNum; ++i)
    {
        EWAHBoolArray<word_t> ewahBoolArray;
        bitVector.compress(ewahBoolArray);
    }

    double costTime = timer.elapsed() / runNum * 1000;
    BOOST_TEST_MESSAGE("it costs " << costTime << " ms in running "
                       << "Bitset::compress() on " << bitVector.size()
                       << " bits, sizeof(word_t): " << sizeof(word_t));
}

template <typename word_t>
void runIterateEWAHBoolArray(const Bitset& bitVector, std::size_t runNum)
{
    EWAHBoolArray<word_t> ewahBoolArray;
    bitVector.compress(ewahBoolArray);

    std::size_t setBitNum = 0;
    izenelib::util::ClockTimer timer;

    for (std::size_t i = 0; i < runNum; ++i)
    {
        setBitNum = 0;
        for (typename EWAHBoolArray<word_t>::const_iterator iter =
                 ewahBoolArray.begin(); iter != ewahBoolArray.end(); ++iter)
        {
            *iter;
            ++setBitNum;
        }
    }

    double costTime = timer.elapsed() / runNum * 1000;
    BOOST_TEST_MESSAGE("it costs " << costTime << " ms in running EWAHBoolArrayBitIterator::next() on "
                       << setBitNum << "/" << bitVector.size()
                       << " bits, sizeof(word_t): " << sizeof(word_t)
                       << ", EWAHBoolArray::sizeInBytes(): " << ewahBoolArray.sizeInBytes());
}

template <typename word_t>
void runUncompressToEWAHBoolArray(
    const Bitset& bitVector,
    std::size_t runNum)
{
    EWAHBoolArray<word_t> ewahBoolArray;
    bitVector.compress(ewahBoolArray);

    izenelib::util::ClockTimer timer;

    for (std::size_t i = 0; i < runNum; ++i)
    {
        Bitset uncompress;
        uncompress.decompress(ewahBoolArray);
    }

    double costTime = timer.elapsed() / runNum * 1000;
    BOOST_TEST_MESSAGE("it costs " << costTime << " ms in running "
                       << "Bitset::decompress() on " << bitVector.size()
                       << " bits, sizeof(word_t): " << sizeof(word_t)
                       << ", EWAHBoolArray::sizeInBytes(): " << ewahBoolArray.sizeInBytes());
}

BOOST_AUTO_TEST_SUITE(bitmap_ewah_test)


BOOST_AUTO_TEST_CASE(runningLength)
{
    BOOST_CHECK(testRunningLengthWord<uint16_t>());
    BOOST_CHECK(testRunningLengthWord<uint32_t>());
    BOOST_CHECK(testRunningLengthWord<uint64_t>());
}

BOOST_AUTO_TEST_CASE(ewahBoolArray)
{
    BOOST_CHECK(testEWAHBoolArray<uint16_t>());
    BOOST_CHECK(testEWAHBoolArray<uint32_t>());
    BOOST_CHECK(testEWAHBoolArray<uint64_t>());
}

BOOST_AUTO_TEST_CASE(ewahBoolArrayLogical)
{
    BOOST_CHECK(testEWAHBoolArrayLogical<uint16_t>());
    BOOST_CHECK(testEWAHBoolArrayLogical<uint32_t>());
    BOOST_CHECK(testEWAHBoolArrayLogical<uint64_t>());
}

BOOST_AUTO_TEST_CASE(ewahBoolArrayAppend)
{
    BOOST_CHECK(testEWAHBoolArrayAppend<uint16_t>());
    BOOST_CHECK(testEWAHBoolArrayAppend<uint32_t>());
    BOOST_CHECK(testEWAHBoolArrayAppend<uint64_t>());
}

BOOST_AUTO_TEST_CASE(ewahBitIterator)
{
    int data_size = 1000; //10000000
    int real_size = 0;
    init_data(data_size, real_size);

    EWAHBoolArray<uint32_t> myarray1;
    int i = 0;

    for(; i < real_size; ++i)
    {
        myarray1.set(copy_data[i]);
    }

    BoolArray<uint32_t> boolarray1 = myarray1.toBoolArray();
    for (EWAHBoolArray<uint32_t>::const_iterator iter = myarray1.begin();
         iter != myarray1.end(); ++iter)
    {
        BOOST_CHECK(boolarray1.get(*iter));
    }

    delete[] int_data;
    delete[] copy_data;
}

BOOST_AUTO_TEST_CASE(compressBitVector)
{
    const std::size_t maxBitNum = 300;
    for (std::size_t i = 0; i <= maxBitNum; ++i)
    {
        testCompressBitVector<uint32_t>(i);
        testCompressBitVector<uint64_t>(i);
    }
}

BOOST_AUTO_TEST_CASE(benchEWAHBoolArray)
{
    const std::size_t bitNum = 1e5;
    const std::size_t setBitNum = bitNum / 5;
    const std::size_t compressNum = 1e4;
    const std::size_t iterateNum = 1e2;

    Bitset bitVector(bitNum);
    setBitVector(bitVector, setBitNum);

    runCompressToEWAHBoolArray<uint32_t>(bitVector, compressNum);
    runIterateEWAHBoolArray<uint32_t>(bitVector, iterateNum);

    runCompressToEWAHBoolArray<uint64_t>(bitVector, compressNum);
    runIterateEWAHBoolArray<uint64_t>(bitVector, iterateNum);
}

BOOST_AUTO_TEST_CASE(benchUncompress)
{
    const std::size_t bitNum = 5e7;
    const std::size_t setBitNum = 5e5;
    const std::size_t uncompressNum = 1e2;

    Bitset bitVector(bitNum);
    setBitVector(bitVector, setBitNum);

    runUncompressToEWAHBoolArray<uint32_t>(bitVector, uncompressNum);
    runUncompressToEWAHBoolArray<uint64_t>(bitVector, uncompressNum);
}

BOOST_AUTO_TEST_SUITE_END()
