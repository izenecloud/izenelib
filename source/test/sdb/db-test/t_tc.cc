#include <sdb/SequentialDB.h>
#include <tcutil.h>
#include <tcadb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

using namespace std;

static int num = 1000000;
static bool trace = 0;
static int degree;
static string inputFile;
static bool rnd = 0;

template<class T> struct MyKeyType {
	friend class boost::serialization::access;
	T key; //not use ystring here, for ystring has no serialize member.
	int data;

	//must be const now
	const string get_key() const {
		return key;
	}

	inline int compare(const MyKeyType& other) {
		return key.compare(other.key);
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & key;
		ar & data;
	}

	void display() const {
		cout<<key<<" "<<data<<" ";
	}

};

template<> struct MyKeyType<int> {
	int key; //not use ystring here, for ystring has no serialize member.
	int data;

	//must be const now
	const int get_key() const {
		return key;
	}

	inline int compare(const MyKeyType& other) {
		return key-other.key;
	}

	void display() const {
		cout<<key<<" "<<data<<" ";
	}

};

NS_IZENELIB_AM_BEGIN

namespace util {

	template<> inline void read_image<MyKeyType<int> >(MyKeyType<int>& dat, const DbObjPtr& ptr) {
		memcpy(&dat, ptr->getData(), sizeof(MyKeyType<int>));
	}

	template<> inline void write_image<MyKeyType<int> >(const MyKeyType<int>& dat, DbObjPtr& ptr) {
		ptr->setData(&dat, sizeof(MyKeyType<int>));
	}
}

NS_IZENELIB_AM_END

int compare(const char* aptr, int asize, const char* bptr, int bsize,
		void* op=0) {
	DbObjPtr ptr, ptr1;
	MyKeyType<string> key, key1;
	ptr.reset(new DbObj(aptr,asize));
	ptr1.reset(new DbObj(bptr, bsize));
	read_image(key, ptr);
	read_image(key1, ptr1);
	return key.compare(key1);
}

int compare1(const char* aptr, int asize, const char* bptr, int bsize,
		void* op=0) {
	DbObjPtr ptr, ptr1;
	MyKeyType<int> key, key1;
	ptr.reset(new DbObj(aptr,asize));
	ptr1.reset(new DbObj(bptr, bsize));
	read_image(key, ptr);
	read_image(key1, ptr1);
	return key.compare(key1);

}

/* pseudo random number generator */
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

