#include <util/bzip.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace izenelib::util;
using namespace std;

BOOST_AUTO_TEST_CASE(bzip_test)
{

	char *input = "abcd34252dfjsfljt23lsj lfserrrrrrrrrrrrrrrrrr111111111111111111111111111111111111111111111111111111 \
			11111111111111111111111111111111111111111111111111111111111111111111111111111111112u4o2u42j5l2j35l2jhl5ljljrl23525";
			
	int isz = strlen(input);

	char *output;
	int osz;

	char *output1;
	int osz1;

	cout<<"input: \n"<<input<<endl;
	cout<<"sz: \n"<<isz<<endl;
	output = _tc_bzcompress(input, isz, &osz);
	cout<<"output: \n"<<output<<endl;
	cout<<"sz: \n"<<osz<<endl;

	output1 = _tc_bzdecompress(output, osz, &osz1);
	cout<<"output1: \n"<<output1<<endl;
	cout<<"sz1: \n"<<osz1<<endl;
	
	
	BOOST_CHECK(isz == osz1);
	free(output);
	free(output1);

}
