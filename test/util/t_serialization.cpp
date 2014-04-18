#include <boost/test/unit_test.hpp>

#include <util/izene_serialization.h>
#include <am/concept/DataType.h>
#include <util/ProcMemInfo.h>
#include <util/hashFunction.h>
#include <util/ustring/UString.h>

using namespace izenelib::util;

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

void test1() {
	string str("izenesoft");
	string str1;
	izene_serialization_boost<string> isb(str);
	char* ptr;
	size_t sz;
	isb.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_boost<string> idb(ptr, sz);
	idb.read_image(str1);
	cout<<"deserialization: "<<str1<<endl;
}

void test2() {
	string str("izenesoft");
	string str1;
	izene_serialization_memcpy<string> isb(str);
	char* ptr;
	size_t sz;
	isb.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_memcpy<string> idb(ptr, sz);
	idb.read_image(str1);
	cout<<"deserialization: "<<str1<<endl;
}

void test3() {
	string str("izenesoft");
	string str1;
	izene_serialization_febird<string> isb(str);
	char* ptr;
	size_t sz;
	isb.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_febird<string> idb(ptr, sz);
	idb.read_image(str1);
	cout<<"deserialization: "<<str1<<endl;
}

void test_padding()
{
    cout << "runing serialization padding test.=====" << endl;
    std::pair<char, uint64_t> t1;
    t1.first = 0x03;
    t1.second = 0x12345678;
    std::pair<char, uint64_t> t11;
    t11.first = 0x03;
    t11.second = 0x12345678;
    std::pair<uint64_t, char> t2;
    t2.first = 0x12345678;
    t2.second = 0x03;
    std::pair<uint64_t, char> t22;
    t22.first = 0x12345678;
    t22.second = 0x03;

    boost::tuple<char, uint64_t, int> t3;
    t3.get<0>() = 0x03;
    t3.get<1>() = 0x1234;
    t3.get<2>() = 0x12345678;

    boost::tuple<char, uint64_t, int> t33;
    t33.get<0>() = 0x03;
    t33.get<1>() = 0x1234;
    t33.get<2>() = 0x12345678;

    boost::tuple<uint64_t, char, int> t4;
    t4.get<0>() = 0x12345678;
    t4.get<1>() = 0x03;
    t4.get<2>() = 0x1234;

    boost::tuple<uint64_t, char, int> t44;
    t44.get<0>() = 0x12345678;
    t44.get<1>() = 0x03;
    t44.get<2>() = 0x1234;

    std::vector<std::pair<float, uint128_t> > t5;

    std::string str("568c15ffba1d46cdcc9c2cb56cbb3702");
    unsigned long long high = 0, low = 0;
    sscanf(str.c_str(), "%016llx%016llx", &high, &low);
    uint128_t data = (uint128_t) high << 64 | (uint128_t) low;

    t5.push_back(std::make_pair(0.01875, data));
    std::vector<std::pair<float, uint128_t> > t55;
    t55.push_back(std::make_pair(0.01875, data));

	char* ptr1;
	size_t sz1;
    izene_serialization_memcpy<std::pair<char, uint64_t> > ism1(t1);
    ism1.write_image(ptr1, sz1);
	char* ptr11;
	size_t sz11;
    izene_serialization_memcpy<std::pair<char, uint64_t> > ism11(t11);
    ism11.write_image(ptr11, sz11);

    BOOST_CHECK(memcmp(ptr1, ptr11, sz1) == 0);

    izene_deserialization_memcpy<std::pair<char, uint64_t> > idm11(ptr11, sz11);
    std::pair<char, uint64_t> out_t1;
    idm11.read_image(out_t1);
    BOOST_CHECK(out_t1.first == t1.first);
    BOOST_CHECK(out_t1.second == t1.second);

	izene_serialization_febird<std::pair<char, uint64_t> > isf1(t1);
    isf1.write_image(ptr1, sz1);
	izene_serialization_febird<std::pair<char, uint64_t> > isf11(t11);
    isf11.write_image(ptr11, sz11);

    BOOST_CHECK(memcmp(ptr1, ptr11, sz1) == 0);

    izene_serialization_boost<std::pair<char, uint64_t> > isb1(t1);
    isb1.write_image(ptr1, sz1);
    izene_serialization_boost<std::pair<char, uint64_t> > isb11(t11);
    isb11.write_image(ptr11, sz11);

    BOOST_CHECK(memcmp(ptr1, ptr11, sz1) == 0);

	char* ptr2;
	size_t sz2;
    izene_serialization_memcpy<std::pair<uint64_t, char> > ism2(t2);
    ism2.write_image(ptr2, sz2);
	char* ptr22;
	size_t sz22;
    izene_serialization_memcpy<std::pair<uint64_t, char> > ism22(t22);
    ism22.write_image(ptr22, sz22);

    BOOST_CHECK(memcmp(ptr2, ptr22, sz2) == 0);

    izene_deserialization_memcpy<std::pair<uint64_t, char> > idm22(ptr22, sz22);
    std::pair<uint64_t, char> out_t2;
    idm22.read_image(out_t2);
    BOOST_CHECK(out_t2.first == t2.first);
    BOOST_CHECK(out_t2.second == t2.second);


	izene_serialization_febird<std::pair<uint64_t, char> > isf2(t2);
    isf2.write_image(ptr2, sz2);
	izene_serialization_febird<std::pair<uint64_t, char> > isf22(t22);
    isf22.write_image(ptr22, sz22);

    BOOST_CHECK(memcmp(ptr2, ptr22, sz2) == 0);

    izene_serialization_boost<std::pair<uint64_t, char> > isb2(t2);
    isb2.write_image(ptr2, sz2);
    izene_serialization_boost<std::pair<uint64_t, char> > isb22(t22);
    isb22.write_image(ptr22, sz22);

    BOOST_CHECK(memcmp(ptr2, ptr22, sz2) == 0);

	char* ptr3;
	size_t sz3;
    izene_serialization_memcpy<boost::tuple<char, uint64_t, int> > ism3(t3);
    ism3.write_image(ptr3, sz3);
	char* ptr33;
	size_t sz33;
    izene_serialization_memcpy<boost::tuple<char, uint64_t, int> > ism33(t33);
    ism33.write_image(ptr33, sz33);

    BOOST_CHECK(memcmp(ptr3, ptr33, sz3) == 0);

    izene_deserialization_memcpy<boost::tuple<char, uint64_t, int> > idm33(ptr33, sz33);
    boost::tuple<char, uint64_t, int>  out_t3;
    idm33.read_image(out_t3);
    BOOST_CHECK(out_t3.get<0>() == t3.get<0>());
    BOOST_CHECK(out_t3.get<1>() == t3.get<1>());
    BOOST_CHECK(out_t3.get<2>() == t3.get<2>());

	izene_serialization_febird<boost::tuple<char, uint64_t, int> > isf3(t3);
    isf3.write_image(ptr3, sz3);
	izene_serialization_febird<boost::tuple<char, uint64_t, int> > isf33(t33);
    isf33.write_image(ptr33, sz33);

    BOOST_CHECK(memcmp(ptr3, ptr33, sz3) == 0);

    izene_serialization_boost<boost::tuple<char, uint64_t, int> > isb3(t3);
    isb3.write_image(ptr3, sz3);
    izene_serialization_boost<boost::tuple<char, uint64_t, int> > isb33(t33);
    isb33.write_image(ptr33, sz33);

    BOOST_CHECK(memcmp(ptr3, ptr33, sz3) == 0);

	char* ptr4;
	size_t sz4;
    izene_serialization_memcpy<boost::tuple<uint64_t, char, int> > ism4(t4);
    ism4.write_image(ptr4, sz4);
	char* ptr44;
	size_t sz44;
    izene_serialization_memcpy<boost::tuple<uint64_t, char, int> > ism44(t44);
    ism44.write_image(ptr44, sz44);

    BOOST_CHECK(memcmp(ptr4, ptr44, sz4) == 0);

	izene_serialization_febird<boost::tuple<uint64_t, char, int> > isf4(t4);
    isf4.write_image(ptr4, sz4);
	izene_serialization_febird<boost::tuple<uint64_t, char, int> > isf44(t44);
    isf44.write_image(ptr44, sz44);

    BOOST_CHECK(memcmp(ptr4, ptr44, sz4) == 0);

    izene_serialization_boost<boost::tuple<uint64_t, char, int> > isb4(t4);
    isb4.write_image(ptr4, sz4);
    izene_serialization_boost<boost::tuple<uint64_t, char, int> > isb44(t44);
    isb44.write_image(ptr44, sz44);

    BOOST_CHECK(memcmp(ptr4, ptr44, sz4) == 0);

	char* ptr5;
	size_t sz5;
    izene_serialization<std::vector<std::pair<float, uint128_t> > > ism5(t5);
    ism5.write_image(ptr5, sz5);
	char* ptr55;
	size_t sz55;
    izene_serialization<std::vector<std::pair<float, uint128_t> > > ism55(t55);
    ism55.write_image(ptr55, sz55);

    BOOST_CHECK(memcmp(ptr5, ptr55, sz5) == 0);

    izene_deserialization<std::vector<std::pair<float, uint128_t> > > idm55(ptr55, sz55);
    std::vector<std::pair<float, uint128_t> >  out_t5;
    idm55.read_image(out_t5);
    BOOST_CHECK(out_t5.back().first == t5.back().first);
    BOOST_CHECK(out_t5.back().second == t5.back().second);

	izene_serialization_febird<std::vector<std::pair<float, uint128_t> > > isf5(t5);
    isf5.write_image(ptr5, sz5);
	izene_serialization_febird<std::vector<std::pair<float, uint128_t> > > isf55(t55);
    isf55.write_image(ptr55, sz55);

    BOOST_CHECK(memcmp(ptr5, ptr55, sz5) == 0);
}

