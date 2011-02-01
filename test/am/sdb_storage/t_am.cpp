#include <ctime>
#include <am/sdb_storage/sdb_storage.h>
#include <am/sdb_storage/sdb_storage_mm.h>
#include <am/am_test/am_test.h>

int rnd = 1;

using namespace izenelib::am_test;
using namespace izenelib::am;

static int num = 100000000;


int main()
{
	
	{
		cout<<"\nsdb_storage<int, string>"<<endl;
		typedef sdb_storage_mm<int, string> SDB_STRING_INT;
		AmTest<int, string , SDB_STRING_INT, true> am;		
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
		cout<<"\nsdb_storage<string, int >"<<endl;
		typedef sdb_storage<string, int > SDB_STRING_INT;
		AmTest<string, int, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setNum(num);
		run_am(am);
	}
}


