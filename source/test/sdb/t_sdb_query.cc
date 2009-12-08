#include <am/am_test/am_test.h>
#include <am/am_test/am_types.h>

using namespace std;

using namespace izenelib::am;
using namespace izenelib::am_test;

struct MyValueType {
	string str;
	DATA_IO_LOAD_SAVE(MyValueType, &str)
};

MAKE_FEBIRD_SERIALIZATION(MyValueType)

namespace izenelib {
	namespace am_test {

		template<> inline MyValueType generateData<MyValueType>(const int a, int num,
				bool rand) {
			MyValueType val;
			int n = myrand()%200;
			val.str = string(n*10, 'c');
			return val;
		}

	}
}

typedef int KeyType;
typedef MyValueType ValueType;
static bool rnd = false;
static int num = 500000;
static int num1 = 100000;
static int is_insert = 1;

void test() {

	//	{
	//		cout<<"\nsdb_hash<"<<endl;
	//		typedef sdb_hash<KeyType, ValueType> SDB;
	//		AmTest<KeyType, ValueType, SDB, true> am;
	//		am.setRandom(rnd);
	//		am.setNum(num);
	//		if (is_insert)
	//			am.run_insert();
	//		am.setRandom(true);
	//		am.setNum(200);
	//		am.run_find();
	//	}

	//	{
	//		cout<<"\nsdb_storage<"<<endl;
	//		typedef sdb_storage<KeyType, ValueType> SDB;
	//		AmTest<KeyType, ValueType, SDB, true> am;
	//		am.setRandom(rnd);
	//		am.setNum(num);
	//		if (is_insert)
	//			am.run_insert();
	//		am.setRandom(true);
	//		am.setNum(num1);
	//		am.run_find();
	//	}


	{
		cout<<"\nsdb_bptree<"<<endl;
		typedef sdb_bptree<KeyType, ValueType> SDB;
		AmTest<KeyType, ValueType, SDB, true> am;
		am.setRandom(rnd);
		am.setNum(num);
		if (is_insert)
			am.run_insert();
		am.setRandom(true);
		//		am.setNum(1);
		//		am.run_find();
		am.setNum(200);
		am.run_find();
		am.setNum(num1);
		am.run_find();
	}

	{
		cout<<"\ntc_hash<"<<endl;
		typedef tc_hash<KeyType, ValueType> SDB;
		AmTest<KeyType, ValueType, SDB, true> am;
		am.setRandom(rnd);
		am.setNum(num);
		if (is_insert)
			am.run_insert();
		am.setRandom(true);
		//			am.setNum(1);
		//			am.run_find();
		am.setNum(200);
		am.run_find();
		am.setNum(num1);
		am.run_find();
	}

	{
		cout<<"\nsdb_btree<"<<endl;
		typedef sdb_btree<KeyType, ValueType> SDB;
		AmTest<KeyType, ValueType, SDB, true> am;
		am.setRandom(rnd);
		//		am.setNum(num);
		if (is_insert)
			am.run_insert();
		am.setRandom(true);
		//		am.setNum(1);
		//		am.run_find();
		am.setNum(200);
		am.run_find();
		am.setNum(num1);
		am.run_find();
	}

	//	{
	//		cout<<"\ntc_btree<"<<endl;
	//		typedef tc_btree<KeyType, ValueType> SDB;
	//		AmTest<KeyType, ValueType, SDB, true> am;
	//		am.setRandom(rnd);
	//		am.setNum(num);
	//		if (is_insert)
	//			am.run_insert();
	//		am.setRandom(true);
	//		am.setNum(num1);
	//		am.run_find();
	//	}

}

int main(int argc, char *argv[]) {
	if (argv[1])
		is_insert = atoi(argv[1]);
	test();
	return 1;
}

