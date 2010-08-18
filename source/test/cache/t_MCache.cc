#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

#include <cache/MCache.h>
#include <cache/MLRUCache.h>
#include <cache/MFCache.h>
#include <cache/CacheDB.h>
#include <cache/IzeneCache.h>

using namespace std;
//using namespace izenelib::cache;

static bool trace = 0; //trace option
static unsigned int cacheSize = 5000;
enum {EXT=0,LIN};
static int hash1 = 0;
static int nReplace = 1;
enum {LRU=0,LFU, SLRU};
static string inputFile = "test.txt";

//Use YString-YString pair for testing. 
typedef string Key;
typedef int Value;

//Storage policy.
typedef izenelib::am::LinearHashTable<Key, Value, NullLock> linHash;
//typedef ExtendibleHash<Key, Value, NullLock> extHash;
//typedef izenelib::am::cccr_hash<Key, Value> linHash;

typedef izenelib::am::rde_hash<Key, Value> extHash;


static void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_mcache [-T <trace_option>] [-size <cacheSize>]  [-hash <ext|lin>] [-replace <lru|lfu|slru|...>] <input_file>\n\n";

	cout<<"\nDescription:\n\n";
	cout
			<<"It will read from {input_file} and take the input words as the input keys to do testing.\n";

	cout<<"\nOption:\n\n";

	cout<<"-T <trace_option>\n";
	cout<<"	If set true, print out progress messages to console.\n";

	cout<<"-size <cacheSize>\n";
	cout<<"	Determinate the capacity of the cache manager.\n";

	cout<<"-hash <ext|lin>\n";
	cout
			<<"	ExtendibleHashMemory or LinearHashTable, default is ExtendibleHashMemory\n";

	cout<<"-replace  <lru|lfu|slru|...>\n";
	cout<<"	Determinate replacement policy. Default is lru.\n";

	cout<<"\n";
}

static  int sum =0;
static  int hit =0;
static  time_t start;

//Top-level 
template<typename T> void run(T& cm) {
	cout<<"Testing getValueWithInsert()"<<endl;
	run_getValueWithInsert(cm);

	/*cout<<"CacheSize*2 Testing getValueWithInsert()"<<endl;
	cm.setCacheSize(cacheSize*2);
	run_getValueWithInsert(cm);

	cout<<"CacheSize/2 Testing getValueWithInsert()"<<endl;
	cm.setCacheSize(cacheSize/2);
	run_getValueWithInsert(cm);

	cout<<"Testing getValue()"<<endl;
	run_getValue(cm);

	cout<<"Testing updateValue()"<<endl;
	run_updateValue(cm);

	cout<<"Testing insertValue()"<<endl;
	run_insertValue(cm);

	cout<<"Testing update"<<endl;
	run_Update(cm);

	cout<<"Testing flush(key)"<<endl;
	run_flush(cm);*/
}

