#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>


#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>


using namespace std;
//using namespace ylib;
using namespace izenelib::cache;

static bool trace = 0; //trace option
static unsigned int cacheSize = 1000;
static double ratio = 1;
static int hash1 = 0;
static int hash2 = 1;
enum {EXT=0,LIN};
static int nReplace = 0;
enum {LRU=0,LFU, SLRU};
static string inputFile;
static string indexFile = "testing-data/mfcache_index.dat";

static int dumpOption = 0;

//Use YString-YString pair for testing. 
typedef string Key;
typedef izenelib::am::NullType Value;



typedef izenelib::am::LinearHashTable<Key,Value, NullLock> linFirstHash;
//typedef izenelib::am::ExtendibleHashTable<Key,Value, NullLock> extHash;
typedef izenelib::am::CCCR_StrHashTable<Key, Value> extFirstHash;

typedef izenelib::am::sdb_hash<Key, Value, NullLock> extSecondHash;
typedef izenelib::am::sdb_hash<Key, Value, NullLock> linSecondHash;


void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_mfcache [-T <trace_option>] [-size <cacheSize>] [-ratio <mem_file_proportion>] [-firsthash <ext|lin>] [-secondhash <ext|lin>]";
	cout
			<< "[-replace <lru|lfu|slru|...>] [-index <index_file>]  [-dump <dump_option>]<input_file>\n\n";

	cout
			<<"Example: /t_mfcache -T 1 -size 2000 -ratio 0.4 -firsthash ext -secondhash lin -replace lfu -index index.dat -dump 0 wordlist.txt\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-size <cacheSize>\n";
	cout<<"	Determinate the capacity of the cache manager.\n";

	cout<<"-ratio <mem_file_proportion>\n";
	cout
			<<"	 Determinate the  proportion of memory and file in terms of cache items in the cache storage. Default is 1.\n";

	cout<<"-firsthash <ext|lin>\n";
	cout
			<<"	ExtendibleHashMemory or LinearHashTable, default is ExtendibleHashMemory\n";

	cout<<"-secondhash <ext|lin>\n";
	cout<<"	ExtendibleHashFile or LinearHashFile, default is LinearHashFile\n";

	cout<<"-replace  <lru|lfu|slru|...>\n";
	cout<<"	Determinate replacement policy. Default is lru.\n";

	cout<<"-index <index_file>\n";
	cout<<"	Index data file that  CacheHash uses. Default is index.dat.\n";

	cout<<"-dump <dumpOption>\n";
	cout<<"	dumpOption = 0,  no dump;\n";
	cout
			<<"	dumpOption = 1,  when  accessing to a key-value pair that reside in FileHash,bring it onto memory.\n";
	cout
			<<"	DumpOption = 2,  evict the EVICT_NUM oldest items when hash_ is full, also bring the active item in file onto memory. \n";
	cout
			<<"	DumpOption=  3,   only evict the EVICT_NUM oldest items when hash_ is full.\n";
	cout
			<<"	DumpOption = 4,   Record the items need to dump and bring them to memory later by call dump().\n";

	cout<<"\n";
}

int sum =0;
int hit =0;
time_t start;

//Top-level 
template<typename T> void run(T& cm) {

	cout<<"Testing getValueWithInsert()"<<endl;
	run_getValueWithInsert(cm);

	cout<<"Ratio/2 Testing getValueWithInsert()"<<endl;
	cm.setRatio(ratio/2);
	run_getValueWithInsert(cm);

	cout<<"CacheSize*2 Testing getValueWithInsert()"<<endl;
	cm.setCacheSize(cacheSize*2);
	run_getValueWithInsert(cm);

	cout<<"CacheSize/2 Testing getValueWithInsert()"<<endl;
	cm.setCacheSize(cacheSize/2);
	run_getValueWithInsert(cm);

	cout<<"Testing getValue()"<<endl;
	run_getValue(cm);

	cout<<"Testing insertValue()"<<endl;
	run_insertValue(cm);

	cout<<"Testing flush()"<<endl;
	run_Update(cm);

	cout<<"Testing flush()"<<endl;
	run_flush(cm);

}

