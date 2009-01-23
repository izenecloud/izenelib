
#include <boost/memory.hpp>
//#include <boost/test/unit_test.hpp>
//#include <math.h>
#include <string>
#include <ctime>
//#include <time.h>

#include <am/slf/SkipListFile.h>

using namespace std;

#define SIZE 10

//BOOST_AUTO_TEST_SUITE( t_SkipListFile_suite )

bool trace = 1;

int degree = 2;

izenelib::am::SkipListFile<int, string> tb("sdb.dat", degree);

//BOOST_AUTO_TEST_CASE(Insertion_check )
void insert_test() {	
	clock_t start, finish;
	start = clock();
	for (int i=0; i<SIZE; i++) {
		if (trace) {
			cout<<"insert key="<<i<<endl;
		}
		char p[20];
		sprintf(p, "%08d", i);
		string str = p;
		tb.insert(i, str);
		if(trace){
			cout<<"numItem: "<<tb.num_items()<<endl<<endl;
			tb.display();
		}
		//cout<<"\nafte insert ...\n";		
	}
	cout<<"mumItem: "<<tb.num_items()<<endl;
	printf("\nIt takes %f seconds before commit()\n", (double)(clock() - start)
			/CLOCKS_PER_SEC);
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf("\nIt takes %f seconds to insert %d  data!\n", (double)(finish
			- start) / CLOCKS_PER_SEC, SIZE);

}

//BOOST_AUTO_TEST_CASE(Searching_check )
void search_test() {

	clock_t start, finish;
	string* v;
	start = clock();
	int c, b;
	c=b=0;

	for (int i=0; i<SIZE; i++) {
		v = tb.find(i);
		if ( v ){
			cout<<*v<<" found"<<endl;
			c++;
		}
		else
			b++;
	}
	if (trace)
		tb.display();
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to find %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE, c, b);

	//  tb.display(std::cout)
}

void delete_test() {

	clock_t start, finish;
	int c, b;
	c=b=0;

	start = clock();
	for (int i=0; i<SIZE/2; i++) {
		cout<<"del "<<i<<endl;
		if (tb.del(i) != 0)
			c++;
		else
			b++;
		if (trace){
			cout<<"numItem: "<<tb.num_items();
			tb.display();
		}
	}
	tb.flush();
	finish = clock();
	printf(
			"\nIt takes %f seconds to delete %d random data! %d data found, %d data lost!\n",
			(double)(finish - start) / CLOCKS_PER_SEC, SIZE/2, c, b);

}

/*
 BOOST_AUTO_TEST_CASE(Allocator_check )
 {
 clock_t start, finish;
 std::string* p[SIZE];
 start = clock();
 for (int j=0; j<32; j++)
 for (int i=0; i<SIZE; i++)
 {
 p[i] = new std::string();
 delete p[i];
 
 
 }

 finish = clock();
 printf( "\nNEW: It takes %f seconds to new 1000000 object\n",
 (double)(finish - start) / CLOCKS_PER_SEC);


 boost::scoped_alloc alloc_;
 start = clock();
 for (int j=0; j<32; j++)
 for (int i=0; i<SIZE; i++)
 {
 p[i] = BOOST_NEW(alloc_, std::string);
 
 }
 finish = clock();
 printf( "\nBOOST_NEW: It takes %f seconds to new 1000000 object\n",
 (double)(finish - start) / CLOCKS_PER_SEC);
 
 }*/

int main() {
	tb.setPageSize(50, 1024);
	tb.open();
	
	search_test();
	insert_test();
	search_test();
	delete_test();
	search_test();
	insert_test();
	search_test();
}

//BOOST_AUTO_TEST_SUITE_END()
