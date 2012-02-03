#include"LFVector.h"
#include<boost/thread/thread.hpp>
#include<iostream>
using namespace std;

lfv::LFVector<int> v1;
   volatile int i = 0;
void push(){
    for(int m = 0; m < 100000; ++m){
        v1.push_back(i + m);
        ++i;
    //    cout<<"Push back: "<<i<<" ,Size: "<<v1.size()<<endl;
       // boost::thread::sleep(boost::get_system_time()+boost::posix_time::milliseconds(10));
    }
}

volatile int j = 0;
void pop(){
    for(int m = 0; m < 100000; ++m){
        v1.pop_back();
        ++j;
    //    cout<<"Pop back. Size: "<<v1.size()<<endl;
      //  boost::thread::sleep(boost::get_system_time()+boost::posix_time::milliseconds(20));
    }
}
#ifdef CONFIG_SMP
    #define LOCK_PREFIX "lock ; "
    #else
    #define LOCK_PREFIX ""
    #endif

#ifdef __x86_64__
    static __inline__ uint64_t atomic_compare_exchange(volatile uint64_t * pv,
        const uint64_t nv, const uint64_t cv)
    {
      register unsigned long __res;
      __asm__ __volatile__ (
          LOCK_PREFIX "cmpxchgq %3,(%1)"
          : "=a" (__res), "=q" (pv) : "1" (pv), "q" (nv), "0" (cv));
      return __res;
    }
#endif
int main(int argc, char* argv[]){
//    cout<<"size: "<<v1.size()<<endl;
//    for(int i = 0; i < 10; ++i){
//        if(i % 3 == 0)
//            v1.pop_back();
//        else
//            v1.push_back(i);
//   }
//   int s = v1.size();
//   cout<<"size: "<<s<<endl;
//   for(int j = 0; j < s; ++j){
//       cout<<j<<": "<<v1.at(j)<<endl;
//   }

    boost::thread thrd(&push);
    boost::thread thrd2(&pop);
    thrd.join();
    thrd2.join();
    cout<<v1.size()<<", "<<i<<", "<<j<<endl;
    cout<<v1.at(12322)<<endl;

}



