#include <am/btree/BTreeFile.h>
#include <boost/memory.hpp>
#include <boost/test/unit_test.hpp>
#include <math.h>
#include <string>
#include <time.h>

#define SIZE 1000000

BOOST_AUTO_TEST_SUITE( t_BTreeFile_suite )

int degree = 1024;

izenelib::am::BTreeFile<int, string> tb("sdb.dat");

BOOST_AUTO_TEST_CASE(Initialization) {
	tb.setDegree(degree);
	tb.setPageSize(60);
	tb.open();
}

BOOST_AUTO_TEST_CASE(Insertion_check) {

	//std::string s = "Kevin";

	clock_t start, finish;
	start = clock();
	for (int i=0; i<SIZE; i++) {
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		tb.insert(i, str);
	}
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d random data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, SIZE);
}

BOOST_AUTO_TEST_CASE(Searching_check) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();

	for (int i=0; i<SIZE; i++) {
		if (tb.find(i) )
			c++;
		else
			b++;
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to find %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE, c, b);

	//  tb.display(std::cout)
}

BOOST_AUTO_TEST_CASE(Delete_check) {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<SIZE/2; i++) {
		if (tb.del(i) != 0)
			c++;
		else
			b++;
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to delete %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE/2, c, b);

}

BOOST_AUTO_TEST_SUITE_END()
