#include <am/bitmap/Ewah.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>
#include <iostream>
#include <cstdio>

using namespace std;
using namespace izenelib::am;
namespace bfs = boost::filesystem;

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
    myarray1.add(zero);
    ba1.add(zero);
    myarray1.add(zero);
    ba1.add(zero);
    myarray1.add(zero);
    ba1.add(zero);
    myarray1.add(specialval);
    ba1.add(specialval);
    myarray1.add(specialval);
    ba1.add(specialval);
    myarray1.add(notzero);
    ba1.add(notzero);
    myarray1.add(zero);
    ba1.add(zero);
    EWAHBoolArray<uword> myarray2;
    BoolArray<uword> ba2;
    myarray2.add(notzero);
    ba2.add(notzero);
    myarray2.add(zero);
    ba2.add(zero);
    myarray2.add(notzero);
    ba2.add(notzero);
    myarray2.add(specialval);
    ba2.add(specialval);
    myarray2.add(specialval);
    ba2.add(specialval);
    myarray2.add(notzero);
    ba2.add(notzero);
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
    BoolArray<uword> aggregate1a;
    caggregate1.toBoolArray(aggregate1a);
    if (aggregate1a != aggregate1) {
        cout << "aggregate 1 failed" << endl;
        isOk = false;
    }
    BoolArray<uword> aggregate2a;
    caggregate2.toBoolArray(aggregate2a);
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
    myarray.add(zero);
    ba.setWord(0, zero);
    myarray.add(zero);
    ba.setWord(1, zero);
    myarray.add(zero);
    ba.setWord(2, zero);
    uword specialval = 1UL + (1UL << 4)+(static_cast<uword>(1) << (sizeof(uword)*8-1));
    myarray.add(specialval);
    ba.setWord(3, specialval);
    myarray.add(notzero);
    ba.setWord(4, notzero);
    myarray.add(notzero);
    ba.setWord(5, notzero);
    myarray.add(notzero);
    ba.setWord(6, notzero);
    myarray.add(notzero);
    ba.setWord(7, notzero);
    myarray.add(specialval);
    ba.setWord(8, specialval);
    myarray.add(zero);
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
        uword valref = ba.getWord(k++);
        if (val != valref) {
            cout<<"the two arrays differ from uncompressed array at "<<k<<" "<< val<< " "<< val2<<" " <<valref << endl;
            isOk = false;
        }
        if (val != val2) {
            cout<<"the two arrays differ at "<<k<<" "<< val<< " "<< val2<<" " <<valref << endl;
            isOk = false;
        }
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
        myarray1.add(x1[k]);
        myarray2.add(x2[k]);
        xand[k] = x1[k] & x2[k];
        xxor[k] = x1[k] | x2[k];
    }
    EWAHBoolArray<uword> myand;
    EWAHBoolArray<uword> mysparseand;
    EWAHBoolArray<uword> myor;
    myarray1.rawlogicaland(myarray2,myand);
    myarray1.sparselogicaland(myarray2,mysparseand);
    myarray1.rawlogicalor(myarray2,myor);
    EWAHBoolArrayIterator<uword> i = myand.uncompress();
    EWAHBoolArrayIterator<uword> ii = mysparseand.uncompress();
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
        if(!ii.hasNext()) {
            cout<<"type 2 error"<<endl;
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
        if(ii.next()!= xand[k]){
            cout<<"type 5 error"<<endl;
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


BOOST_AUTO_TEST_SUITE(bitmap_ewah_test)


BOOST_AUTO_TEST_CASE(runningLength)
{
    BOOST_CHECK(testRunningLengthWord<uint16_t > ());
    BOOST_CHECK(testRunningLengthWord<uint32_t > ());
    BOOST_CHECK(testRunningLengthWord<uint64_t > ());
}

BOOST_AUTO_TEST_CASE(ewahBoolArray)
{
    BOOST_CHECK(testEWAHBoolArray<uint16_t > ());
    BOOST_CHECK(testEWAHBoolArray<uint32_t > ());
    BOOST_CHECK(testEWAHBoolArray<uint64_t > ());
}

BOOST_AUTO_TEST_CASE(ewahBoolArrayLogical)
{
    BOOST_CHECK(testEWAHBoolArrayLogical<uint16_t > ());
    BOOST_CHECK(testEWAHBoolArrayLogical<uint32_t > ());
    BOOST_CHECK(testEWAHBoolArrayLogical<uint64_t > ());
}

BOOST_AUTO_TEST_CASE(ewahBoolArrayAppend)
{
    BOOST_CHECK(testEWAHBoolArrayAppend<uint16_t > ());
    BOOST_CHECK(testEWAHBoolArrayAppend<uint32_t > ());
    BOOST_CHECK(testEWAHBoolArrayAppend<uint64_t > ());
}

BOOST_AUTO_TEST_CASE(ewahBoolArrayBench)
{
    EWAHBoolArray<uint32_t> myarray;
    size_t max = 10000000;
    size_t i = 1;
    size_t maxstep = 10;
    size_t count = 0;
    for(; i < max; )
    {
        i += (1+rand() % maxstep);
        myarray.set(i);
        ++count;
    }
    /*
    EWAHBoolArrayBitIterator<uint32_t> iter = myarray.bit_iterator();


    i = 1;
    while(iter.next())
    {
        BOOST_CHECK((i++) ==iter.getCurr());
    }
    BOOST_CHECK(i = max);
    */	
    std::cout<<"count "<<count<<" sizeInBytes "<<myarray.sizeInBytes()<<std::endl;
}

BOOST_AUTO_TEST_CASE(ewahBitIterator)
{
    EWAHBoolArray<uint32_t> myarray1;
    BoolArray<uint32_t> boolarray1;
    size_t max = 2000; //10000000;
    size_t maxstep = 100;
    size_t i = 1;

    for(; i < max; )
    {
        myarray1.set(i);
        i += (1 + rand() % maxstep);
    }

    myarray1.toBoolArray(boolarray1);
    EWAHBoolArrayBitIterator<uint32_t> iter = myarray1.bit_iterator();
    while(iter.next())
    {
        BOOST_CHECK( boolarray1.get(iter.getCurr()) == true);
    }
}

BOOST_AUTO_TEST_SUITE_END()