void test_performance() {
	clock_t t1;
	{
		char* ptr;
		size_t sz;
		t1 = clock();
		for (int i=0; i<1000000; i++) {
			string str("izenesoft");
			string str1;
			izene_serialization_boost<string> isb(str);

			isb.write_image(ptr, sz);

			izene_deserialization_boost<string> idb(ptr, sz);
			idb.read_image(str1);
			//cout<<"deserialization: "<<str1<<endl;
			 BOOST_CHECK(str == str1);
		}
		cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;
		printf("izene_serialization_boost elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
		displayMemInfo();
	}

	/*{
	 t1 = clock();
	 char* ptr;
	 size_t sz;
	 for (int i=0; i<1000000; i++) {
	 string str("izenesoft");
	 string str1;
	 izene_serialization_boost1<string> isb(str);
	 
	 isb.write_image(ptr, sz);
	 //cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	 izene_deserialization_boost1<string> idb(ptr, sz);
	 idb.read_image(str1);
	 //cout<<"deserialization: "<<str1<<endl;
	 BOOST_CHECK(str == str1);
	 }
	 cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;
	 printf("izene_serialization_boost1 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	 displayMemInfo();
	 }*/

	{
		t1 = clock();
		char* ptr;
		size_t sz;
		for (int i=0; i<1000000; i++) {
			string str("izenesoft");
			string str1;
			izene_serialization_febird<string> isb(str);

			isb.write_image(ptr, sz);

			//cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

			izene_deserialization_febird<string> idb(ptr, sz);
			idb.read_image(str1);
			//cout<<"deserialization: "<<str1<<endl;
			 BOOST_CHECK(str == str1);
		}
		cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

		printf("izene_serialization_febird elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
		displayMemInfo();
	}
	{
		t1 = clock();
		char* ptr;
		size_t sz;
		for (int i=0; i<1000000; i++) {
			string str("izenesoft");
			string str1;
			izene_serialization_memcpy<string> isb(str);

			isb.write_image(ptr, sz);

			//cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

			izene_deserialization_memcpy<string> idb(ptr, sz);
			idb.read_image(str1);
			//cout<<"deserialization: "<<str1<<endl;
			BOOST_CHECK(str == str1);
		}
		cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

		printf("izene_serialization_memcpy elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
		displayMemInfo();
	}

}

template<typename T> void test_serialization(T &dat) {

	T dat1;
	char* ptr;
	size_t sz = 0;

	izene_serialization<T> iss(dat);
	iss.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization<T> idb(ptr, sz);
	idb.read_image(dat1);

	 BOOST_CHECK(dat1 == dat);
	 BOOST_CHECK(izene_hashing(dat1) == izene_hashing(dat) );
	//cout<<"deserialization: "<<dat1<<endl;
}

template<typename T> void test_serialization_febird(T &dat) {

	T dat1;
	char* ptr;
	size_t sz;

	izene_serialization_febird<T> isf(dat);
	isf.write_image(ptr, sz);

	cout<<"serialization febird: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_febird<T> idb(ptr, sz);
	idb.read_image(dat1);

	 BOOST_CHECK(dat1 == dat);
	//cout<<"deserialization: "<<dat1<<endl;
}

template<typename T> void test_serialization_boost(T &dat) {

	T dat1;
	char* ptr;
	size_t sz;

	izene_serialization_boost<T> isf(dat);
	isf.write_image(ptr, sz);

	cout<<"serialization boost: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_boost<T> idb(ptr, sz);
	idb.read_image(dat1);

	 BOOST_CHECK (dat1 == dat);
	 BOOST_CHECK (izene_hashing(dat1) == izene_hashing(dat) );
	//cout<<"deserialization: "<<dat1<<endl;
}

class testobj {
public:
	int a;
public:
	testobj() {
		a = 0;
	}
	testobj(int b) :
		a(b) {

	}
	int compare(const testobj& other) const {
		return 1;
	}
	friend ostream& operator <<(ostream& os, const testobj& dat) {
		os<<dat.a;
		return os;
	}
	bool operator ==(const testobj& other) {
		return a == other.a;
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & a;
	}
	DATA_IO_LOAD_SAVE(testobj, &a)
};

MAKE_FEBIRD_SERIALIZATION(testobj)
//MAKE_MEMCPY_SERIALIZATION(testobj)

//MAKE_MEMCPY_TYPE(testobj)

namespace boost {
	namespace serialization {
		template<typename Archive> void serialize(Archive & ar, testobj & t,
				const unsigned int) {
			ar & t.a;
		}
	}
}

namespace febird {
//#define typeid(testobj).name()##Febird 
template<class DataIO> void DataIO_saveObject(DataIO& dio, const testobj& x) {
	dio & x.a;
}
template<class DataIO> void DataIO_loadObject(DataIO& dio, const testobj& x) {
	dio & x.a;
}

}
struct SOBJ {
	int a;
	int b;
	int c;
	bool operator ==(const SOBJ& other) const {
		return a == other.a && b == other.b && c == other.c;
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & a;
		ar & b;
		ar & c;
	}

};

MAKE_MEMCPY_TYPE(SOBJ);

//MAKE_MEMCPY_SERIALIZATION(vector<SOBJ> );
//MAKE_MEMCPY_SERIALIZATION(SOBJ)

//MAKE_FEBIRD_SERIALIZATION( vector<string> )

BOOST_AUTO_TEST_CASE(izene_serialization_test)

{
	//test_performance();

	testobj dat1(100);
	test_serialization(dat1);

	int a = 9999996;
	float b= 333.11;
	double c = 1.003;
	string str = "aaa";

	vector<int> vint;
	vint.push_back(1);
	vint.push_back(33);

	vector<string> vstr;
	vstr.push_back("aa");
	vstr.push_back("abc");
	vstr.push_back("55");
	vstr.push_back("551111");
	vstr.push_back("wps");
	
	SOBJ so;
	so.a = 33;
	so.b = 45;
	so.c = 444;

	test_serialization(a);
	test_serialization(b);
	test_serialization(c);
	test_serialization_febird(a);
	test_serialization_febird(b);
	test_serialization_febird(c);
	test_serialization_boost(str);
	test_serialization_boost(vint);
	test_serialization_boost(vstr);
	test_serialization(so);

	test_serialization(vstr);
	test_serialization_boost(vstr);
	test_serialization_febird(vstr);

	vector<int> va;
	test_serialization(va);
	vector<SOBJ> vsobj;
	vsobj.push_back(so);
	vsobj.push_back(so);
	test_serialization(vsobj);
	string shit("[NONE]SENSITIVE");
	test_serialization(shit);

	{

		vector< vector<int> > vvint;
		vector<int> a;
		a.push_back(1);
		a.push_back(2);
		vector<int> b;
		a.push_back(3);
		vvint.push_back(a);
		vvint.push_back(b);

		test_serialization(vvint);
		test_serialization_boost(vvint);
		test_serialization_febird(vvint);
	}

	izenelib::util::UString ustr("10K_E", izenelib::util::UString::CP949);

	test_serialization_boost(ustr);
	cout<<"!!!"<<endl;
	test_serialization(ustr);

	izenelib::util::UString ustr2("10K_E", izenelib::util::UString::UTF_8);
	test_serialization_boost(ustr2);
	cout<<"!!!"<<endl;
	test_serialization(ustr2);

    test_padding();

	/*	test1();
	 test2();
	 test3();*/
}

