#include <util/izene_serialization.h>
#include <am/concept/DataType.h>

using namespace izenelib::util;

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
	bool operator ==(const SOBJ& other) {
		return a == other.a && b == other.b && c == other.c;
	}
};

MAKE_MEMCPY_TYPE(SOBJ);

//MAKE_MEMCPY_SERIALIZATION(SOBJ)

MAKE_FEBIRD_SERIALIZATION( vector<string> )

int main() {
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

	/*	test1();
	 test2();
	 test3();*/
}

