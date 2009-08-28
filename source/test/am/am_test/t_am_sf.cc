#include <am/am_test/am_test.h>
#include <am/am_test/am_types.h>
#include <am/sdb_storage/sdb_storage.h>

using namespace std;

using namespace izenelib::am;
using namespace izenelib::am_test;

static string inputFile = "test.txt";
static string indexFile = "index.dat";
static int num = 1000000;
static bool rnd = 0;
static int loop = 1;
static bool trace = 0;

struct MyKeyType {
	unsigned int did;
	unsigned int tid1;
	unsigned int tid2;

	inline int compare(const MyKeyType& other) const {
		if (did != other.did) {
			return (did - other.did);
		} else if (tid1 != other.tid1) {
			return (tid1 - other.tid1);
		} else {
			return (tid2 - other.tid2);
		}
	}

};

MAKE_MEMCPY_TYPE(MyKeyType)

struct BigString {
	string str;
};

namespace izenelib {
namespace util {

template <> class izene_serialization<BigString> {
	const BigString& dat_;
public:
	izene_serialization(const BigString& dat) :
		dat_(dat) {

	}
	void write_image(char* &ptr, size_t &size) {
		ptr = (char*)dat_.str.c_str();
		size = dat_.str.size();
	}
};

template <> class izene_deserialization<BigString> {
	const char* ptr_;
	const size_t size_;
public:
	izene_deserialization(const char* ptr, const size_t size) :
		ptr_(ptr), size_(size) {
	}
	void read_image(BigString& dat) {
		dat.str = string(ptr_, size_);
	}
};

}

}

typedef float MyValueType;

namespace izenelib {
namespace am_test {

template<> inline MyKeyType generateData<MyKeyType>(const int a, int num,
		bool rand) {
	int b;
	if (rand)
		b = myrand()%num;
	else
		b = a;
	MyKeyType key;
	key.did = b;
	key.tid1 = myrand()%100;
	key.tid2 = myrand()%100;
	return key;
}

template<> inline BigString generateData<BigString>(const int a, int num,
		bool rand) {
	char p[20];
	int b;
	if (rand)
		b = myrand()%(num+1);
	else
		b = a;
	sprintf(p, "%08d", b);
	BigString bstr;
	string app(myrand()%10000, '*');
	bstr.str = string(p) + app;
	//cout<<bstr.str<<endl;
	return bstr;
}

}
}

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_am [-T <trace_option>] [-loop <num>][-n <num>] [-rnd <1|0>] <input_file>\n\n";
}

/*
void test1() {
	izenelib::util::ClockTimer timer;
	displayMemInfo();
	{
		cout<<"\nsdb_btree<MyKeyType, BigString >"<<endl;
		typedef sdb_btree<MyKeyType, BigString> CCCR_STRING_INT;
		AmTest<MyKeyType, BigString, CCCR_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(false);
		am.setNum(num);
		am.run_insert();
		am.setRandom(true);
		am.run_find();
	}
	printf("insert elapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	displayMemInfo();
	 {
	 cout<<"\nsdb_hash<MyKeyType, BigString >"<<endl;
	 typedef sdb_hash<MyKeyType, BigString> CCCR_STRING_INT;
	 AmTest<MyKeyType, BigString, CCCR_STRING_INT, true> am;
	 am.setTrace(trace);
	 am.setRandom(false);
	 am.setNum(num);
	 am.run_insert();
	 am.setRandom(true);
	 am.run_find();
	 }
	displayMemInfo();
	{
		cout<<"\ntc_btree<MyKeyType, BigString >"<<endl;
		typedef tc_btree<MyKeyType, BigString> CCCR_STRING_INT;
		AmTest<MyKeyType, BigString, CCCR_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(false);
		am.setNum(num);
		am.run_insert();
		am.setRandom(true);
		am.run_find();
	}
	printf("insert elapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	displayMemInfo();
	{
		cout<<"\ntc_hash<MyKeyType, BigString >"<<endl;
		typedef tc_hash<MyKeyType, BigString> CCCR_STRING_INT;
		AmTest<MyKeyType, BigString, CCCR_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(false);
		am.setNum(num);
		am.run_insert();
		am.setRandom(true);
		am.run_find();
	}
	printf("insert elapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	displayMemInfo();
	{
		cout<<"\nsdb_storage<MyKeyType, BigString >"<<endl;
		typedef sdb_storage<MyKeyType, BigString> CCCR_STRING_INT;
		AmTest<MyKeyType, BigString, CCCR_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(false);
		am.setNum(num);
		am.run_insert();
		am.setRandom(true);
		am.run_find();
	}
	printf("insert elapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	displayMemInfo();

}*/

