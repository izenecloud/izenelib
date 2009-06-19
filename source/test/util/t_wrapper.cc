#include <string>
#include <ctime>
#include <am/util/Wrapper.h>
#include <map>

using namespace izenelib::am::util;
using namespace izenelib::util;

static size_t range= 100;
static size_t num = 100;
static bool trace = true;
static string inputFile = "input.txt";

typedef map<int, int>::iterator IT;

#if 0

namespace izenelib {
namespace am {
namespace util {

template<> inline void read_image<map<int, int> >(map<int, int>& dat,
		const DbObjPtr& ptr) {
	int num;
	char* p = (char*)ptr->getData();
	memcpy(&num, p, sizeof(int));
	p += sizeof(int);
	for (int i=0; i<num; i++) {
		pair<int, int> a;
		memcpy(&a, p, sizeof(pair<int, int>));
		p += sizeof(pair<int, int>);
		dat.insert(a);
	}
}

template<> inline void write_image<map<int, int> >(const map<int, int>& dat,
		DbObjPtr& ptr) {
	int sz = sizeof(pair<int, int>)*dat.size() + sizeof(int);
	char *buf = new char[sz];
	char *p = buf;
	int num = dat.size();
	memcpy(p, &num, sizeof(int));
	p += sizeof(int);
	for (map<int, int>::const_iterator it=dat.begin(); it != dat.end(); it++) {
		memcpy(p, &(*it), sizeof(pair<int, int>));
		p += sizeof(pair<int, int>);
	}
	ptr->setData(buf, sz);
	delete buf;
}

template<> inline void read_image<map<string, int> >(map<string, int>& dat,
		const DbObjPtr& ptr) {
	int num;
	char* p = (char*)ptr->getData();
	memcpy(&num, p, sizeof(int));
	p += sizeof(int);
	for (int i=0; i<num; i++) {
		pair<string, int> a;
		int len;
		memcpy(&len, p, sizeof(int));
		p += sizeof(int);
		char *buf = new char[len];
		memcpy(buf, p, len);
		a.first = (string)buf;
		delete buf;
		buf = 0;

		p += len;
		memcpy(&(a.second), p, sizeof(int));
		p += sizeof(int);
		//memcpy(&a, p, sizeof(pair<int, int>));
		dat.insert(a);
	}
}

template<> inline void write_image<map<string, int> >(
		const map<string, int>& dat, DbObjPtr& ptr) {
	int num = dat.size();
	int sz = dat.size()*100;
	char *buf = new char[sz];
	char *p = buf;

	memcpy(p, &num, sizeof(int));
	p += sizeof(int);
	map<string, int>::const_iterator it=dat.begin();
	for (; it != dat.end(); it++) {
		int len = it->first.size()+1;
		memcpy(p, &len, sizeof(len));
		p += sizeof(int);
		memcpy(p, it->first.c_str(), len);
		p += len;
		memcpy(p, &(it->second), sizeof(int));
		p += sizeof(int);
		//memcpy(buf, &(*it), sizeof(pair<int, int>) );
	}
	ptr->setData(buf, sz);
	delete buf;
}

#if 0
template<> inline void read_image<vector<int> >(vector<int>& dat,
		const DbObjPtr& ptr) {
	dat.resize(ptr->getSize()/sizeof(int));
	memcpy(&dat[0], ptr->getData(), ptr->getSize());
}

template<> inline void write_image<vector<int> >(const vector<int>& dat,
		DbObjPtr& ptr) {
	ptr->setData(&dat[0], sizeof(int)*dat.size() );

}
#endif

#if 0
template<> inline void read_image<vector<int> >(vector<int>& dat,
		const DbObjPtr& ptr) {
	stringstream istr((char*)ptr->getData());
	{
		boost::archive::text_iarchive ia(istr);
		ia & dat;
	}
	//make sure oa is deconstructed to avoid stream_error exception.

}

template<> inline void write_image<vector<int> >(const vector<int>& dat,
		DbObjPtr& ptr) {
	stringstream ostr;
	{
		boost::archive::text_oarchive oa(ostr);
		oa & dat;
	}

	//stringstream ostr1;
	//{
	//	boost::archive::binary_oarchive oa(ostr1);
	//	oa & dat;
	//}
	//cout<<"text: "<<ostr.str().size()<<endl;
	//cout<<"binary: "<<ostr1.str().size()<<endl;
	//make sure oa is deconstructed to avoid stream_error exception.
	ptr->setData(ostr.str().c_str(), ostr.str().size());
}

#endif

}
}
}

#endif

void wrapper_test1() {
	clock_t t1 = clock();

	int cnt = num;
	while (cnt--) {
		map<int, int> a;
		int t = random()%range;
		for (int i=0; i<t; i++) {
			a[i] = i;
		}
		DbObjPtr ptr(new DbObj);
		write_image(a, ptr);
		map<int, int> b;
		read_image(b, ptr);
		if (trace) {
			for (IT it = b.begin(); it != b.end(); it++) {
				cout<<it->first<<" : "<<it->second<<endl;
			}
		}
	}

	printf("1 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

void wrapper_test2() {
	clock_t t1 = clock();

	int cnt = num;
	while (cnt--) {
		vector<int> a;
		int t = random()%range;
		for (int i=0; i<t; i++) {
			a.push_back(i);
		}
		DbObjPtr ptr(new DbObj);
		write_image(a, ptr);
		vector<int> b;
		read_image(b, ptr);
		if (trace) {
			for (size_t i=0; i<b.size(); i++)
				cout<<b[i]<<endl;
		}
	}

	printf("2 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

void wrapper_test3() {
	clock_t t1 = clock();

	int cnt = num;
	while (cnt--) {
		map<string, int> a;
		int t = random()%range;
		for (int i=0; i<t; i++) {
			char p[10];
			sprintf(p, "%d", i);
			string str = p;
			a[p] = i;
		}
		DbObjPtr ptr(new DbObj);
		write_image(a, ptr);
		map<string, int> b;
		read_image(b, ptr);
		if (trace) {
			map<string, int>::iterator it = b.begin();
			for (; it != b.end(); it++) {
				cout<<it->first<<" : "<<it->second<<endl;
			}
		}
	}

	printf("3 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

}

void wrapper_test4() {
	clock_t t1 = clock();

	int cnt = num;
	while (cnt--) {
		string str;
		int t = random()%range;
		char p[10];
		sprintf(p, "%d", t);
		str = p;

		DbObjPtr ptr(new DbObj);
		write_image(str, ptr);
		string b;
		read_image(b, ptr);
		if (trace) {
			cout<<b<<endl;
		}
	}
	printf("4 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void wrapper_test5() {
	clock_t t1 = clock();

	int cnt = num;
	while (cnt--) {
		string str;
		int t = random()%range;

		DbObjPtr ptr(new DbObj);
		write_image(t, ptr);
		int b;
		read_image(b, ptr);
		if (trace) {
			cout<<b<<endl;
		}
	}
	printf("5 elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
}

void run() {
	wrapper_test1();
	wrapper_test2();
	wrapper_test3();
	wrapper_test4();
	wrapper_test5();
}

void ReportUsage(void) {
	cout
			<<"\nUSAGE: ./t_wrapper [-T <trace_option>] [-n <num>] [-size <sz>] \n\n";

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
			} else if (str == "size") {
				range = atoi(*argv++);
			} else {
				cout<<"Input parameters error\n";
				return 0;
			}
		} else {
			inputFile = str;
			break;
		}
	}
	//try
	{
		run();

	}
	/*catch(bad_alloc)
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
	 }*/
}
