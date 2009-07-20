#include <util/izene_serialization.h>
#include <am/concept/DataType.h>
#include <util/ProcMemInfo.h>
#include <util/hashFunction.h>
#include <wiselib/ustring/UString.h>

using namespace izenelib::util;

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
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
			assert(str == str1);
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
			assert(str == str1);
		}
		cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;
		printf("izene_serialization_boost1 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
		displayMemInfo();
	}*/
	
	{
		t1 = clock();
		for (int i=0; i<1000000; i++) {
			string str("izenesoft");
			string str1;
			izene_serialization_febird<string> isb(str);
			char* ptr;
			size_t sz;
			isb.write_image(ptr, sz);

			//cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

			izene_deserialization_febird<string> idb(ptr, sz);
			idb.read_image(str1);
			//cout<<"deserialization: "<<str1<<endl;
			assert(str == str1);
		}
		printf("izene_serialization_febird elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
		displayMemInfo();
	}
	{
		t1 = clock();
		for (int i=0; i<1000000; i++) {
			string str("izenesoft");
			string str1;
			izene_serialization_memcpy<string> isb(str);
			char* ptr;
			size_t sz;
			isb.write_image(ptr, sz);

			//cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

			izene_deserialization_memcpy<string> idb(ptr, sz);
			idb.read_image(str1);
			//cout<<"deserialization: "<<str1<<endl;
			assert(str == str1);
		}
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

	assert(dat1 == dat);
	assert(izene_hashing(dat1) == izene_hashing(dat) );
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

	assert(dat1 == dat);
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

	assert(dat1 == dat);
	assert(izene_hashing(dat1) == izene_hashing(dat) );
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

MAKE_FEBIRD_SERIALIZATION( vector<string> )

int main() {
	test_performance();
	
	
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

	wiselib::UString ustr("10K_E", wiselib::UString::CP949);

	test_serialization_boost(ustr);
	cout<<"!!!"<<endl;
	test_serialization(ustr);
	
	
	/*	test1();
	 test2();
	 test3();*/
}

