#include <sdb/SequentialDB.h>

using namespace std;

//#include "db.h"
#include "db_cxx.h"

using namespace std;

static int num = 1000000;
static bool trace = 0;
//static int degree;
static string inputFile;
static bool rnd = 0;
static string typeStr = "btree";

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
		memcpy(&dat, ptr->getData(),ptr->getSize());
	}

	template<> inline void write_image<MyKeyType<int> >(const MyKeyType<int>& dat, DbObjPtr& ptr) {
		ptr->setData(&dat, sizeof(MyKeyType<int>));
	}
}

NS_IZENELIB_AM_END

int compare(DB* dbp, const DBT* a, const DBT* b) {

	DbObjPtr ptr, ptr1;
	MyKeyType<string> key, key1;
	ptr.reset(new DbObj(a->data,a->size));
	ptr1.reset(new DbObj(a->data, b->size));
	read_image(key, ptr);
	read_image(key1, ptr1);
	return key.compare(key1);
}

int compare1(DB* dbp, const DBT* a, const DBT* b) {
	DbObjPtr ptr, ptr1;
	MyKeyType<int> key, key1;
	ptr.reset(new DbObj(a->data,a->size));
	ptr1.reset(new DbObj(a->data, b->size));
	read_image(key, ptr);
	read_image(key1, ptr1);
	return key.compare(key1);

}

/* pseudo random number generator */
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

DB* createDB(const char* name, int option=0) {

	DB *dbp;
	int ret;
	/* Create the database handle and open the underlying database. */

	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		exit(1);
	}
	dbp->set_cachesize(dbp, 4, 1024*1024*128, 1);
	if (typeStr.compare("btree") == 0) {
		//dbp->set_bt_minkey(dbp, degree);
		if (option == 1) {
			dbp->set_bt_compare(dbp, compare);
		}
		if (option == 2) {
			dbp->set_bt_compare(dbp, compare1);
		}
		if ((ret = dbp->open(dbp, NULL, name, NULL, DB_BTREE, DB_CREATE, 0664))
				!= 0) {
			dbp->err(dbp, ret, "%s", name);
			//  goto err;
		}

	} else {
		if ((ret = dbp->open(dbp, NULL, name, NULL, DB_HASH, DB_CREATE, 0664))
				!= 0) {
			dbp->err(dbp, ret, "%s", name);
			//  goto err;
		}
	}
	return dbp;

}

void test_insert1(const char *name) {
	DB* dbp = createDB(name);
	int ret;
	clock_t t1 = clock();
	for (int i =0; i<num; i++) {
		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
		char p[8];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}

		key.data = p;
		key.size = sizeof(p);
		//data.data = p;
		//data.size = sizeof(p);

		/* Store a key/data pair. */
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace) {
				printf("db: %s: key stored.\n", (char *)key.data);
			}
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->put");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_insert2(const char *name) {
	DB* dbp = createDB(name);
	int ret;

	clock_t t1 = clock();
	string ystr;
	ifstream inf(inputFile.c_str());
	while (inf>>ystr) {
		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		key.data = (void*)ystr.c_str();
		key.size = sizeof(ystr);

		/* Store a key/data pair. */
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace) {
				printf("db: %s: key stored.\n", (char *)key.data);
			}
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->put");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_insert3(const char *name, int op=1) {

	DB* dbp = createDB(name, op);
	int ret;
	clock_t t1 = clock();
	for (int i =0; i<num; i++) {
		char p[8];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}

		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		string str = p;
		MyKeyType<string> key1 = { str, i };
		DbObjPtr ptr = new DbObj;
		write_image(key1, ptr);
		char* temp = new char[ptr->getSize()];
		memcpy(temp, ptr->getData(), ptr->getSize());
		key.data = temp;
		key.size = ptr->getSize();

		/* Store a key/data pair. */
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace) {
				printf("db: %s: key stored.\n", (char *)key.data);
			}
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->put");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