void test() {

	/*	displayMemInfo();	
	 {
	 cout<<"\ntc_hash<MyKeyType, MyValueType >"<<endl;
	 typedef tc_hash<MyKeyType, MyValueType > CCCR_STRING_INT;
	 AmTest<MyKeyType, MyValueType, CCCR_STRING_INT, true> am;
	 am.setTrace(trace);
	 am.setRandom(false);
	 am.setNum(num);
	 am.run_insert();
	 am.setRandom(true);
	 am.run_find();
	 }
	*/ 
	/* displayMemInfo();	
	 {
	 cout<<"\nsdb_hash<MyKeyType, MyValueType >"<<endl;
	 typedef sdb_hash<MyKeyType, MyValueType > CCCR_STRING_INT;
	 AmTest<MyKeyType, MyValueType, CCCR_STRING_INT, true> am;
	 am.setTrace(trace);
	 am.setRandom(false);
	 am.setNum(num);
	 am.run_insert();
	 am.setRandom(true);
	 am.run_find();
	 }
	 displayMemInfo();*/
	
	izenelib::util::ClockTimer timer;

	 displayMemInfo();
	 {
	 cout<<"\nsdb_btree<MyKeyType, MyValueType >"<<endl;
	 typedef sdb_btree<MyKeyType, MyValueType > CCCR_STRING_INT;
	 AmTest<MyKeyType, MyValueType, CCCR_STRING_INT, true> am;
	 am.setTrace(trace);
	 am.setRandom(true);
	 am.setNum(num);
	 am.run_insert();
	 am.setRandom(true);
	 am.run_find();
	 }
	 printf("\nelapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
/*	 displayMemInfo();
	 {
		 cout<<"\nsdb_btree<string, MyValueType >"<<endl;
		 typedef sdb_btree<string, MyValueType > CCCR_STRING_INT;
		 AmTest<string, MyValueType, CCCR_STRING_INT, true> am;
		 am.setTrace(trace);
		 am.setRandom(false);
		 am.setNum(num);
		 am.run_insert();
		 am.setRandom(true);
		 am.run_find();
	 }
	 printf("\nelapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
/*	 displayMemInfo();
	 {
	 cout<<"\nsdb_hash<MyKeyType, MyValueType >"<<endl;
	 typedef sdb_hash<MyKeyType, MyValueType > CCCR_STRING_INT;
	 AmTest<MyKeyType, MyValueType, CCCR_STRING_INT, true> am;
	 am.setTrace(trace);
	 am.setRandom(false);
	 am.setNum(num);
	 am.run_insert();
	 am.setRandom(true);
	 am.run_find();
	 }
	 printf("\nelapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	 displayMemInfo();
	 {
		 cout<<"\nsdb_hash<string, MyValueType >"<<endl;
		 typedef sdb_hash<string, MyValueType > CCCR_STRING_INT;
		 AmTest<string, MyValueType, CCCR_STRING_INT, true> am;
		 am.setTrace(trace);
		 am.setRandom(false);
		 am.setNum(num);
		 am.run_insert();
		 am.setRandom(true);
		 am.run_find();
	 }
	 printf("\nelapsed 1 ( actually ): %lf seconds\n", timer.elapsed() );
	 displayMemInfo();*/

}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		ReportUsage();
		return 0;
	}
	argv++;
	while (*argv != NULL) {
		string str;
		str = *argv++;

		if (str[0] == '-') {
			str = str.substr(1, str.length());
			if (str == "T") {
				trace = bool(atoi(*argv++));
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = (bool)atoi(*argv++);
			} else if (str == "loop") {
				loop = atoi(*argv++);
			} else if (str == "index") {
				indexFile = *argv++;
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}
	try {
		test();
		//test1();
	}

	catch(bad_alloc)
	{
		cout<<"Memory allocation error!"<<endl;
	}
	catch(ios_base::failure)
	{
		cout<<"Reading or writing file error!"<<endl;
	}
	catch(...)
	{
		cout<<"OTHER ERROR HAPPENED!"<<endl;
	}

}
