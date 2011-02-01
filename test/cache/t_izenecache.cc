#ifndef T_IZENECACHE_CPP_
#define T_IZENECACHE_CPP_

#include <cstdio>
#include <cstdlib>
#include <cache/IzeneCache.h>
#include <string>
#include <boost/test/unit_test.hpp>
#include <am/3rdparty/ext_hash_map.h>
#include <am/3rdparty/stl_map.h>
#include <am/3rdparty/rde_hash.h>
#include <am/3rdparty/stx_btree.h>

#include <util/ustring/UString.h>


namespace rde{

/*
template<> 
inline  hash_value_t extract_int_key_value<izenelib::util::UString>(const izenelib::util::UString& t){
	uint64_t convkey = 0;
	
	char* p = (char*)t.c_str();
	izenelib::util::UString::size_t length = t.length();	
	size_t size = length*sizeof(izenelib::util::UString::value_type);
	for (size_t i = 0; i < size; i++)
		convkey = 37*convkey + (uint8_t)*p++;
	return convkey;
}
*/

}


using namespace std;
using namespace izenelib::am;
using namespace izenelib::cache;

typedef string KeyType;
typedef int ValueType;
//typedef izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, RDE_HASH, LFU> MyCache;
typedef ILRUCache<KeyType, ValueType> MyCache;

static string inputFile("test.txt");
static string inputFile1("test1.txt");
static bool trace = true;
static size_t cacheSize = 2501;

BOOST_AUTO_TEST_SUITE( izene_cache_suite )