void test_insert4(const char *name, int op=2) {

	DB* dbp = createDB(name, op);
	int ret;
	clock_t t1 = clock();
	for (int i =0; i<num; i++) {
		int p;
		if (!rnd) {
			p =i;
		} else {
			p = myrand()% num+1;
		}

		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		MyKeyType<int> key1 = { p, p };
		DbObjPtr ptr = new DbObj;
		write_image(key1, ptr);
		char* temp = new char[ptr->getSize()];
		memcpy(temp, ptr->getData(), ptr->getSize());
		key.data = temp;
		key.size = ptr->getSize();

		/* Store a key/data pair. */
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace) {
				printf("db: %s: key stored.\n", (char *)key.data);
			}
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->put");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_get1(const char *name) {
	DB* dbp = createDB(name);
	int ret;
	clock_t t1 = clock();
	for (int i =0; i<num; i++) {
		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
		char p[8];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}

		key.data = p;
		key.size = sizeof(p);
		//data.data = p;
		//data.size = sizeof(p);

		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace)
				printf("db: %s: key retrieved: data was %s.\n",
						(char *)key.data, (char *)data.data);
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->get");//exit(1);
			}
			// goto err;
		}
	}
	printf("befor close eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	dbp->close(dbp, 0);
	//printf("eclipse: %ld seconds\n", time(0)- start);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_get2(const char *name) {
	DB* dbp = createDB(name);
	int ret;
	clock_t t1 = clock();
	ifstream inf(inputFile.c_str());
	string ystr;
	while (inf>>ystr) {
		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		key.data = (void*)ystr.c_str();
		key.size = sizeof(ystr);

		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace)
				printf("db: %s: key retrieved: data was %s.\n",
						(char *)key.data, (char *)data.data);
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->get");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void test_get3(const char *name, int op=1) { //	tcbdbsetcmpfunc(adb->bdb, compare, NULL);

	DB* dbp = createDB(name, op);

	clock_t t1 = clock();
	int ret=0;
	for (int i =0; i<num; i++) {
		char p[8];
		if (!rnd) {
			sprintf(p, "%08d", i);
		} else {
			sprintf(p, "%08d", myrand()% num+1);
		}

		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		string str = p;
		MyKeyType<string> key1 = { str, i };
		DbObjPtr ptr = new DbObj;
		write_image(key1, ptr);
		char* temp = new char[ptr->getSize()];
		memcpy(temp, ptr->getData(), ptr->getSize());
		key.data = temp;
		key.size = ptr->getSize();

		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace)
				printf("db: %s: key retrieved: data was %s.\n",
						(char *)key.data, (char *)data.data);
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->get");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

void test_get4(const char *name, int op=2) {

	DB* dbp = createDB(name, op);
	int ret;

	clock_t t1 = clock();
	for (int i =0; i<num; i++) {
		int p;
		if (!rnd) {
			p =i;
		} else {
			p = myrand()% num+1;
		}

		DBT key, data;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		MyKeyType<int> key1 = { p, p };
		DbObjPtr ptr = new DbObj;
		write_image(key1, ptr);
		char* temp = new char[ptr->getSize()];
		memcpy(temp, ptr->getData(), ptr->getSize());
		key.data = temp;
		key.size = ptr->getSize();

		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
			if (trace)
				printf("db: %s: key retrieved: data was %s.\n",
						(char *)key.data, (char *)data.data);
		} else {
			if (trace) {
				dbp->err(dbp, ret, "DB->get");//exit(1);
			}
			// goto err;
		}
	}
	dbp->close(dbp, 0);
	printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void ReportUsage(void) {

	cout
			<<"\nUSAGE:./t_tc [-T <trace_option>] [-rnd <true|false>] [-db [hash|btree]] [-n <num>]\n\n";

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
			} else if (str == "db") {
				typeStr = *argv++;
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

	if (typeStr.compare("hash") == 0) {

		cout<<"1.bdbh"<<endl;
		test_insert1("1.bdbh");
		test_get1("1.bdbh");

		cout<<"2.bdbh"<<endl;
		test_insert2("2.bdbh");
		test_get2("2.bdbh");

		cout<<"3.bdbh"<<endl;
		test_insert3("3.bdbh");
		test_get3("3.bdbh");

		cout<<"4.bdbh"<<endl;
		test_insert4("4.bdbh");
		test_get4("4.bdbh");

	} else {
		cout<<"1.bdbb"<<endl;
		test_insert1("1.bdbb");
		test_get1("1.bdbb");

		cout<<"2.bdbb"<<endl;
		test_insert2("2.bdbb");
		test_get2("2.bdbb");

		cout<<"3.bdbb"<<endl;
		test_insert3("3.bdbb");
		test_get3("3.bdbb");

		cout<<"4.bdbb"<<endl;
		test_insert4("4.bdbb");
		test_get4("4.bdbb");

	}

	printf("%lf", double (clock() - t1)/CLOCKS_PER_SEC);

}

