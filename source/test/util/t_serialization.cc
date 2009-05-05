#include <util/izene_serialization.h>
#include <am/concept/DataType.h>

using namespace izenelib::util;

void test1() {
	string str("izenesoft");
	string str1;
	izene_serialization_boost<string> isb(str);
	void* ptr;
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
	void* ptr;
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
	void* ptr;
	size_t sz;
	isb.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization_febird<string> idb(ptr, sz);
	idb.read_image(str1);
	cout<<"deserialization: "<<str1<<endl;
}

template<typename T> void test_serialization(T &dat) {

	T dat1;
	void* ptr;
	size_t sz;

	izene_serialization<T> iss(dat);
	iss.write_image(ptr, sz);

	cout<<"serialization: "<<(char*)ptr<<" | "<<sz<<endl;

	izene_deserialization<T> isd(ptr, sz);
	isd.read_image(dat1);

	cout<<"deserialization: "<<dat1<<endl;
}

class testobj {
public:
	int a;
	void print() {
	}
	int compare(const testobj& other) const {
		return 1;
	}
	friend ostream& operator <<(ostream& os, const testobj& dat) {
		os<<dat.a;
		return os;
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & a;
	}
	
	//DATA_IO_LOAD_SAVE(testobj, &a)
};

MAKE_FEBIRD_SERIALIZATION(testobj)
//MAKE_MEMCPY_SERIALIZATION(testobj)

//#define typeid(testobj).name()##Febird 
template<class DataIO> void DataIO_saveObject(DataIO& dio, const testobj& x) {

	dio & x.a;
}
template<class DataIO> void DataIO_loadObject(DataIO& dio, const testobj& x) {
	dio & x.a;
}

int main() {

#ifdef typeid(testobj).name()##Febird
	cout<<"cool11"<<endl;
#endif

	testobj dat1;
	dat1.a = 100;
	test_serialization(dat1);

/*	test1();
	test2();
	test3();*/
}

