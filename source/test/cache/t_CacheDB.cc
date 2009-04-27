#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cassert>

#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>

using namespace std;
//using namespace ylib;
using namespace izenelib::cache;

static bool trace = 0; //trace option
static unsigned int cacheSize = 1000;
static int hash1 = 0;
static int hash2 = 1;
enum {EXT=0,LIN};
static int nReplace = 0;
enum {LRU=0,LFU, SLRU};
static string inputFile;
static string indexFile = "index_db.dat";

//Use YString-YString pair for testing. 
typedef string Key;
typedef izenelib::am::NullType Value;

typedef izenelib::am::LinearHashTable<Key,Value, NullLock> linHash;
//typedef izenelib::am::ExtendibleHashTable<Key,Value, NullLock> extHash;
typedef izenelib::am::cccr_hash<Key, Value> extHash;

typedef izenelib::am::sdb_hash<Key, Value, NullLock> extDataHash;
typedef izenelib::am::sdb_hash<Key, Value, NullLock> linDataHash;
//typedef LinearHashFile<YString, YString, NullLock> linDataHash;

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_Cachedb [-T <trace_option>] [-size <cacheSize>]  [-mcache <ext|lin>] [-datahash <ext|lin>]";
	cout
			<< "[-replace <lru|lfu|slru|...>] [-index <index_file>]  <input_file>\n\n";

	cout
			<<"Example: /t_Cachedb -T 1 -size 2000  -mcache ext -datahash lin -replace lfu -index index.dat  wordlist.txt\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-size <cacheSize>\n";
	cout<<"	Determinate the capacity of the cache manager.\n";

	cout<<"-mcache <ext|lin>\n";
	cout
			<<"	ExtendibleHashMemory or LinearHashTable, default is ExtendibleHashMemory\n";

	cout<<"-datahash <ext|lin>\n";
	cout<<"	ExtendibleHashFile or LinearHashFile, default is LinearHashFile\n";

	cout<<"-replace  <lru|lfu|slru|...>\n";
	cout<<"	Determinate replacement policy. Default is lru.\n";

	cout<<"-index <index_file>\n";
	cout<<"	Index data file that  MCache uses. Default is index.dat.\n";

	cout<<"\n";
}

//Top-level 
template<typename T> void run(T& cm) {

	cout<<"Testing getValueWithInsert()"<<endl;
	run_getValueWithInsert(cm);

	cout<<"Testing getValue()"<<endl;
	run_getValue(cm);

	cout<<"Testing hashKey()"<<endl;	
	 run_hasKey(cm);

	 cout<<"Testing del()"<<endl;
	 run_del(cm);
	 
	 cout<<"CacheSize*2 Testing getValueWithInsert()"<<endl;
	 cm.setCacheSize(cacheSize*2);
	 run_getValueWithInsert(cm);

	 cout<<"CacheSize/2 Testing getValueWithInsert()"<<endl;
	 cm.setCacheSize(cacheSize/2);
	 run_getValueWithInsert(cm);
	 

	 cout<<"Testing getValue()"<<endl;
	 run_getValue(cm);	
	
	cm.displayHash();
	cm.clear();
	cm.displayHash();

}

//Top-level 

void run_DataHashTest() {

	cout<<"Testing DataHash....\n";
	linDataHash lh(indexFile);

	int sum =0;
	int hit =0;	
	clock_t t1 = clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {

		//cout<<"input "<<ystr<<endl;
		sum++;
		if (lh.insert(ystr, val) )
			hit++;
		if (trace) {
			cout<< "DataHashTest: insert  value="<<ystr<<endl;
			cout<< "LinHash numItem = "<<lh.num_items()<<endl;

		}
	}
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

}

//Top-level 
template<typename T> void run_getValueWithInsert(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {

		//cout<<"input "<<ystr<<endl;
		sum++;
		if (cm.getValueWithInsert(ystr, val))
			hit++;
		if (trace) {
			cout<< "getValueWithInsert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "CacheDB numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}

	cout<<"CacheDB with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

// test getValueWithInsert();
template<typename T> void run_del(T& cm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;
	while (inf>>ystr) {

		cm.del(ystr);
		if (trace) {
			cout<< "del: key="<<ystr<<endl;
			cout<< "CacheDB numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cout<<"CacheDb with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

}

//Top-level 
template<typename T> void run_getValue(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

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
			cout<< "CacheDB numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cout<<"CacheDB with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
}

template<typename T> void run_flush(T& cm) {

	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;

	cout<< "MCache numItem = "<<cm.numItems()<<endl;
	cm.displayHash();
	cm.flush();
	cout<< "MCache numItem = "<<cm.numItems()<<endl;
	cm.displayHash();
	while (inf>>ystr) {

		//cout<<"TimeToLive: "<<cm.getCacheInfo(ystr.get_key()).TimeToLive<<endl;					
		cm.flush(ystr);
		if (trace) {
			cout<< "flush: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "CacheDb numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cout<< "CacheDb numItem = "<<cm.numItems()<<endl;
	cm.displayHash();
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
}

//Top-level 
template<typename T> void run_hasKey(T& cm) {
	int sum =0;
	int hit =0;
	clock_t t1 = clock();

	ifstream inf(inputFile.c_str());
	string ystr;
	while (inf>>ystr) {
		sum++;
		if (cm.hasKey( ystr ) )
			hit++;
		if (trace) {
			cout<< "flush: key="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "CacheDB numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
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
			} else if (str == "mcache") {
				string m = *argv++;
				if (m == "lin")
					hash1 = LIN;
				else {
				}//use default 				
			} else if (str == "datahash") {
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
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}

	//try
	{
		//run_DataHashTest();	
		if(nReplace == LRU)
		{
			if(hash1 == EXT)
			{
				//typedef MCache<Key, Value, lruCmp<Key>, extHash, NullLock> dbMCache1;
				typedef MLRUCache<Key, Value, extHash, NullLock> dbMCache1;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, lruCmp<Key>, dbMCache1, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, lruCmp<Key>, dbMCache1, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}
			else
			{
				typedef MCache<Key, Value, lruCmp<Key>, linHash, NullLock> dbMCache2;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, lruCmp<Key>, dbMCache2, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, lruCmp<Key>, dbMCache2, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}

		}
		else if(nReplace == LFU)
		{
			if(hash1 == EXT)
			{
				typedef MCache<Key, Value, lfuCmp<Key>, extHash, NullLock> dbMCache3;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, lfuCmp<Key>, dbMCache3, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, lfuCmp<Key>, dbMCache3, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}
			else
			{
				typedef MCache<Key, Value, lfuCmp<Key>, linHash, NullLock> dbMCache4;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, lfuCmp<Key>, dbMCache4, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, lfuCmp<Key>, dbMCache4, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}

		}
		else if(nReplace == SLRU)
		{

			if(hash1 == EXT)
			{
				typedef MCache<Key, Value, slruCmp<Key>, extHash, NullLock> dbMCache5;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, slruCmp<Key>, dbMCache5, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, slruCmp<Key>, dbMCache5, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}
			else
			{
				typedef MCache<Key, Value, slruCmp<Key>, linHash, NullLock> dbMCache6;
				if(hash2 == EXT)
				{

					CacheDB<Key, Value, slruCmp<Key>, dbMCache6, extDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
				else
				{
					CacheDB<Key, Value, slruCmp<Key>, dbMCache6, linDataHash> cm(cacheSize, indexFile.c_str());run(cm);
				}
			}

		}
		else
		{
			cout<<"Bad input paramters"<<endl;
		}

	}
	/*catch(bad_alloc)
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
	}*/

	return 1;

}
