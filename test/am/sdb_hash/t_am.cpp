#include <ctime>
#include <am/sdb_hash/sdb_hash.h>
#include <am/sdb_hash/sdb_fixedhash.h>
#include <am/am_test/am_test.h>

int rnd = 1;

using namespace izenelib::am_test;
using namespace izenelib::am;

static int num = 10000000;


int main()
{
	
	{
		cout<<"\nsdb_hash<uint64_t, string>"<<endl;
		typedef sdb_hash<uint64_t, string> SDB_STRING_INT;
		AmTest<uint64_t, string , SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setNum(num);
		run_am(am);
	}
	
	/*{
		cout<<"\nsdb_hash<int, vector<int> >"<<endl;
		typedef sdb_hash<int, vector<int> > SDB_STRING_INT;
		AmTest<int, vector<int>, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setTrace(true);
		am.setNum(num);
		run_am(am);
	}*/
	
	{
		cout<<"\nsdb_hash<string, int >"<<endl;
		typedef sdb_hash<string, int > SDB_STRING_INT;
		AmTest<string, int, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setNum(num);
		run_am(am);
	}
}


