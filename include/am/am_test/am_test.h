#ifndef AM_TEST_H_
#define AM_TEST_H_

#include <util/ProcMemInfo.h>
#include <util/hashFunction.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <am/am.h>
#include <util/ClockTimer.h>

using namespace std;
using namespace __gnu_cxx;
using namespace boost;
//using namespace izenelib::util;
using namespace izenelib::am;

#ifdef INNER_TRACE
#undef INNER_TRACE
#define INNER_TRACE 0
#endif

#ifndef INNER_TRACE
#define INNER_TRACE 0
#endif

namespace izenelib {
namespace am_test {

// pseudo random number generator
inline int myrand(void) {
	static int cnt = 0;
	return (lrand48() + cnt++) & 0x7FFFFFFF;
}

template<typename Type> inline
Type generateData(const int a, int num=1000000, bool rand=false) {
	return Type();
}

template<> inline string generateData<string>(const int a, int num, bool rand) {
	if (num < 1000000)
		num = 1000000;
	char p[10];
	int b;
	if (rand)
		b = myrand()%(num+1);
	else
		b = a;
	sprintf(p, "%08d", b);
	return string(p);
}

template<> inline int generateData<int>(const int a, int num, bool rand) {
	if (num < 1000000)
		num = 1000000;
	if (rand)
		return myrand()%(num+1);
	else
		return a;
}

template<> inline float generateData<float>(const int a, int num, bool rand) {
	if (num < 1000000)
		num = 1000000;
	if (rand)
		return float(myrand())/(num+1);
	else
		return a;
}

template<> inline vector<int> generateData<vector<int> >(const int a, int num,
		bool rand) {
	vector<int> vret;
	int max;
	if (rand) {
		max = myrand()%( 100 );
	} else {
		max = num %100;
	}
	for (int i=0; i<max; i++) {
		vret.push_back(i);
	}
	return vret;
}

void displayMemInfo(std::ostream& os = std::cout) {
	unsigned long rlimit = 0, vm = 0, rss = 0;
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	os << "vm: " << vm << "bytes rss: " << rss << "bytes" << endl;
}

template<typename KeyType, typename ValueType, typename AM, bool open=false> class AmTest {
	bool rand_;
	int num_;
	int loop_;
	bool trace_;
	AMOBJ<KeyType, ValueType, AM, open>* am_;
	izenelib::util::ClockTimer timer;	
public:
	AmTest() :
		rand_(true), num_(1000000), loop_(1), trace_(false) {
		if ( !open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
	}
	~AmTest() {
		if ( !open) {
			if (am_) {
				delete am_;
				am_ = 0;
			}
		}
	}

	void setNum(int num) {
		num_ = num;
	}
	void setLoop(int num) {
		loop_ = num;
	}
	void setRandom(bool rand) {
		rand_ = rand;
	}
	void setTrace(bool trace) {
		trace_ = trace;
	}

	void run_insert(bool mem=true) {
		clock_t t1 = clock();
		timer.restart();
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		int hit = 0;
		int sum = 0;
		for (int i =0; i<num_; i++) {
			sum++;
#if INNER_TRACE
			if (trace_) {
				cout<<"Insert key="<<generateData<KeyType>(i, num_, rand_)<<endl;
			}
			am_->display();
#endif
			KeyType key = generateData<KeyType>(i, num_, rand_);
			ValueType value = generateData<ValueType>(i, num_, rand_);
			if (am_->insert(key, value) )
				hit++;

			//            cout<<"idx="<<i<<","<<key<<","<<value<<endl;
			if (i % 1000000 == 0) {
				cout<<"idx="<<i<<endl;
				displayMemInfo();
			}
		}
		if (mem) {
			printf("insert elapsed 0 ( by clock(), cpu ) : %lf seconds\n",
					double(clock()- t1)/CLOCKS_PER_SEC);
			printf("insert elapsed 1 ( actually ): %lf seconds\n",
					timer.elapsed() );
			printf("insert success ratio: %d /%d\n", hit, sum);
			displayMemInfo();
		}
		if (open)
			delete am_;

	}

	//when KeyType is YString, for ylib
	void run_insert_ylib(bool mem=true) {
		clock_t t1 = clock();
		timer.restart();
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		for (int i =0; i<num_; i++) {
			if (trace_) {
#if INNER_TRACE
				cout<<"Insert key="<<generateData<KeyType>(i, num_, rand_)<<endl;
#endif
			}
			am_->insert(generateData<KeyType>(i, num_, rand_) );
		}
		if (mem) {
			printf("insert elapsed: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
			displayMemInfo();
		}
		if (open)
			delete am_;

	}

	void run_find(bool mem=true) {
		clock_t t1 = clock();
		timer.restart();
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		int hit = 0;
		int sum = 0;
		for (int i =0; i<num_; i++) {
			sum++;
#if INNER_TRACE
			if (trace_) {
				cout<<"find key="<<generateData<KeyType>(i, num_, rand_)<<endl;
			}
#endif

			ValueType pv;
			bool ret = am_->get(generateData<KeyType>(i, num_, rand_), pv);
			if (ret) {
				hit++;
			} else {
				//cout<<"Unfound idx="<<i<<endl;
			}
			if (i % 1000000 == 0) {
				cout<<"idx="<<i<<endl;
				displayMemInfo();
			}
		}
		if (mem) {
			printf("find elapsed 0: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
			printf("find elapsed 1: %lf seconds\n", timer.elapsed() );
			printf("find success ratio: %d /%d\n", hit, sum);
			displayMemInfo();
		}
		if (open)
			delete am_;

	}

	void run_del(bool mem=true) {
		clock_t t1 = clock();
		timer.restart();
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		int hit = 0;
		int sum = 0;
		for (int i =0; i<num_; i++) {
			sum++;

#if INNER_TRACE
			if (trace_) {
				cout<<"del key="<<generateData<KeyType>(i, num_, rand_)<<endl;
			}
#endif
			if (am_->del(generateData<KeyType>(i, num_, rand_) ))
				hit++;
		}
		if (mem) {
			printf("del elapsed 0: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
			printf("del elapsed 1: %lf seconds\n", timer.elapsed() );
			printf("del success ratio: %d /%d\n", hit, sum);
			displayMemInfo();
		}
		if (open)
			delete am_;

	}

	void run_seq(bool mem=true) {
		typedef typename AM::SDBCursor SDBCursor;
		
		clock_t t1 = clock();
		timer.restart();
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		int sum = 0;
		SDBCursor locn = am_->get_first_locn();
		KeyType key;
		ValueType val;
		while (am_->get(locn, key, val) ) {
			am_->seq(locn);
			sum++;
		}
		if (mem) {
			printf("seq elapsed 0: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
			printf("seq elapsed 1: %lf seconds\n", timer.elapsed() );
			printf("seq num : %d\n", sum);
			displayMemInfo();
		}
		if (open)
			delete am_;
	}
	void display(std::ostream& os = std::cout) {
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		am_->display(os);
		if (open)
			delete am_;
	}

	int num_items() {
		if (open)
			am_ = new AMOBJ<KeyType, ValueType, AM, open>();
		int num = am_->num_items();
		if (open)
			delete am_;
		return num;
	}

};

template<typename T> void run_am(T& cm) {
	cm.run_insert();
	cout<<"num: "<<cm.num_items()<<endl;
	cm.run_find();
	cout<<"num: "<<cm.num_items()<<endl;
	cm.run_seq();
	cout<<"num: "<<cm.num_items()<<endl;
	cm.run_del();
	cout<<"num: "<<cm.num_items()<<endl;
}

template<typename T> void run_am_nod(T& cm) {
	cm.run_insert();
	cout<<"num: "<<cm.num_items()<<endl;
	cm.run_find();
	cout<<"num: "<<cm.num_items()<<endl;
}

template<typename T> void run_loop(unsigned int loop) {
	clock_t t1 = clock();
	izenelib::util::ClockTimer timer;
	vector< boost::shared_ptr<T> > am_group;
	for (unsigned int i=0; i<loop; i++) {
		boost::shared_ptr<T> am;
		am.reset(new T);
		am->setNum(100);
		am->setRandom(true);
		am_group.push_back(am);
	}
	typename vector< boost::shared_ptr<T> >::iterator it = am_group.begin();
	for (; it != am_group.end(); it++) {
		(*it)->run_insert(false);
	}
	printf("insert elapsed 0: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);
	printf("insert elapsed 1: %lf seconds\n", timer.elapsed() );
	displayMemInfo();
}

}
}
#endif /*AM_TEST_H_*/
