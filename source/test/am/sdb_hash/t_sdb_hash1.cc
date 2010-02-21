#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_btree/sdb_btree.h>
#include <boost/test/unit_test.hpp>

using namespace izenelib::am;

typedef string KeyType;
typedef int ValueType;
typedef sdb_hash<KeyType, ValueType> SDB_HASH;
typedef sdb_btree<KeyType, ValueType> SDB_BTREE;
typedef SDB_HASH::SDBCursor SDBCursor;

static string inputFile("../../db/test2.txt");
static string inputFile1("../../db/wordlist.txt");
//static string inputFile2("../../db/wordlist_PLU.txt");
static string inputFile2("../../db/wordlist_PLU.txt");
static bool trace = false;
static int num = 10000000;

static SDB_HASH cm("shash.dat");
static SDB_BTREE cm1("btree.dat");


struct MyKeyType{
	int a;
	string b;
	int compare(const MyKeyType&)const {
		return 1;		
	}
	
	DATA_IO_LOAD_SAVE(MyKeyType, &a&b);
	template <class Archive> void serialize(Archive& ar,
			const unsigned int version) {
		ar& a;
		ar& b;	
	}
};


MAKE_FEBIRD_SERIALIZATION( MyKeyType );

static void test_user_defined_type(){
	sdb_hash<MyKeyType, int> sdb1;
	sdb_hash<MyKeyType, int> sdb2;	
}


static void test_mykeytype(){
	sdb_hash<MyKeyType, int> cm("cool.dat");
	cm.open();
	cout<<"\ninsert_test"<<endl;
	cout<<"SerialType: "<<IsFebirdSerial<std::vector<MyKeyType>  >::yes<<endl; 
	cout<<"SerialType: "<<IsFebirdSerial<MyKeyType >::yes<<endl; 
	{
		//trace = true;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile2.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			val++;
			sum++;
			MyKeyType key;
			key.a = val;
			key.b = ystr;
			if ( sum>num )
				break;
			if (trace) {
				cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
			}
			if (cm.insert(key, val) ) {
				hit++;
			}
		}
		if (trace) {
			cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
			cout<< "b=tree numItem = "<<cm.num_items()<<endl;
			cm.display(cout, false);
		}
		cm.flush();
		cm.display();
		//int num = cm.num_items();
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
	}
	
	
	
}

static void test_openclose() {
	{
		SDB_HASH tdb("t1.dat");
		tdb.open();
		tdb.close();
		tdb.open();
	}
	{
		SDB_HASH tdb("t2.dat");
		tdb.close();
		tdb.open();
		tdb.insert("a", 1);
		tdb.insert("a33", 222);
		tdb.close();
		tdb.open();
	}
	{
		SDB_HASH tdb("t3.dat");
		tdb.open();
		//tdb.display(cout, false);
		//tdb.flush();
		//while(1);
	}
}

template<typename T> void insert_test1() {

	{
		trace = false;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile2.c_str());
		string ystr;
		ValueType val = 0;
		//int cnt = 0;
		while (inf>>ystr) {
			val++;
			sum++;
			if (sum>num)
				break;
			if (trace) {
				cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
			}
			if (cm.insert(ystr, val) ^ cm1.insert(ystr, val) ) {
				hit++;
			} else {
				if (trace) {
					cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
					cout<<sum<<endl;
					cm.display(cout, false);
					cout<<"\n\n========================\n\n";
					cm1.display(cout, false);
					//assert(false);
					cout<<ystr<<endl;
				}
			}
			if (trace) {
				cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.num_items()<<endl;
				cm.display(cout, false);
			}
		}
		cm.flush();
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
	}

}

template<typename T> void seq_test(T& cm) {
	cout<<"\nseq_test"<<endl;

#if 1
	{
		trace = false;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();

		//cm1.display(cout, false);
		cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1"<<endl;

		//cm.display(cout, false);

		KeyType key= "  ";
		KeyType last_key="";
		ValueType val = 0;
		//typedef  SDB_BPTREE::SDBCursor SDBCursor;
		SDBCursor locn;
		locn = cm.get_first_locn();	
		cout <<"abc"<<endl;
		while (cm.get(locn, key, val)) {
			if (last_key.compare(key) == 0)
				assert(false);
			if (trace)
				cout<<key<<"/"<<val<<" -> ";
			++sum;
			cm.seq(locn);
			last_key = key;
		}
		cout<<"\nseq total num: "<<sum<<endl;
		
		cm.flush();
		//cm.display(cout, false);
		cout<<"\nseq total num: "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
		trace = false;

	}
#endif	

}

template<typename T> void insert_test(T& cm) {
	cout<<"\ninsert_test"<<endl;
	{
		//trace = true;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile2.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			val++;
			sum++;
			if (sum>num)
				break;
			if (trace) {
				cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
			}
			if (cm.insert(ystr, val) ) {
				hit++;
			}
		}
		if (trace) {
			cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
			cout<< "b=tree numItem = "<<cm.num_items()<<endl;
			cm.display(cout, false);
		}
		cm.flush();
		cm.display();
		//int num = cm.num_items();
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
	}

}

template<typename T> void get_test(T& cm) {
	cout<<"\nget_test"<<endl;

#if 1
	{
		trace = false;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile2.c_str());
		string ystr;
		ValueType val = 0;
		//int cnt = 0;
		while (inf>>ystr) {
			++sum;
			if (sum>num)
				break;
			if (trace) {
				cout<< "get: key="<<ystr<<endl;
			}
			if (cm.get(ystr, val) ) {
				hit++;
				if (trace) {
					cout<< "get key->val: "<<ystr<<"->"<<val<<endl;
					//cm.display(cout, false);
				}
			}

		}
		//BOOST_CHECK_EQUAL(hit, 100000);
		//assert(hit == 100000);
		cm.flush();
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
	}
#endif	

}

template<typename T> void del_test(T& cm) {
	cout<<"\ndel_test"<<endl;
#if 1
	{
		trace = false;
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile2.c_str());
		string ystr;
		ValueType val = 0;
		//int cnt = 0;
		while (inf>>ystr) {
			++sum;
			if (sum> num)
				break;
			if (trace) {
				cout<< "del: key="<<ystr<<endl;
			}
			if (cm.del(ystr) ) {
				hit++;
			}
			if (trace) {
				//cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cm.display(cout, false);
			}

		}
		//BOOST_CHECK_EQUAL(hit, 100000);
		//assert(hit == 100000);
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	}
#endif	

}

BOOST_AUTO_TEST_SUITE( shash_suite )
BOOST_AUTO_TEST_CASE(shash_open_close_test)
{
	test_openclose();
	test_user_defined_type();
	test_mykeytype();
}


BOOST_AUTO_TEST_CASE(shash_test)
{
	//cm.setCacheSize(10000);
	//cm.setDegree(3);
	cm.open();

	get_test(cm);
	seq_test(cm);
	insert_test(cm);
	get_test(cm);
	seq_test(cm);
	del_test(cm);
	get_test(cm);
	seq_test(cm);
	insert_test(cm);
	get_test(cm);
	seq_test(cm);

	num = 10000;
	del_test(cm);
	get_test(cm);
	seq_test(cm);

	insert_test(cm);
	get_test(cm);
	seq_test(cm);

	cm.close();
	cm1.close();
}

BOOST_AUTO_TEST_SUITE_END()

