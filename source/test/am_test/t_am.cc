#include <am_test/am_test.h>
#include <am_test/am_types.h>

using namespace std;

using namespace izenelib::am;
using namespace izenelib::am_test;

static string inputFile = "test.txt";
static string indexFile = "index.dat";
static int num = 1000000;
static bool rnd = 0;
static int loop = 1;
static bool trace = 0;

void ReportUsage(void) {
	cout
			<<"\nUSAGE:./t_am [-T <trace_option>] [-loop <num>][-n <num>] [-rnd <1|0>] <input_file>\n\n";
}

void test_vector() {
	{
		cout<<"\ncccr_hash<int, vector<int> >"<<endl;
		typedef cccr_hash<int, vector<int> > CCCR_STRING_INT;
		AmTest<int, vector<int>, CCCR_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}
	
	{
		cout<<"\nwrapped_hash_map<int, vector<int> >"<<endl;
		typedef wrapped_hash_map<int, vector<int> > CCCR_STRING_INT;
		AmTest<int, vector<int>, CCCR_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}	


	{
		cout<<"\nsdb_btree<int, vector<int> >"<<endl;
		typedef sdb_btree<int, vector<int> > SDB_STRING_INT;
		AmTest<int, vector<int>, SDB_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}
	
	{
		cout<<"\ntc_hash<int, vector<int> >"<<endl;
		typedef tc_hash<int, vector<int> > SDB_STRING_INT;
		AmTest<int, vector<int>, SDB_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}
	

	{
		cout<<"\nsdb_hash<int, vector<int> >"<<endl;
		typedef sdb_hash<int, vector<int> > SDB_STRING_INT;
		AmTest<int, vector<int>, SDB_STRING_INT, true> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

}

void test_pre() {

	{
		cout<<"\nsdb_hash<string, NULLType>"<<endl;
		typedef sdb_hash<string, NullType> SDBHASH_STRING_INT;
		AmTest<string, NullType, SDBHASH_STRING_INT,true> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nsdb_hash<int, NULLTYPE>"<<endl;
		typedef sdb_hash<int, NullType> SDBHASH_STRING_INT;
		AmTest<int, NullType, SDBHASH_STRING_INT,true> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nsdb_btree<int, NULLTYPE>"<<endl;
		typedef sdb_btree<int, NullType> SDBHASH_STRING_INT;
		AmTest<int, NullType, SDBHASH_STRING_INT,true> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		typedef DynamicPerfectHash<string, int> DPH_STRING_INT;
		AmTest<string, int, DPH_STRING_INT> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	/*{
		cout<<"\nCCCR_StrHashTable<string, int>"<<endl;
		typedef CCCR_StrHashTable<string, int> CCCR_STRING_INT;
		AmTest<string, int, CCCR_STRING_INT> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}*/

	{
		cout<<"\ncccr_hash<string, int>"<<endl;
		typedef cccr_hash<string, int> CCCR_STRING_INT;
		AmTest<string, int, CCCR_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nwrapped_map<string, int>"<<endl;
		typedef wrapped_map<string, int> MAP_STRING_INT;
		AmTest<string, int, MAP_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nwrapped_hash_map<string, int>"<<endl;
		typedef wrapped_hash_map<string, int> HMAP_STRING_INT;
		AmTest<string, int, HMAP_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nLinearHashTable<string, int>"<<endl;
		typedef izenelib::am::LinearHashTable<string, int> LHT_STRING_INT;
		AmTest<string, int, LHT_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	cout<<"\n=================\n";
	{
		typedef DynamicPerfectHash<int, string> DPH_STRING_INT;
		AmTest<int, string, DPH_STRING_INT> am;
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\n cccr_hash<int, string>"<<endl;
		typedef cccr_hash<int, string> CCCR_INT_STRING;
		AmTest<int, string, CCCR_INT_STRING> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	/*{
		cout<<"\n CCCR_StrHashTable<int, string>"<<endl;
		typedef CCCR_StrHashTable<int, string, 131072, numeric_hash>
				CCCR_INT_STRING;
		AmTest<int, string, CCCR_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);
	}*/

	{
		cout<<"\nwrapped_map<int, string>"<<endl;
		typedef wrapped_map<int, string> MAP_STRING_INT;
		AmTest<int, string, MAP_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nwrapped_hash_map<int, string>"<<endl;
		typedef wrapped_hash_map<int, string> HMAP_STRING_INT;
		AmTest<int, string, HMAP_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}

	{
		cout<<"\nLinearHashTable<int, string>"<<endl;
		typedef izenelib::am::LinearHashTable<int, string> LHT_STRING_INT;
		AmTest<int, string, LHT_STRING_INT> am;
		am.setTrace(trace);
		am.setRandom(rnd);
		am.setNum(num);
		run_am(am);
	}
	{
		cout<<"\nDynamicPerfectHash<int, string>"<<endl;
		typedef DynamicPerfectHash<int, string> DPH_INT_STRING;
		AmTest<int, string, DPH_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);
	}

}

void test_loop() {

	/*{
		cout<<"\nCCCR_StrHashTable<string, int, 128>"<<endl;
		typedef CCCR_StrHashTable<string, int, 7> CCCR_STRING_INT;
		typedef AmTest<string, int, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}*/

	{
		cout<<"\ncccr_hash<string, int, 7>"<<endl;
		typedef cccr_hash<string, int, 7> CCCR_STRING_INT;
		typedef AmTest<string, int, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nwrapped_map<string, int>"<<endl;
		typedef wrapped_map<string, int> MAP_STRING_INT;
		typedef AmTest<string, int, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nwrapped_hash_map<string, int>"<<endl;
		typedef wrapped_hash_map<string, int> MAP_STRING_INT;
		typedef AmTest<string, int, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nLinearHashTable<string, int>"<<endl;
		typedef izenelib::am::LinearHashTable<string, int> MAP_STRING_INT;
		typedef AmTest<string, int, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}
	

	/*{
		cout<<"\nCCCR_StrHashTable<int, string, 64>"<<endl;
		typedef CCCR_StrHashTable<int, string, 64> CCCR_STRING_INT;
		typedef AmTest<int, string, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}*/

	{
		cout<<"\ncccr_hash<int, string, 7>"<<endl;
		typedef cccr_hash<int, string, 7> CCCR_STRING_INT;
		typedef AmTest<int, string, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nwrapped_map<int, string>"<<endl;
		typedef wrapped_map<int, string> MAP_STRING_INT;
		typedef AmTest<int, string, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nwrapped_hash_map<int, string>"<<endl;
		typedef wrapped_hash_map<int, string> MAP_STRING_INT;
		typedef AmTest<int, string, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\nLinearHashTable<int, string>"<<endl;
		typedef izenelib::am::LinearHashTable<int, string> MAP_STRING_INT;
		typedef AmTest<int, string, MAP_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

}

void test_izenelib() {
	{
		cout<<"\nsdb_hash<string, int>"<<endl;
		typedef sdb_hash<string, int> SDBHASH_STRING_INT;
		AmTest<string, int, SDBHASH_STRING_INT,true> am;
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nwrapped_map<string, int>"<<endl;
		typedef wrapped_map<string, int> MAP_STRING_INT;
		AmTest<string, int, MAP_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);
		//
	}

	{
		cout<<"\nwrapped_hash_map<string, int>"<<endl;
		typedef wrapped_hash_map<string, int> HMAP_STRING_INT;
		AmTest<string, int, HMAP_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);
		//
	}

	{
		cout<<"\nLinearHashTable<string, int>"<<endl;
		typedef izenelib::am::LinearHashTable<string, int> LHT_STRING_INT;
		AmTest<string, int, LHT_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\ncccr_hash<string, int>"<<endl;
		typedef cccr_hash<string, int> CCCR_STRING_INT;
		AmTest<string, int, CCCR_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);

	}

	/*{
		cout<<"\nCCCR_StrHashTable<string, int>"<<endl;
		typedef CCCR_StrHashTable<string, int> CCCR_STRING_INT;
		AmTest<string, int, CCCR_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);
	}*/

	{
		cout<<"\nsdb_btree<string, int>"<<endl;
		typedef sdb_btree<string, int> SBTREE_STRING_INT;
		AmTest<string, int, SBTREE_STRING_INT, true> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\ntc_hash<string, int>"<<endl;
		typedef tc_hash<string, int> TH_STRING_INT;
		AmTest<string, int, TH_STRING_INT, true> am;
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"DynamicPerfectHash<string, int>"<<endl;
		typedef DynamicPerfectHash<string, int> DPH_STRING_INT;
		AmTest<string, int, DPH_STRING_INT> am;
		am.setRandom(rnd);
		run_am(am);
	}

}

void test_izenelib1() {
	{
		cout<<"\nsdb_hash<int, string>"<<endl;
		typedef sdb_hash<int, string> SDBHASH_INT_STRING;
		AmTest<int, string, SDBHASH_INT_STRING,true> am;
		am.setRandom(rnd);
		run_am(am);
	}

	{
		cout<<"\nwrapped_map<int, string>"<<endl;
		typedef wrapped_map<int, string> MAP_INT_STRING;
		AmTest<int, string, MAP_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\nwrapped_hash_map<int, string>"<<endl;
		typedef wrapped_hash_map<int, string> HMAP_INT_STRING;
		AmTest<int, string, HMAP_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);
	}
	{
		cout<<"\nLinearHashTable<in, string>"<<endl;
		typedef izenelib::am::LinearHashTable<int, string> LHT_INT_STRING;
		AmTest<int, string, LHT_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);

	}

	/*{
		cout<<"\n CCCR_StrHashTable<int, string>"<<endl;
		typedef CCCR_StrHashTable<int, string, 131072, numeric_hash>
				CCCR_INT_STRING;
		AmTest<int, string, CCCR_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);
	}*/

	{
		cout<<"\n cccr_hash<int, string>"<<endl;
		typedef cccr_hash<int, string> CCCR_INT_STRING;
		AmTest<int, string, CCCR_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\nsdb_btree<int, string>"<<endl;
		typedef sdb_btree<int, string> SBTREE_INT_STRING;
		AmTest<int, string, SBTREE_INT_STRING, true> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\ntc_hash<int, string>"<<endl;
		typedef tc_hash<int, string> TH_INT_STRING;
		AmTest<int, string, TH_INT_STRING, true> am;
		am.setRandom(rnd);
		run_am(am);

	}

	{
		cout<<"\nDynamicPerfectHash<int, string>"<<endl;
		typedef DynamicPerfectHash<int, string> DPH_INT_STRING;
		AmTest<int, string, DPH_INT_STRING> am;
		am.setRandom(rnd);
		run_am(am);
	}

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

		if (str[0] == '-') {
			str = str.substr(1, str.length());
			if (str == "T") {
				trace = bool(atoi(*argv++));
			} else if (str == "n") {
				num = atoi(*argv++);
			} else if (str == "rnd") {
				rnd = (bool)atoi(*argv++);
			} else if (str == "loop") {
				loop = atoi(*argv++);
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
	try {	
		test_vector();
		
		test_loop();
		test_pre();
		test_izenelib();
		test_izenelib1();
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

}