// test getValueWithInsert();
template<typename T> void run_getValueWithInsert(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		sum++;
		if (cm.getValueWithInsert(ystr, val) )
			hit++;
		if (trace) {
			cout<< "getValueWithInsert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MFCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}

	}
	if (dumpOption == DUMP_LATER) {
		cm.dump();
	}
	unsigned long rlimit = 0, vm = 0, rss = 0;

	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

	//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
	cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
	//sleep(5);

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


// test getValueWithInsert();
template<typename T> void run_insertValue(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		sum++;
		cm.insertValue(ystr, val);			
		if (trace) {
			cout<< "Insert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MFCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}

	}
	if (dumpOption == DUMP_LATER) {
		cm.dump();
	}
	unsigned long rlimit = 0, vm = 0, rss = 0;

	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

	//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
	cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
	//sleep(5);

	cout<<"MFCache with "<<"CacheSize="<<cacheSize<<" Ratio= "<<ratio<<endl;
	//cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	//cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

//Top-level 
template<typename T> void run_getValue(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		sum++;
		if (cm.getValue(ystr, val))
			hit++;
		if (trace) {
			cout<< "getValue: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MFCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	if (dumpOption == DUMP_LATER) {
		cm.dump();
	}

	cout<<"MFCache with "<<"CacheSize="<<cacheSize<<" Ratio= "<<ratio<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<<time(0)- start<<endl;
}

template<typename T> void run_Update(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	ifstream inf(inputFile.c_str());
	string ystr;
	while (inf>>ystr) {
		sum++;
		//cout<<"TimeToLive: "<<cm.getCacheInfo(ystr.get_key()).TimeToLive<<endl;					
		cm.setTimeToLive(ystr, 1);
		if (0) {
			cout<< "update: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cm.flush();
	cout<< "MCache numItem = "<<cm.numItems()<<endl;
	cm.displayHash();

	cout<<"eclipse:"<<time(0)- start<<endl;
}

//Top-level 
template<typename T> void run_flush(T& cm) {

	ifstream inf(inputFile.c_str());
	string ystr;
	while (inf>>ystr) {
		sum++;
		cm.flush(ystr);
		if (trace) {
			cout<< "flush: key="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MFCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cout<<"eclipse:"<<time(0)- start<<endl;
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
		//cout<< str<<endl;

		if (str[0] == '-') {
			str = str.substr(1, str.length());
			if (str == "T") {
				trace = bool(atoi(*argv++));
			} else if (str == "size") {
				cacheSize = atoi(*argv++);
			} else if (str == "ratio") {
				ratio = atof(*argv++);
			} else if (str == "firsthash") {
				string m = *argv++;
				if (m == "lin")
					hash1 = LIN;
				else {
				}//use default 				
			} else if (str == "secondhash") {
				string m = *argv++;
				if (m == "ext")
					hash2 = EXT;
				else {
				}//use default 		

			} else if (str == "replace") {
				string m = *argv++;
				if (m == "lru")
					nReplace = LRU;
				else if (m == "slru")
					nReplace = SLRU;
				else if (m == "lfu")
					nReplace = LFU;
				else {
				}// use default lru.				
			} else if (str == "index") {
				indexFile = *argv++;
			} else if (str == "dump") {
				dumpOption = atoi(*argv++);
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}

	try
	{

		if(nReplace == LRU)
		{
			if(hash1 == EXT)
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, lruCmp<Key>,extFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, lruCmp<Key>,extFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}
			else
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, lruCmp<Key>,linFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, lruCmp<Key>,linFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}

		}
		else if(nReplace == LFU)
		{
			if(hash1 == EXT)
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, lfuCmp<Key>,extFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, lfuCmp<Key>,extFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}
			else
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, lfuCmp<Key>,linFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, lfuCmp<Key>,linFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}

		}
		else if(nReplace == SLRU)
		{

			if(hash1 == EXT)
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, slruCmp<Key>,extFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, slruCmp<Key>,extFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}
			else
			{
				if(hash2 == EXT)
				{
					MFCache<Key, Value, slruCmp<Key>,linFirstHash,extSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
				else
				{
					MFCache<Key, Value, slruCmp<Key>,linFirstHash,linSecondHash, NullLock> cm(cacheSize, ratio, dumpOption, indexFile.c_str());run(cm);
				}
			}

		}
		else
		{
			cout<<"Bad input paramters"<<endl;
		}

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

	return 1;

}
