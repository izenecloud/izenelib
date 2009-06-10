#include <am/am_test/am_test.h>
#include <am/am_test/am_types.h>

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

void test_loop() {
	{
		cout<<"\ncccr_hash<string, int, 7>"<<endl;
		typedef cccr_hash<string, int, 7> CCCR_STRING_INT;
		typedef AmTest<string, int, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

	{
		cout<<"\ncccr_hash<int, string, 7>"<<endl;
		typedef cccr_hash<int, string, 7> CCCR_STRING_INT;
		typedef AmTest<int, string, CCCR_STRING_INT> AMTEST;
		run_loop<AMTEST>(loop);
	}

}

void test (){
	{
		cout<<"\n cccr_hash<int, string>"<<endl;
		typedef cccr_hash<int, string> CCCR_INT_STRING;
		AmTest<int, string, CCCR_INT_STRING> am;
		am.setRandom(rnd);
		am.setNum(num);
		am.setTrace(trace);
		run_am(am);
	}

	{
		cout<<"\n cccr_hash<string, int>"<<endl;
		typedef cccr_hash<string, int> CCCR_INT_STRING;
		AmTest<string, int, CCCR_INT_STRING> am;
		am.setRandom(rnd);
		am.setNum(num);
		am.setTrace(trace);
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
		test();
		test_loop();	
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
