#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

//#include "YString.h"

#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>


using namespace std;
//using namespace ylib;
using namespace izenelib::cache;


typedef string YString;
typedef YString Key;
typedef YString Value;


typedef izenelib::am::LinearHashTable<Key,Value, izenelib::util::NullLock> FirstHash;
//typedef izenelib::am::ExtendibleHashTable<Key,Value, NullLock> extHash;
//typedef izenelib::am::cccr_hash<Key, Value> FirstHash;

typedef izenelib::am::sdb_hash<Key, Value, izenelib::util::NullLock> SecondHash;
typedef izenelib::am::sdb_hash<Key, Value, izenelib::util::NullLock> linSecondHash;

static bool trace = 1; //trace option
static int cacheSize = 2000;
static double ratio = 0.4;
enum {LRU=0,LFU, SLRU};
static string inputFile = "test.txt";
static string outputFile;
static char* indexFile = "index_1m.dat";
static char* indexDbFile = "index_3db.dat";
static int dumpOption = 0;

//Use YString-YString pair for testing. 

typedef MCache<Key, Value, slruCmp<Key>, FirstHash, ReadWriteLock> MCacheType;

MFCache<Key, Value, slruCmp<Key>, FirstHash, SecondHash> cm(cacheSize, ratio,
		dumpOption, indexFile);
MCache<Key, Value, slruCmp<Key>, FirstHash> mcache(cacheSize);
MLRUCache<Key, Value, FirstHash> mlrucache(cacheSize);
CacheDB<Key, Value ,slruCmp<Key>, MCacheType, SecondHash, ReadWriteLock> cachedb(cacheSize,
		indexDbFile);

static int sum =0;
static int hit =0;
static time_t start = time(0);

boost::mutex io_mutex;

//test MFCache
struct run_thread {
	run_thread(char *str_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		while (inf>>ystr) {
			sum++;
			if (cm.getValueWithInsert(ystr, ystr))
				hit++;
			if (trace) {
				boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getValueWithInsert: value="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "MFCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
			}

		}

	}
	char str[100];
};

//test MCache

struct run_thread_mc {
	run_thread_mc(char *str_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		while (inf>>ystr) {
			sum++;
			if (mcache.getValueWithInsert(ystr, ystr))
				hit++;
			if (trace) {
				boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getValueWithInsert: value="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "MCache numItem = "<<mcache.numItems()<<endl;
				mcache.displayHash();
			}

		}

	}
	char str[100];

};

//test MLRUCache

struct run_thread_mlruc {
	run_thread_mlruc(char *str_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		while (inf>>ystr) {
			sum++;
			if (mlrucache.getValueWithInsert(ystr, ystr))
				hit++;
			if (trace) {
				boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getValueWithInsert: value="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "MLRUCache numItem = "<<mlrucache.numItems()<<endl;
				//mlrucache.displayHash();
			}

		}

	}
	char str[100];

};

//test CacheDb


struct run_thread_cdb {
	run_thread_cdb(char *str_) {
		strcpy(str, str_);
	}
	void operator()() {
		ifstream inf(str);
		YString ystr;
		//int num =1000;
		//while (inf>>ystr && num--) 
		while( inf>>ystr ){
			sum++;
			if (cachedb.getValueWithInsert(ystr, ystr))
				hit++;
			if (trace) {
				//boost::mutex::scoped_lock lock(io_mutex);
				cout<<str<<": getValueWithInsert: value="<<ystr<<endl;
				//cm.printKeyInfoMap();
				cout<< "CacheDB numItem = "<<cachedb.numItems()<<endl;
				cachedb.displayHash();
			}

		}

	}
	char str[100];

};

static void run() {
#if 1	
	boost::thread_group threads;
	for (int i=1; i<=15; i++) {
		char fileName[100];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", i);
		threads.create_thread(run_thread(fileName) );
	}
	threads.join_all();
	cout<<"MFCache Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;

	//==================================================	
	sum =0;
	hit =0;
	start = time(0);
	boost::thread_group threads1;

	for (int i=1; i<=15; i++) {
		char fileName[100];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", i);
		threads1.create_thread(run_thread_mc(fileName) );
	}
	threads1.join_all();
	cout<<"MCache Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;


#endif


	sum =0;
	hit =0;
	start = time(0);
	boost::thread_group threads2;

	for (int i=1; i<=1; i++) {
		char fileName[100];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", i);
		threads2.create_thread(run_thread_cdb(fileName) );
	}
	threads2.join_all();
	cout<<"CacheDb Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;

	//==================================================	
	
	
	sum =0;
	hit =0;
	start = time(0);
	boost::thread_group threads3;

	for (int i=1; i<=15; i++) {
		char fileName[100];
		sprintf(fileName, "../db/dat/wordlist_%d.dat", i);
		threads3.create_thread(run_thread_mlruc(fileName) );
	}
	threads3.join_all();
	cout<<"MLRUCache Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;
	//==================================================		

}

//int main(int argc, char *argv[]) 
BOOST_AUTO_TEST_CASE(t_mul_cache)
{
	//if (argv[1])
	//	trace = atoi(argv[1]);
	run();
}

