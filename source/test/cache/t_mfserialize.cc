#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>


#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>

//#include "YString.h"


using namespace std;
//using namespace ylib;
using namespace izenelib::cache;

static bool trace = 0; //trace option
static unsigned int cacheSize = 2000;
static double ratio =0.4;
enum {EXT=0,LIN};
enum {LRU=0,LFU, SLRU};
static string inputFile  = "test.txt";
static string indexFile = "mfserialize.dat";

static int dumpOption = 0;

//Use YString-YString pair for testing. 
typedef string YString;
typedef YString Key;
typedef YString Value;

//Storage policy.
typedef izenelib::am::LinearHashTable<Key,Value, NullLock> linFirstHash;
//typedef izenelib::am::ExtendibleHashTable<Key,Value, NullLock> extHash;
typedef izenelib::am::cccr_hash<Key, Value> extFirstHash;

typedef izenelib::am::sdb_hash<Key, Value, NullLock> extSecondHash;
typedef izenelib::am::sdb_hash<Key, Value, NullLock> linSecondHash;


static int sum =0;
static int hit =0;
static time_t start;

//Top-level 


// test getValueWithInsert();
template<typename T> void run_getValueWithInsert(T& cm) {	
	
	sum =0;
	hit =0;
	start = time(0);
	ifstream inf(inputFile.c_str());
	YString ystr;
	while (inf>>ystr) {
		sum++;
		if (cm.getValueWithInsert(ystr, ystr) )
			hit++;
		if (trace) {
			cout<< "getValueWithInsert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MFCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}

	}

	cout<<"MFCache with "<<"CacheSize="<<cacheSize<<" Ratio= "<<ratio<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

static void run() {

	/*cout<<"load from file."<<endl;
	 {
	 std::ifstream ifs("mfcache.bak");
	 boost::archive::text_iarchive ia(ifs);
	 cm.load(ia);
	 }*/
	MFCache<Key, Value, slruCmp<Key>,linFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());
	
	cout<<"Testing getValueWithInsert()"<<endl;
	run_getValueWithInsert(cm);
	std::ofstream ofs("mfcache.bak");
	{
		boost::archive::text_oarchive oa(ofs);
		cm.save(oa);
	}
	cm.displayHash();	

}


static void run_load() {
	
	MFCache<Key, Value, slruCmp<Key>,linFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());
	cm.displayHash();
	std::ifstream ifs("mfcache.bak");
	{
			boost::archive::text_iarchive ia(ifs);
			cm.load(ia);
	}
	cout<<"after load"<<endl;
	cm.displayHash();
	
	//cm.printKeyInfoMap();
	//trace = 1;
	run_getValueWithInsert(cm);
	
}


//int main(int argc, char *argv[])
BOOST_AUTO_TEST_CASE(t_mfserialize)
{		
	run();	
    run_load();		

}