BOOST_AUTO_TEST_CASE(izene_cache_test)
{
	trace = 0;
	MyCache cm(cacheSize);
	{
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (trace) {
				cout<< "Insert: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
			if (cm.insertValue(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			//cm.displayCacheInfos();
		}

		cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

		unsigned long rlimit = 0, vm = 0, rss = 0;
		ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

		cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
		double hitRatio, workload;
		cm.getEfficiency(hitRatio, workload);
		cout<<"\nTesting GetEffclassicency:"<<endl;
		cout<<"HitRatio: "<<hitRatio<<endl;
		cout<<"workload: "<<workload<<endl;
		cm.resetStartingTime();
	}

	{
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			if (cm.getValue(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
			//cm.displayCacheInfos();
		}

		cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	}

	{
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.getValueWithInsert(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	}

	{
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile.c_str());
		string ystr;
		ValueType val = 100;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.updateValue(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cm.getValue(ystr, val);
				cout<< "update: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	}

	{
		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile.c_str());
		string ystr;
		ValueType val;
		while (inf>>ystr) {
			sum++;
			if (cm.del(ystr))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			assert( cm.getValue(ystr, val) == false );
			if (trace) {
				cout<< "del: key="<<ystr<<"->"<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"MCache with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;

	}

}

BOOST_AUTO_TEST_CASE(izene_cache_performance_test)
{
	trace = 0;
	{
		izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, RDE_HASH, LRU> cm(cacheSize*10);

		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile1.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.getValueWithInsert(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"RDE_HASH with "<<"CacheSize="<<cacheSize*10<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
		unsigned long rlimit = 0, vm = 0, rss = 0;

		ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

		//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
		cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

	}

	{
		izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, LINEAR_HASH, LRU> cm(cacheSize*10);

		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile1.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.getValueWithInsert(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"LINEAR_HASH with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
		unsigned long rlimit = 0, vm = 0, rss = 0;

		ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

		//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
		cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

	}

	/*{
		izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, STX_BTREE> cm(cacheSize*10);

		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile1.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.getValueWithInsert(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: "<<ystr<<" -> "<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"STX_BTREE with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
		unsigned long rlimit = 0, vm = 0, rss = 0;

		ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

		//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
		cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

	}*/

/*	{
		izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, CCCR_HASH> cm(cacheSize*10);

		int sum =0;
		int hit =0;
		clock_t t1 = clock();
		ifstream inf(inputFile1.c_str());
		string ystr;
		ValueType val = 0;
		while (inf>>ystr) {
			sum++;
			val++;
			if (cm.getValueWithInsert(ystr, val))
			hit++;
			else {
				//	outf<<ystr<<endl;
			}
			if (trace) {
				cout<< "get: key="<<ystr<<"->"<<val<<endl;
				cout<< "MCache numItem = "<<cm.numItems()<<endl;
				cm.displayHash();
				cout<<endl;
			}
		}

		cout<<"CCCR_HASH with "<<"CacheSize="<<cacheSize<<endl;
		cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
		cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
		unsigned long rlimit = 0, vm = 0, rss = 0;

		ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

		//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
		cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

	}*/
	
	trace = 0;
		{
			izenelib::cache::IzeneCache<izenelib::util::UString, ValueType, NullLock, RDE_HASH, LRU> cm(cacheSize*10);

			int sum =0;
			int hit =0;
			clock_t t1 = clock();
			ifstream inf(inputFile1.c_str());
			izenelib::util::UString ystr;
			string str;
			ValueType val = 0;
			while (inf>>str) {
				sum++;
				val++;
				ystr = izenelib::util::UString(str, izenelib::util::UString::CP949);
				if (cm.getValueWithInsert(ystr, val))
				hit++;
				else {
					//	outf<<ystr<<endl;
				}
				if (trace) {
					cout<< "get: key="<<str<<"->"<<val<<endl;
					cout<< "izeneCache (UString->int) numItem = "<<cm.numItems()<<endl;
					cm.displayHash();
					cout<<endl;
				}
			}

			cout<<"RDE_HASH(UString->int) with "<<"CacheSize="<<cacheSize*10<<endl;
			cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
			cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
			unsigned long rlimit = 0, vm = 0, rss = 0;

			ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

			//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
			cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

		}
		
	cout<<"\n=================LFU==================="<<endl;	
		trace = 0;
			{
				izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, RDE_HASH, LFU> cm(cacheSize*2);

				int sum =0;
				int hit =0;
				clock_t t1 = clock();
				ifstream inf(inputFile1.c_str());
				string ystr;
				ValueType val = 0;
				while (inf>>ystr) {
					sum++;
					val++;
					if (cm.getValueWithInsert(ystr, val))
					hit++;
					else {
						//	outf<<ystr<<endl;
					}
					if (trace) {
						cout<< "get: key="<<ystr<<"->"<<val<<endl;
						cout<< "MCache numItem = "<<cm.numItems()<<endl;
						cm.displayHash();
						cout<<endl;
					}
				}

				cout<<"RDE_HASH with "<<"CacheSize="<<cacheSize*2<<endl;
				cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
				cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
				unsigned long rlimit = 0, vm = 0, rss = 0;

				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
				cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

			}

			{
				izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, LINEAR_HASH, LFU> cm(cacheSize*2);

				int sum =0;
				int hit =0;
				clock_t t1 = clock();
				ifstream inf(inputFile1.c_str());
				string ystr;
				ValueType val = 0;
				while (inf>>ystr) {
					sum++;
					val++;
					if (cm.getValueWithInsert(ystr, val))
					hit++;
					else {
						//	outf<<ystr<<endl;
					}
					if (trace) {
						cout<< "get: key="<<ystr<<"->"<<val<<endl;
						cout<< "MCache numItem = "<<cm.numItems()<<endl;
						cm.displayHash();
						cout<<endl;
					}
				}

				cout<<"LINEAR_HASH with "<<"CacheSize="<<cacheSize<<endl;
				cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
				cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
				unsigned long rlimit = 0, vm = 0, rss = 0;

				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
				cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

			}

			/*{
				izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, STX_BTREE> cm(cacheSize*2);

				int sum =0;
				int hit =0;
				clock_t t1 = clock();
				ifstream inf(inputFile1.c_str());
				string ystr;
				ValueType val = 0;
				while (inf>>ystr) {
					sum++;
					val++;
					if (cm.getValueWithInsert(ystr, val))
					hit++;
					else {
						//	outf<<ystr<<endl;
					}
					if (trace) {
						cout<< "get: "<<ystr<<" -> "<<val<<endl;
						cout<< "MCache numItem = "<<cm.numItems()<<endl;
						cm.displayHash();
						cout<<endl;
					}
				}

				cout<<"STX_BTREE with "<<"CacheSize="<<cacheSize<<endl;
				cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
				cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
				unsigned long rlimit = 0, vm = 0, rss = 0;

				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
				cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

			}*/

		/*	{
				izenelib::cache::IzeneCache<KeyType, ValueType, NullLock, CCCR_HASH> cm(cacheSize*2);

				int sum =0;
				int hit =0;
				clock_t t1 = clock();
				ifstream inf(inputFile1.c_str());
				string ystr;
				ValueType val = 0;
				while (inf>>ystr) {
					sum++;
					val++;
					if (cm.getValueWithInsert(ystr, val))
					hit++;
					else {
						//	outf<<ystr<<endl;
					}
					if (trace) {
						cout<< "get: key="<<ystr<<"->"<<val<<endl;
						cout<< "MCache numItem = "<<cm.numItems()<<endl;
						cm.displayHash();
						cout<<endl;
					}
				}

				cout<<"CCCR_HASH with "<<"CacheSize="<<cacheSize<<endl;
				cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
				cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
				unsigned long rlimit = 0, vm = 0, rss = 0;

				ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

				//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
				cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

			}*/
			
			trace = 0;
				{
					izenelib::cache::IzeneCache<izenelib::util::UString, ValueType, NullLock, RDE_HASH, LFU> cm(cacheSize*2);

					int sum =0;
					int hit =0;
					clock_t t1 = clock();
					ifstream inf(inputFile1.c_str());
					izenelib::util::UString ystr;
					string str;
					ValueType val = 0;
					while (inf>>str) {
						sum++;
						val++;
						ystr = izenelib::util::UString(str, izenelib::util::UString::CP949);
						if (cm.getValueWithInsert(ystr, val))
						hit++;
						else {
							//	outf<<ystr<<endl;
						}
						if (trace) {
							cout<< "get: key="<<str<<"->"<<val<<endl;
							cout<< "izeneCache (UString->int) numItem = "<<cm.numItems()<<endl;
							cm.displayHash();
							cout<<endl;
						}
					}

					cout<<"RDE_HASH(UString->int) with "<<"CacheSize="<<cacheSize*2<<endl;
					cout<<"Hit ratio: "<<hit<<" / "<<sum<<endl;
					cout<<"eclipse:"<< double(clock()- t1)/CLOCKS_PER_SEC<<endl;
					unsigned long rlimit = 0, vm = 0, rss = 0;

					ProcMemInfo::getProcMemInfo(vm, rss, rlimit);

					//cout<<"memory usage: "<<cm.getMemSizeOfValue()<<"bytes"<<endl;	
					cout << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;

				}
	

}

BOOST_AUTO_TEST_SUITE_END()

#endif /*T_IZENECACHE_CPP_*/
