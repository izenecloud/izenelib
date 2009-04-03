#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

#include "YString.h"

#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>


using namespace std;
using namespace ylib;
using namespace izenelib::cache;

static bool trace = 0; //trace option
static unsigned int cacheSize = 2000;
static double ratio =0.4;
enum {EXT=0,LIN};
enum {LRU=0,LFU, SLRU};
static string inputFile = "../db/test2.txt";
static string indexFile = "index.dat";


//Use YString-YString pair for testing. 
typedef YString Key;
typedef YString Value;

//Storage policy.
typedef izenelib::am::LinearHashTable<Key,Value, NullLock> linFirstHash;
//typedef izenelib::am::ExtendibleHashTable<Key,Value, NullLock> extHash;
typedef izenelib::am::CCCR_StrHashTable<Key, Value> extFirstHash;

int sum =0;
int hit =0;
time_t start;


template<typename T> void run_getValueWithInsert(T& cm);
//Top-level 
 void run() {

	/*cout<<"load from file."<<endl;
	 {
	 std::ifstream ifs("mfcache.bak");
	 boost::archive::text_iarchive ia(ifs);
	 cm.load(ia);
	 }*/
	MCache<Key, Value, lfuCmp<Key>,linFirstHash, NullLock> cm(cacheSize);

	cout<<"Testing getValueWithInsert()"<<endl;
	run_getValueWithInsert(cm);
	std::ofstream ofs("mcache.bak");
	{
		boost::archive::text_oarchive oa(ofs);
		cm.save(oa);
	}
	cm.displayHash();
	

}

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
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}

	cout<<"MCache with "<<"CacheSize="<<cacheSize<<" Ratio= "<<ratio<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;	

}

 void run_load() {	
	MCache<Key, Value, slruCmp<Key>, linFirstHash, NullLock> cm(cacheSize);
	std::ifstream ifs("mcache.bak");
	{
			boost::archive::text_iarchive ia(ifs);
			cm.load(ia);
	}	
	cm.displayHash();
	
	//cm.printKeyInfoMap();
	//trace = 1;
	run_getValueWithInsert(cm);
	
}


int main(int argc, char *argv[])
{	
	run();	
	run_load();		

}
