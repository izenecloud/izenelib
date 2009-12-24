#include <sdb/SequentialDB.h>
//#include <am/btree/BTreeFile.h>
//#include <am/skip_list_file/SkipListFile.h>
#include "YString.h"

using namespace std;
using namespace izenelib::sdb;

using namespace boost;

static bool trace = 0; //trace option

//static int maxDataSize = 100;

string indexFile1("msdb1.dat");
string indexFile2("msdb2.dat");
string indexFile3("msdb3.dat");
string indexFile4("msdb4.dat");
string indexFile5("msdb5.dat");


//Use YString-YString pair for testing. 

typedef YString Key;
typedef izenelib::am::DataType<YString,NullType> MyDataType;

//typedef BTreeFile<Key, NullType, ReadWriteLock> BTF;
typedef sdb_btree<Key, NullType, ReadWriteLock> SBTREE;
typedef sdb_bptree<Key, NullType, ReadWriteLock> SBPTREE;
//typedef SkipListFile<Key, NullType, ReadWriteLock> SLF;
typedef sdb_hash<Key, NullType, ReadWriteLock> SHASH;

//typedef SequentialDB<Key, NullType, ReadWriteLock, BTF> SDB_BT;
//typedef SequentialDB<Key, NullType, ReadWriteLock, SLF> SDB_SL;
typedef SequentialDB<Key, NullType, ReadWriteLock, SHASH> SDB_HASH;
typedef SequentialDB<Key, NullType, ReadWriteLock, SBTREE> SDB_BTREE;
typedef SequentialDB<Key, NullType, ReadWriteLock, SBPTREE> SDB_BPTREE;

//SDB_BT cm1(indexFile1);
//SDB_SL cm2(indexFile2);
SDB_HASH cm3(indexFile3);
SDB_BTREE cm4(indexFile4);
SDB_BPTREE cm5(indexFile5);

int sum =0;
int hit =0;
time_t start = time(0);

boost::mutex io_mutex;