// test getValueWithInsert();
template<typename T> void run_getValueWithInsert(T& cm) {
	//		ofstream outf("unique.out");		
	sum =0;
	hit =0;
	clock_t t1 = clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;

	while (inf>>ystr ) {
		sum++;
        val++;
		if (cm.getValueWithInsert(ystr, val))
			hit++;
		else {
			//	outf<<ystr<<endl;
		}
		if (trace) {
			cout<< "getValueWithInsert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}

	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	unsigned long rlimit = 0, vm = 0, rss = 0;

	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

	//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
	cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
	//sleep(5);

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

// test getValueWithInsert();
template<typename T> void run_insertValue(T& cm) {
	//		ofstream outf("unique.out");		
	sum =0;
	hit =0;
	start = time(0);
	clock_t t1= clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		val++;
		cm.insertValue(ystr, val);
		if (trace) {
			cout<< "getValueWithInsert: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}

	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	//cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

// test getValueWithInsert();
template<typename T> void run_updateValue(T& cm) {
	//		ofstream outf("unique.out");		
	sum =0;
	hit =0;
	start = time(0);
	clock_t t1= clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		val +=2;
		cm.updateValue(ystr, val);
		if (trace) {
			cout<< "update: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}

	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	double hitRatio, workload;
	cm.getEfficiency(hitRatio, workload);
	cout<<"\nTesting GetEfficency:"<<endl;
	//cout<<"HitRatio: "<<hitRatio<<endl;
	cout<<"workload: "<<workload<<endl;
	cm.resetStartingTime();

}

template<typename T> void run_getValue(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	clock_t t1= clock();
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
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
	cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

}

template<typename T> void run_Update(T& cm) {
	sum =0;
	hit =0;
	start = time(0);
	clock_t t1= clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	Value val;
	while (inf>>ystr) {
		sum++;
		val++;
		//cout<<"TimeToLive: "<<cm.getCacheInfo(ystr.get_key()).TimeToLive<<endl;					
		cm.setTimeToLive(ystr, 1);
		if (0) {
			cout<< "update: value="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	//cm.printKeyInfoMap();
	cm.flush();
	//cm.printKeyInfoMap();
	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<< "MCache numItem = "<<cm.numItems()<<endl;
	cm.displayHash();

	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
}

template<typename T> void run_flush(T& cm) {

	ifstream inf(inputFile.c_str());
	string ystr;
	clock_t t1= clock();
	while (inf>>ystr) {
		sum++;
		cm.flush(ystr);
		if (trace) {
			cout<< "flush: key="<<ystr<<endl;
			//cm.printKeyInfoMap();
			cout<< "MCache numItem = "<<cm.numItems()<<endl;
			cm.displayHash();
		}
	}
	//cout<<"Memory usage: " <<cm.getMemSizeOfValue()<<" bytes"<<endl;
	cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
}

/*
 //Top-level 
 template<typename T>
 void run(T& cm)
 {
 int sum =0;
 int hit =0; 
 time_t start = time(0);
 ifstream inf(inputFile.c_str());	
 YString ystr;		
 while(inf>>ystr)
 {
 sum++;	
 
 if(cm.getValueWithInsert(ystr.get_key(), ystr))
 hit++;	
 if(trace)
 {
 cout<< "getValueWithInsert: value="<<ystr<<endl;
 //cm.printKeyInfoMap();
 //cout<< "MCache numItem = "<<cm.numItems()<<endl;
 cm.displayHash();								
 }						
 }		
 
 cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
 cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
 cout<<"eclipse:"<<time(0)- start<<endl;	

 }
 */

//int main(int argc, char *argv[]) 
BOOST_AUTO_TEST_CASE(t_mcache)
{

	/*if (argc < 2) {
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
			} else if (str == "hash") {
				string m = *argv++;
				if (m == "lin")
					hash1 = LIN;
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
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}*/

	try
	{
		{	
			izenelib::cache::ILFUCache<Key,Value> cm(cacheSize);
			run(cm);
		}

		if(nReplace == LRU)
		{
			if(hash1 == EXT)
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::lruCmp<Key>, extHash, NullLock> cm(cacheSize);run(cm);
			}
			else
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::lruCmp<Key>, linHash, NullLock> cm(cacheSize);run(cm);
			}

		}
		else if(nReplace == LFU)
		{
			if(hash1 == EXT)
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::lfuCmp<Key>, extHash, NullLock> cm(cacheSize);run(cm);
			}
			else
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::lfuCmp<Key>, linHash, NullLock> cm(cacheSize);run(cm);
			}

		}
		else if(nReplace == SLRU)
		{

			if(hash1 == EXT)
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::slruCmp<Key>, extHash, NullLock> cm(cacheSize);run(cm);
			}
			else
			{
				izenelib::cache::MCache<Key, Value, izenelib::cache::slruCmp<Key>, linHash, NullLock> cm(cacheSize);run(cm);
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

	//return 1;

}
