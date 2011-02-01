#include <febird/io/DataIO.h>
#include <febird/io/DataInput.h>
#include <febird/io/DataOutput.h>
//#include <febird/io/MemStream.h>
#include <febird/io/StreamBuffer.h>
#include <febird/io/FileStream.h>
#include <febird/io/IStreamWrapper.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

using namespace std;
using namespace febird;


struct POD
{
	int a, b, c, d;
	DATA_IO_LOAD_SAVE(POD, &a&b&c&d)
    
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & a & b & c & d;
    }
};


struct CMPLX
{
  int a;
  float b;
  vector<int> c;

  DATA_IO_LOAD_SAVE(CMPLX, &a&b&c)
    
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & a & b & c;      
    }
};

template<class T>
void getRandomData(T& t)
{
}

template<>
void getRandomData(int& t)
{
  t = rand();
}

template<>
void getRandomData(double& t)
{
  t = (float)rand();
  t = (float)rand()/t;
  
}

template<>
void getRandomData(string& t)
{  
  t = "";
  for (size_t i=0; i< 25; i++)
  {
    char a = 'a'+rand()%26;
    t += a;
  }  
}

template<>
void getRandomData(POD& t)
{
  t = POD();
}


template<class T>
void getRandomVector(vector<T>& v, size_t sz);

template<>
void getRandomData(CMPLX& t)
{
  t = CMPLX();
  getRandomVector(t.c, rand()%10);
}

template<class T>
void getRandomVector(vector<T>& v, size_t sz)
{
  for (size_t i=0; i< sz; i++)
  {
    T t;
    getRandomData(t);
    v.push_back(t);
  }
  
}

template<class T>
void boost_Serialization(const vector<T>& testData, boost::archive::binary_oarchive& out, clock_t& time)
{
  time = 0;
  clock_t start, finish;
  start = clock();
  out<<testData;
  finish = clock();
  time = finish - start;
}

template<class T>
void boost_Deserialization(vector<T>& testData, boost::archive::binary_iarchive& in, clock_t& time)
{
  time = 0;
  clock_t start, finish;
  start = clock();
  in >> testData;
  finish = clock();
  time = finish - start;
}

template<class T, class OutAr>
void febird_Serialization(const vector<T>& testData, OutAr& out, clock_t& time)
{
  time = 0;
  clock_t start, finish;
  start = clock();
  out & testData;
  finish = clock();
  time = finish - start;
}

template<class T, class InAr>
void febird_Deserialization(vector<T>& testData, InAr& in, clock_t& time)
{
  time = 0;
  clock_t start, finish;
  start = clock();
  in & testData;
  finish = clock();
  time = finish - start;
}

template<class T, class StreamType>
void boost_test(size_t loop, size_t vctr_sz, StreamType& ss, T*)
{
  boost::archive::binary_oarchive oa(ss);
  clock_t time = 0;

  for (size_t i=0; i<loop; i++)
  {
    vector<T> v;
    getRandomVector(v, vctr_sz);
    clock_t t;
    boost_Serialization(v, oa, t);
    time += t;
  }

  cout <<"Serialization: "<< (float)time/CLOCKS_PER_SEC<<" s\n";
  
  boost::archive::binary_iarchive ia(ss);
  time = 0;

  for (size_t i=0; i<loop; i++)
  {
    vector<T> v;
    clock_t t;
    boost_Deserialization(v, ia, t);
    time += t;
  }

  cout <<"Deserialization: "<< (float)time/CLOCKS_PER_SEC<<" s\n";
}


template<class T, class InAr, class OutAr>
void febird_test(size_t loop, size_t vctr_sz, InAr& ia, OutAr& oa, T*)
{
  clock_t time = 0;

  for (size_t i=0; i<loop; i++)
  {
    vector<T> v;
    getRandomVector(v, vctr_sz);
    clock_t t;
    febird_Serialization(v, oa, t);
    time += t;
  }
  
  cout <<"Serialization: "<< (float)time/CLOCKS_PER_SEC<<" s\n";
  
  ia.clone(oa);
  time = 0;  
  for (size_t i=0; i<loop; i++)
  {
    vector<T> v;
    clock_t t;
    febird_Deserialization(v, ia, t);
    time += t;
  }

  cout <<"Deserialization: "<< (float)time/CLOCKS_PER_SEC<<" s\n";
}

template<class T>
void memcopy_test(size_t loop, size_t vctr_sz, T*)
{
  clock_t ser_time = 0;
  clock_t deser_time = 0;
  
  for (size_t i=0; i<loop; i++)
  {
    vector<T> v;
    getRandomVector(v, vctr_sz);
    
    clock_t start, finish;
    
    start = clock();
    char* buf = new char[v.size()*sizeof(T)];
    memcpy(buf, (char*) &v[0], v.size()*sizeof(T));
    finish = clock();

    ser_time += finish - start;

    vector<T> v2;
    start = clock();
    v2.resize(v.size());
    memcpy((char*) &v2[0], buf, v.size()*sizeof(T));
    finish = clock();

    deser_time += finish - start;

    if (v2.size()!=v.size())
      cout<<"memcpy error!\n";
    
    delete buf;
  }
  
  cout <<"Serialization: "<< (float)ser_time/CLOCKS_PER_SEC<<" s\n";
  
  cout <<"Deserialization: "<< (float)deser_time/CLOCKS_PER_SEC<<" s\n";
}

template<class T>
void compare(size_t loop, size_t vctr_sz, T*)
{
  cout<<"boost:\n";
  stringstream ss;
  boost_test(loop, vctr_sz, ss, (T*)0);

  cout<<"\nfebird AutoGrownMemIO:\n";

  NativeDataOutput<AutoGrownMemIO> oa;
  NativeDataInput<AutoGrownMemIO> ia;

  febird_test(loop, vctr_sz, ia, oa, (T*)0);

//  memcpy test
//  cout<<"\nmemcpy:\n";
//  char* buf = new char[loop*vctr_sz*sizeof(T)*2];
//  memcopy_test(loop, vctr_sz, (T*)0);
//  delete buf;
}

int main(int argc, char* argv[])
{

  const size_t loop = 100000;
  const size_t vctr_sz = 100;
  
  cout<<"loop: "<<loop<<"  vector size: "<<vctr_sz<<endl;
  
  cout<<"\n==============vector<int>=============="<<endl;
  compare(loop, vctr_sz, (int*)0);

  cout<<"\n==============vector<float>=============="<<endl;
  compare(loop, vctr_sz, (float*)0);

  cout<<"\n==============vector<POD>=============="<<endl;
  compare(loop, vctr_sz, (POD*)0);

  cout<<"\n==============vector<CMPLX>=============="<<endl;
  compare(loop, vctr_sz, (CMPLX*)0);
  

}