//test MFCache
template<typename T> struct run_thread1 {
	run_thread1(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		//	int n= 5000;
		while (inf>>ystr) {
			sum++;
			if (trace) {
				//	boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": insertValue: value="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t1 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			MyDataType val(ystr);
			if (cm.insertValue(val))
				hit++;

		}
		cm.flush();
		cout<< "t1 numItem = "<<cm.numItems()<<endl;

	}
	char str[1000];
	T& cm;
	//cm.flush();
};

template<typename T> struct run_thread2 {
	run_thread2(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		while (inf>>ystr) {
			sum++;
			if (trace) {
				//boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getValue: key="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t2 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			MyDataType val;
			if (cm.getValue(ystr.get_key(), val))
				hit++;

		}
		cm.flush();
		cout<< "t2 numItem = "<<cm.numItems()<<endl;

	}
	char str[1000];
	T& cm;
	//cm.flush();
};

template<typename T> struct run_thread3 {
	run_thread3(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		vector<MyDataType> result;
		int n= 100;
		while (inf>>ystr && n--) {
			sum++;
			if (trace) {
				//	boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getvalueforward: key="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t3 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			//break;
			cm.getValueForward(20, result, ystr);
			if (trace)
				cout<<result.size()<<endl;
			result.clear();
			//cm.getValueBackward(200, result, ystr);
			if (trace)
				cout<<result.size()<<endl;
			result.clear();
			//cm.getValueBetween(result, ystr, "zzz");
			if (trace)
				cout<<result.size()<<endl;
		}
		cout<< "t3 numItem = "<<cm.numItems()<<endl;
		cm.flush();

	}
	char str[1000];
	T& cm;
	//cm.flush();
};

template<typename T> struct run_thread4 {
	run_thread4(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		vector<MyDataType> result;
		//int n= 5000;
		while (inf>>ystr) {
			if (trace) {
				//	boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": del key: key="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t4 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			cm.del(ystr);
		}
		cout<< "t4 numItem = "<<cm.numItems()<<endl;
		cm.flush();
	}
	char str[1000];
	T& cm;
	//
};

template<typename T> struct run_thread5 {
	run_thread5(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		NullType val;
		//int n= 5000;
		while (inf>>ystr) {
			if (trace) {
				//	boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<"update key: key="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t5 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			cm.update(ystr, val);
		}
		cout<< "t5 numItem = "<<cm.numItems()<<endl;
		cm.flush();
	}
	char str[1000];
	T& cm;
	//
};

template<typename T> struct run_thread6 {
	run_thread6(T& cm_, char *str_) :
		cm(cm_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		//int n= 5000;
		while (inf>>ystr) {
			if (trace) {
				//	boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": Get Next,prev, nearest key: key="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "t6 numItem = "<<cm.numItems()<<endl;
				//cm.display();								
			}
			ystr = cm.getNext(ystr);
			if (trace)
				cout<<"getNext "<<ystr<<endl;
			ystr = cm.getPrev(ystr);
			if (trace)
				cout<<"getPrev "<<ystr<<endl;
			ystr = cm.getNearest(ystr);
			if (trace)
				cout<<"getNearest "<<ystr<<endl;
		}
		cout<< "t6 numItem = "<<cm.numItems()<<endl;
		cm.flush();
	}
	char str[1000];
	T& cm;
};

/*
 template<typename T> struct run_thread7 {
 run_thread7(T& cm_, char *str_) :
 cm(cm_) {
 strcpy(str, str_);
 }
 void operator()() {
 MyDataType dat;
 typename T::SDBCursor locn;
 while (cm.seq(locn, dat) ) {
 if (trace) {
 //	boost::mutex::scoped_lock lock(io_mutex);
 cout<<str<<": seq get key="<<dat.key<<endl;
 //cm.printKeyInfoMap();
 //cout<< "t7 numItem = "<<cm.numItems()<<endl;
 //cm.display();								
 }
 }
 cout<< "t7 numItem = "<<cm.numItems()<<endl;
 cm.flush();
 }
 char str[1000];
 T& cm;
 //
 };*/

template<typename T> void run1(T& cm) {

	boost::thread_group threads;
	//for(int i=1; i<=2; i++)
	{
		char fileName[1000];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", 1);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 2);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 3);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 4);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 5);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 6);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 9);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 10);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 6);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 7);
		threads.create_thread(run_thread4<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 8);
		threads.create_thread(run_thread4<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 9);
		threads.create_thread(run_thread5<T>(cm, fileName) );

		//=====================
//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 5);
//		threads.create_thread(run_thread3<T>(cm, fileName) );
//
//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 6);
//		threads.create_thread(run_thread3<T>(cm, fileName) );

		//sprintf(fileName, "../db/dat/wordlist_%d.dat", 10);
		//threads.create_thread(run_thread6<T>(cm, fileName) );

		//sprintf(fileName, "../db/dat/wordlist_%d.dat", 11);
		//threads.create_thread(run_thread7<T>(cm, fileName) );*/

	}
	threads.join_all();

	cout<<"SDB Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;
	cm.flush();

}

template<typename T> void run(T& cm) {

	boost::thread_group threads;
	for (int i=1; i<=2; i++) {
		char fileName[1000];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", 1);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 2);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 3);
		threads.create_thread(run_thread1<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 4);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 5);
		threads.create_thread(run_thread2<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 6);
		threads.create_thread(run_thread2<T>(cm, fileName) );

//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 7);
//		threads.create_thread(run_thread3<T>(cm, fileName) );
//
//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 8);
//		threads.create_thread(run_thread3<T>(cm, fileName) );
//
//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 9);
//		threads.create_thread(run_thread3<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 9);
		threads.create_thread(run_thread4<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 10);
		threads.create_thread(run_thread4<T>(cm, fileName) );

		sprintf(fileName, "../db/dat/wordlist_%d.dat", 9);
		threads.create_thread(run_thread5<T>(cm, fileName) );

//		sprintf(fileName, "../db/dat/wordlist_%d.dat", 10);
//		threads.create_thread(run_thread6<T>(cm, fileName) );

		//sprintf(fileName, "../db/dat/wordlist_%d.dat", 11);
		//threads.create_thread(run_thread7<T>(cm, fileName) );
	}
	threads.join_all();

	cout<<"SDB Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;
	cm.flush();

}

int main(int argc, char *argv[]) {
	if (argv[1])
		trace = atoi(argv[1]);
	//cm1.open();
	//run1(cm1);
	//run(cm1);
	//cm1.close();

	//cm2.open();
	//run1(cm2);
	//run(cm2);
	//cm2.close();

//	cm3.setCacheSize(10000);
//	cm3.open();
//	//run1(cm3);
//	run(cm3);
//	cm3.close();

	cm4.setCacheSize(10000);
	cm4.open();
	run(cm4);
	run1(cm4);
	cm4.close();
//	
//	cm5.setCacheSize(2000);
//	cm5.open();
//	run(cm5);
//	run1(cm5);
//	cm5.close();

}

