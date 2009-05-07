#include <ctime>
#include <am/sdb_hash/sdb_hash.h>
#include <am_test/am_test.h>

int rnd = 1;

using namespace izenelib::am_test;
using namespace izenelib::am;

static int num = 1000000;


int main()
{
	
	{
		cout<<"\nsdb_hash<int, unsigned int >"<<endl;
		typedef sdb_hash<int, unsigned  int > SDB_STRING_INT;
		AmTest<int, unsigned  int, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setNum(num);
		run_am(am);
	}
	
	{
		cout<<"\nsdb_hash<int, vector<int> >"<<endl;
		typedef sdb_hash<int, vector<int> > SDB_STRING_INT;
		AmTest<int, vector<int>, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setTrace(true);
		am.setNum(num);
		run_am(am);
	}
	
	{
		cout<<"\nsdb_hash<int, int >"<<endl;
		typedef sdb_hash<int, int > SDB_STRING_INT;
		AmTest<int, int, SDB_STRING_INT, true> am;		
		am.setRandom(rnd);	
		am.setNum(num);
		run_am(am);
	}
}