void test_insert1(const char *name) {
	TCADB *adb = tcadbnew();
	/* open the database */
	if (!tcadbopen(adb, name)) {
		fprintf(stderr, "open error\n");
	}
	clock_t t1 = clock();
	for (int i=0; i<num; i++) {
		char p[10];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}
		char *q= new char[1];
		q[1] = '\0';
		if (!tcadbput2(adb, p, q) ) {
			fprintf(stderr, "get error\n");
		}

	}
	tcadbsync(adb);
	/* close the database */
	if (!tcadbclose(adb)) {
		fprintf(stderr, "close error\n");
	}
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_insert2(const char *name) {
	/* open the database */
	TCADB *adb = tcadbnew();
	if (!tcadbopen(adb, name)) {
		fprintf(stderr, "open error\n");
	}
	clock_t t1 = clock();
	string str;
	ifstream inf(inputFile.c_str());
	char *q= new char[1];
	q[1] = '\0';
	while (inf>>str) {
		if (!tcadbput2(adb, str.c_str(), q) ) {
			fprintf(stderr, "get error\n");
		}
	}
	tcadbsync(adb);
	/* close the database */
	if (!tcadbclose(adb)) {
		fprintf(stderr, "close error\n");
	}
	/* delete the object */
	tcadbdel(adb);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_insert3(const char *name, bool btree=0) {
	if (btree) {
		TCBDB *adb = tcbdbnew();
		tcbdbsetcmpfunc(adb, compare, NULL);
		tcbdbsetcache(adb, 1000000, 1000000);
		if (!tcbdbopen(adb, name,15)) {
			fprintf(stderr, "open error\n");
		}
		//	tcbdbsetcmpfunc(adb->bdb, compare, NULL);
		clock_t t1 = clock();

		for (int i=0; i<num; i++) {
			char p[10];
			if (!rnd) {
				sprintf(p, "%08d", i);
			} else {
				sprintf(p, "%08d", myrand()% num+1);
			}

			MyKeyType<string> key;
			key.key = p;
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);		

			if (!tcbdbput(adb, ptr->getData(), ptr->getSize(), NULL, 0) ) {
				fprintf(stderr, "get error\n");
			}

		}
		tcbdbsync(adb);
		/* close the database */
		if (!tcbdbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

	} else {
		TCADB *adb = tcadbnew();
		/* open the database */
		if (!tcadbopen(adb, name)) {
			fprintf(stderr, "open error\n");
		}
		//	tcbdbsetcmpfunc(adb->bdb, compare, NULL);
		clock_t t1 = clock();

		for (int i=0; i<num; i++) {
			char p[10];
			if (!rnd) {
				sprintf(p, "%08d", i);
			} else {
				sprintf(p, "%08d", myrand()% num+1);
			}

			MyKeyType<string> key;			
			key.key = string(p);
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);
			

			if (!tcadbput(adb, ptr->getData(), ptr->getSize(), NULL, 0) ) {
				fprintf(stderr, "get error\n");
			}

		}
		tcadbsync(adb);
		/* close the database */
		if (!tcadbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	}

}

void test_insert4(const char *name, bool btree=0) {
	if (btree) {
		TCBDB *adb = tcbdbnew();
		tcbdbsetcache(adb, 1000000, 1000000);
		tcbdbsetcmpfunc(adb, compare1, NULL);
		if (!tcbdbopen(adb, name, 15)) {
			fprintf(stderr, "open error\n");
		}
		clock_t t1 = clock();

		for (int i=0; i<num; i++) {
			int p;
			if (!rnd) {
				p = i;
			} else {
				p = myrand()% num+1;
			}

			MyKeyType<int> key;
			key.key = p;
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			if (!tcbdbput(adb, ptr->getData(), ptr->getSize(), NULL, 0) ) {
				fprintf(stderr, "get error\n");
			}

		}
		tcbdbsync(adb);
		/* close the database */
		if (!tcbdbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

	} else {
		TCADB *adb = tcadbnew();
		/* open the database */
		if (!tcadbopen(adb, name)) {
			fprintf(stderr, "open error\n");
		}
		clock_t t1 = clock();

		for (int i=0; i<num; i++) {
			int p;
			if (!rnd) {
				p = i;
			} else {
				p = myrand()% num+1;
			}

			MyKeyType<int> key;
			key.key = p;
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			if (!tcadbput(adb, ptr->getData(), ptr->getSize(), NULL, 0) ) {
				fprintf(stderr, "get error\n");
			}

		}
		tcadbsync(adb);
		/* close the database */
		if (!tcadbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	}
	//	tcbdbsetcmpfunc(adb->bdb, compare1, NULL);

}

void test_get1(const char *name) {
	TCADB *adb = tcadbnew();
	/* open the database */
	if (!tcadbopen(adb, name)) {
		fprintf(stderr, "open error\n");
	}
	clock_t t1 = clock();
	for (int i=0; i<num; i++) {
		char p[10];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}
		char* value = 0;
		value = tcadbget2(adb, p);
		if (trace && value) {
			printf("%s\n", value);
			free(value);
		}
	}
	tcadbsync(adb);
	/* close the database */
	if (!tcadbclose(adb)) {
		fprintf(stderr, "close error\n");
	}
	/* delete the object */
	tcadbdel(adb);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_get2(const char *name) {
	TCADB *adb = tcadbnew();

	/* open the database */
	if (!tcadbopen(adb, name)) {
		fprintf(stderr, "open error\n");
	}
	clock_t t1 = clock();
	string str;
	ifstream inf(inputFile.c_str());
	while (inf>>str) {
		char* value = 0;
		value = tcadbget2(adb, str.c_str());

		if (trace && value) {
			printf("%s\n", value);
			free(value);
		}

	}
	tcadbsync(adb);
	/* close the database */
	if (!tcadbclose(adb)) {
		fprintf(stderr, "close error\n");
	}
	/* delete the object */
	tcadbdel(adb);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_get3(const char *name, bool btree=0) {
	if (btree) {
		TCBDB *adb = tcbdbnew();
		tcbdbsetcmpfunc(adb, compare, NULL);
		tcbdbsetcache(adb, 1000000, 1000000);
		if (!tcbdbopen(adb, name,15)) {
			fprintf(stderr, "open error\n");
			exit(1);
		}
		//tcbdbsetcmpfunc(adb->bdb, compare, NULL);
		clock_t t1 = clock();
		for (int i=0; i<num; i++) {
			char p[10];
			if (!rnd) {
				sprintf(p, "%08d", i);
			} else {
				sprintf(p, "%08d", myrand()% num+1);
			}
			void* value;

			MyKeyType<string> key;
			key.key = p;
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			int sp;
			value = tcbdbget(adb, (void*)(ptr->getData()), ptr->getSize(), &sp);
			if (trace && value) {
				//printf("%s\n", value);
				free(value);
			}
		}
		tcbdbsync(adb);
		/* close the database */
		if (!tcbdbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		/* delete the object */
		tcbdbdel(adb);
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

	} else {
		TCADB *adb = tcadbnew();
		/* open the database */
		if (!tcadbopen(adb, name)) {
			fprintf(stderr, "open error\n");			
		}
		//tcbdbsetcmpfunc(adb->bdb, compare, NULL);
		clock_t t1 = clock();
		for (int i=0; i<num; i++) {
			char p[10];
			if (!rnd) {
				sprintf(p, "%08d", i);
			} else {
				sprintf(p, "%08d", myrand()% num+1);
			}
			void* value;

			MyKeyType<string> key;
			key.key = p;
			key.data = i;

			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			int sp;
			value = tcadbget(adb, (void*)(ptr->getData()), ptr->getSize(), &sp);
			if (trace && value) {
				//printf("%s\n", value);
				free(value);
			}
		}
		tcadbsync(adb);
		/* close the database */
		if (!tcadbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		/* delete the object */
		tcadbdel(adb);
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	}

}

void test_get4(const char *name, bool btree=0) {
	if (btree) {
		TCBDB *adb = tcbdbnew();
		tcbdbsetcache(adb, 1000000, 1000000);
		tcbdbsetcmpfunc(adb, compare1, NULL);
		if (!tcbdbopen(adb, name, 15)) {
			fprintf(stderr, "open error\n");
			exit(1);
		}
		//tcbdbsetcmpfunc(adb->bdb, compare1, NULL);
		clock_t t1 = clock();
		for (int i=0; i<num; i++) {
			int p;
			void* value = 0;
			if (!rnd) {
				p = i;
			} else {
				p = myrand()% num+1;
			}

			MyKeyType<int> key;
			key.key = p;
			key.data = i;
			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			int sp;
			value = tcbdbget(adb, (void*)(ptr->getData()), ptr->getSize(), &sp);
			if (trace && value) {
				//printf("%s\n", value);
				free(value);
			}
		}
		tcbdbsync(adb);
		/* close the database */
		if (!tcbdbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		/* delete the object */
		tcbdbdel(adb);
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

	} else {
		TCADB *adb = tcadbnew();
		/* open the database */
		if (!tcadbopen(adb, name)) {
			fprintf(stderr, "open error\n");
		}
		//tcbdbsetcmpfunc(adb->bdb, compare1, NULL);
		clock_t t1 = clock();
		for (int i=0; i<num; i++) {
			int p;
			void* value = 0;
			if (!rnd) {
				p = i;
			} else {
				p = myrand()% num+1;
			}

			MyKeyType<int> key;
			key.key = p;
			key.data = i;
			DbObjPtr ptr;
			ptr.reset(new DbObj);
			write_image(key, ptr);

			int sp;
			value = tcadbget(adb, (void*)(ptr->getData()), ptr->getSize(), &sp);
			if (trace && value) {
				//printf("%s\n", value);
				free(value);
			}
		}
		tcadbsync(adb);
		/* close the database */
		if (!tcadbclose(adb)) {
			fprintf(stderr, "close error\n");
		}
		/* delete the object */
		tcadbdel(adb);
		printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	}

}

/*
 void test_traverse(const char *name) {
 TCADB *adb = tcadbnew();
 // open the database
 if (!tcadbopen(adb, name)) {
 fprintf(stderr, "open error\n");
 }
 // traverse records 
 tcadbiterinit(adb);
 char* key, value;
 while ((key = tcadbiternext2(adb)) != NULL) {
 value = tcadbget2(adb, key);
 if (trace && value) {
 printf("%s:%s\n", key, value);
 free(value);
 }
 free(key);
 }
 }

 void test_traverse(const char *name) {
 TCADB *adb = tcadbnew();
 // open the database 
 if (!tcadbopen(adb, name)) {
 fprintf(stderr, "open error\n");
 }
 // traverse records 
 tcadbiterinit(adb);
 char* key, value;
 while ((key = tcadbiternext2(adb)) != NULL) {
 value = tcadbget2(adb, key);
 if (trace && value) {
 printf("%s:%s\n", key, value);
 free(value);
 }
 free(key);
 }
 }
 */

void ReportUsage(void) {

	cout
			<<"\nUSAGE:./t_tc [-T <trace_option>] [-rnd <true|false>]  [-n <num>]\n\n";

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
				rnd = bool(atoi(*argv++));
			} else if (str == "degree") {
				degree = atoi(*argv++);
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}
	clock_t t1 = clock();

	cout<<"1.tch"<<endl;
	//test_insert1("1.tch#rcnum=1000000");
	//test_get1("1.tch#rcnum=1000000");
	
	test_insert1("1.tch");
	test_get1("1.tch");
	//test_traversal("1.tch#rcnum=1000000");

	cout<<"2.tch"<<endl;
	test_insert2("2.tch");
	test_get2("2.tch");


	cout<<"3.tch"<<endl;
	test_insert3("3.tch");
	test_get3("3.tch");
	//test_traversal("3.tch#rcnum=1000000");	

	cout<<"4.tch"<<endl;
	test_insert4("4.tch");
	test_get4("4.tch");
	
	cout<<"1.tcb"<<endl;
	test_insert1("1.tcb");
	test_get1("1.tcb");
	
	//test_insert1("1.tcb#lcnum=1000000#ncnum=1000000");
	//test_get1("1.tcb#lcnum=1000000#ncnum=1000000");
	//test_traversal("1.tcb#lcnum=1000000#ncnum=1000000");

	cout<<"2.tcb"<<endl;
	test_insert2("2.tcb");
	test_get2("2.tcb");
	//test_insert2("2.tcb#lcnum=1000000#ncnum=1000000");
	//test_get2("2.tcb#lcnum=1000000#ncnum=1000000");
	//test_traversal("2.tcb");

	cout<<"3.tcb"<<endl;
	test_insert3("3.tcb#lcnum=1000000#ncnum=1000000", 1);
	test_get3("3.tcb#lcnum=1000000#ncnum=1000000", 1);
	//test_traversal("3.tcb");

	cout<<"4.tcb"<<endl;
	test_insert4("4.tcb#lcnum=1000000#ncnum=1000000", 1);
	test_get4("4.tcb#lcnum=1000000#ncnum=1000000", 1);

	printf("%lf", double (clock() - t1)/CLOCKS_PER_SEC);

}

