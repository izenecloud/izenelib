/*
 * Copyright 2013 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// @author: Xin Liu <xliux@fb.com>

/*  #include <boost/version.hpp>
#if BOOST_VERSION >= 105300 
#include <memory>
#include <set>
#include <vector>
#include <3rdparty/folly/RWSpinLock.h>
#include <3rdparty/folly/ConcurrentSkipList.h>

#include <boost/thread/thread.hpp>
#include <boost/system/system_error.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <glog/logging.h>
//#include <gflags/gflags.h>
#include <3rdparty/folly/Foreach.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
//#include "folly/String.h"

#define FLAGS_num_threads 4

namespace {
//DEFINE_int32(num_threads, 12, "num concurrent threads to test");



using namespace folly;
using std::vector;

typedef int ValueType;
typedef detail::SkipListNode<ValueType> SkipListNodeType;
typedef ConcurrentSkipList<ValueType> SkipListType;
typedef SkipListType::Accessor SkipListAccessor;
typedef vector<ValueType> VectorType;
typedef std::set<ValueType> SetType;

static const int kHeadHeight = 2;
static const int kMaxValue = 5000;

static void randomAdding(int size,
    SkipListAccessor skipList,
    SetType *verifier,
    int maxValue = kMaxValue) 
{
  for (int i = 0; i < size; ++i) 
  {
    int32_t r = rand() % maxValue;
    verifier->insert(r);
    skipList.add(r);
  }
}

static void randomRemoval(int size,
    SkipListAccessor skipList,
    SetType *verifier,
    int maxValue=kMaxValue) 
{
  for (int i = 0; i < size; ++i) {
    int32_t r = rand() % maxValue;
    verifier->insert(r);
    skipList.remove(r);
  }
}

static void sumAllValues(SkipListAccessor skipList, int64_t *sum)
{
  *sum = 0;
  FOR_EACH(it, skipList)
  {
    *sum += *it;
  }
  VLOG(20) << "sum = " << sum;
}

static void concurrentSkip(const vector<ValueType> *values,
    SkipListAccessor skipList)
{
  int64_t sum = 0;
  SkipListAccessor::Skipper skipper(skipList);
  FOR_EACH(it, *values) {
    if (skipper.to(*it)) sum += *it;
  }
  VLOG(20) << "sum = " << sum;
}

bool verifyEqual(SkipListAccessor skipList,
    const SetType &verifier) {
  BOOST_CHECK_EQUAL(verifier.size(), skipList.size());
  FOR_EACH(it, verifier) {
    CHECK(skipList.contains(*it)) << *it;
    SkipListType::const_iterator iter = skipList.find(*it);
    CHECK(iter != skipList.end());
    BOOST_CHECK_EQUAL(*iter, *it);
  }
  BOOST_CHECK(std::equal(verifier.begin(), verifier.end(), skipList.begin()));
  return true;
}

BOOST_AUTO_TEST_SUITE(concurrent_skip_list)


BOOST_AUTO_TEST_CASE(skip_list)
{
    LOG(INFO) << "nodetype size=" << sizeof(SkipListNodeType);

    BOOST_AUTO( skipList,SkipListType::create(kHeadHeight));
    BOOST_CHECK(skipList.first() == NULL);
    BOOST_CHECK(skipList.last() == NULL);

    skipList.add(3);
    BOOST_CHECK(skipList.contains(3));
    BOOST_CHECK(!(skipList.contains(2)));
    BOOST_CHECK_EQUAL(3, *skipList.first());
    BOOST_CHECK_EQUAL(3, *skipList.last());

    BOOST_CHECK_EQUAL(3, *skipList.find(3));
    BOOST_CHECK(!(skipList.find(3) == skipList.end()));
    BOOST_CHECK(skipList.find(2) == skipList.end());

    {
      SkipListAccessor::Skipper skipper(skipList);
      skipper.to(3);
      CHECK_EQ(3, *skipper);
    }

    skipList.add(2);
    BOOST_CHECK_EQUAL(2, *skipList.first());
    BOOST_CHECK_EQUAL(3, *skipList.last());
    skipList.add(5);
    BOOST_CHECK_EQUAL(5, *skipList.last());
    skipList.add(3);
    BOOST_CHECK_EQUAL(5, *skipList.last());
    //skipList.insert(9);
    BOOST_AUTO( ret , skipList.insert<int>(9));
    BOOST_CHECK_EQUAL(9, *ret.first);
    //LOG(INFO)<<"*ret.first"<<(*ret.first)<<" "<<ret.second<<std::endl;
    BOOST_CHECK(ret.second);

    ret = skipList.insert(5);
    BOOST_CHECK_EQUAL(5, *ret.first);
    BOOST_CHECK(!ret.second);

    BOOST_CHECK_EQUAL(2, *skipList.first());
    BOOST_CHECK_EQUAL(9, *skipList.last());
    BOOST_CHECK(skipList.pop_back());
    BOOST_CHECK_EQUAL(5, *skipList.last());
    BOOST_CHECK(skipList.pop_back());
    BOOST_CHECK_EQUAL(3, *skipList.last());

    skipList.add(9);
    skipList.add(5);

    CHECK(skipList.contains(2));
    CHECK(skipList.contains(3));
    CHECK(skipList.contains(5));
    CHECK(skipList.contains(9));
    CHECK(!skipList.contains(4));

    // lower_bound
    BOOST_AUTO( it , skipList.lower_bound(5));
    BOOST_CHECK_EQUAL(5, *it);
    it = skipList.lower_bound(4);
    BOOST_CHECK_EQUAL(5, *it);
    it = skipList.lower_bound(9);
    BOOST_CHECK_EQUAL(9, *it);
    it = skipList.lower_bound(12);
    BOOST_CHECK(!it.good());

    it = skipList.begin();
    BOOST_CHECK_EQUAL(2, *it);

    // skipper test
    SkipListAccessor::Skipper skipper(skipList);
    skipper.to(3);
    BOOST_CHECK_EQUAL(3, skipper.data());
    skipper.to(5);
    BOOST_CHECK_EQUAL(5, skipper.data());
    CHECK(!skipper.to(7));

    skipList.remove(5);
    skipList.remove(3);
    CHECK(skipper.to(9));
    BOOST_CHECK_EQUAL(9, skipper.data());

    CHECK(!skipList.contains(3));
    skipList.add(3);
    CHECK(skipList.contains(3));
    int pos = 0;
    FOR_EACH(it, skipList) {
      LOG(INFO) << "pos= " << pos++ << " value= " << *it;
    }
  

  {
    BOOST_AUTO(skipList,SkipListType::create(kHeadHeight));

    SetType verifier;
    randomAdding(10000, skipList, &verifier);
    verifyEqual(skipList, verifier);

    // test skipper
    SkipListAccessor::Skipper skipper(skipList);
    int num_skips = 1000;
    for (int i = 0; i < num_skips; ++i) {
      int n = i * kMaxValue / num_skips;
      bool found = skipper.to(n);
      BOOST_CHECK_EQUAL(found, (verifier.find(n) != verifier.end()));
    }
  }

}

static std::string makeRandomeString(int len) {
  std::string s;
  for (int j = 0; j < len; j++) {
    s.push_back((rand() % 26) + 'A');
  }
  return s;
}

BOOST_AUTO_TEST_CASE(ConcurrentSkipListSort) 
{
  typedef folly::ConcurrentSkipList<std::string> SkipListT;
  boost::shared_ptr<SkipListT> skip = SkipListT::createInstance();
  SkipListT::Accessor accessor(skip);
  {
    for (int i = 0; i < 100000; i++) {
      std::string s = makeRandomeString(7);
      accessor.insert(s);
    }
  }
  //BOOST_CHECK(boost::is_sorted(accessor.begin(), accessor.end()));
  BOOST_CHECK(std::adjacent_find(accessor.begin(), accessor.end()) ==accessor.end());
}


template<typename T>
struct Deleter {
    void operator()(T *p) {};
};

struct UniquePtrComp {
  bool operator ()(const boost::interprocess::unique_ptr<int,Deleter<int> > &x, const boost::interprocess::unique_ptr<int,Deleter<int> > &y) const
  {
    if (!x) return false;
    if (!y) return true;
    return *x < *y;
  }
};



/* can't be compiled 
BOOST_AUTO_TEST_CASE(ConcurrentSkipListTestMovableData)
{

  typedef folly::ConcurrentSkipList<boost::interprocess::unique_ptr<int,Deleter<int> >, UniquePtrComp>//
    SkipListT;
  //SkipListT a;

  SkipListT::createInstance();

  BOOST_AUTO( sl , SkipListT::createInstance()) ;
  SkipListT::Accessor accessor(sl);

  static const int N = 10;
  for (int i = 0; i < N; ++i) {
    accessor.insert(boost::interprocess::unique_ptr<int,Deleter<int> >(new int(i)));
  }

  for (int i = 0; i < N; ++i) {
    BOOST_CHECK(accessor.find(boost::interprocess::unique_ptr<int,Deleter<int> >(new int(i))) !=
        accessor.end());
  }
  BOOST_CHECK(accessor.find(boost::interprocess::unique_ptr<int,Deleter<int> >(new int(N))) ==
      accessor.end());

}
*/
/*  
void testConcurrentAdd(int numThreads) {
  BOOST_AUTO(skipList,SkipListType::create(kHeadHeight));

  vector<boost::thread*> threads;
  vector<SetType> verifiers(numThreads);
  try {
    for (int i = 0; i < numThreads; ++i) {
      threads.push_back(new boost::thread(
            &randomAdding, 100, skipList, &verifiers[i], kMaxValue));
    }
  } catch (const boost::system::system_error& e) {
    LOG(WARNING)
      << "Caught " << e.what()
      << ": could only create " << threads.size() << " threads out of "
      << numThreads;
  }
  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i]->join();
  }

  SetType all;
  FOR_EACH(s, verifiers) {
    all.insert(s->begin(), s->end());
  }
  verifyEqual(skipList, all);
}

BOOST_AUTO_TEST_CASE(ConcurrentSkipListTestConcurrentAdd)
{
  for (int numThreads = 10; numThreads < 100; numThreads += 10) {
    testConcurrentAdd(numThreads);
  }
}

void testConcurrentRemoval(int numThreads, int maxValue) 
{
  BOOST_AUTO( skipList , SkipListType::create(kHeadHeight));
  for (int i = 0; i < maxValue; ++i) {
    skipList.add(i);
  }

  vector<boost::thread*> threads;
  vector<SetType > verifiers(numThreads);
  try {
    for (int i = 0; i < numThreads; ++i) {
      threads.push_back(new boost::thread(
            &randomRemoval, 100, skipList, &verifiers[i], maxValue));
    }
  } catch (const boost::system::system_error& e) {
    LOG(WARNING)
      << "Caught " << e.what()
      << ": could only create " << threads.size() << " threads out of "
      << numThreads;
  }
  for(BOOST_AUTO(t, threads.begin());t!=threads.end();t++) 
  {
    (*t)->join();
  }

  SetType all;
  FOR_EACH(s, verifiers) {
    all.insert(s->begin(), s->end());
  }

  BOOST_CHECK_EQUAL(maxValue, all.size() + skipList.size());
  for (int i = 0; i < maxValue; ++i) {
    if (all.find(i) != all.end()) {
      CHECK(!skipList.contains(i)) << i;
    } else {
      CHECK(skipList.contains(i)) << i;
    }
  }
}




BOOST_AUTO_TEST_CASE(ConcurrentSkipListConcurrentRemove) {
  for (int numThreads = 10; numThreads < 1000; numThreads += 100) {
    testConcurrentRemoval(numThreads, 100 * numThreads);
  }
}

static void testConcurrentAccess(
    int numInsertions, int numDeletions, int maxValue) {
  BOOST_AUTO( skipList , SkipListType::create(kHeadHeight));

  vector<SetType> verifiers(FLAGS_num_threads);
  vector<int64_t> sums(FLAGS_num_threads);
  vector<vector<ValueType> > skipValues(FLAGS_num_threads);

  for (int i = 0; i < FLAGS_num_threads; ++i) {
    for (int j = 0; j < numInsertions; ++j) {
      skipValues[i].push_back(rand() % (maxValue + 1));
    }
    std::sort(skipValues[i].begin(), skipValues[i].end());
  }

  vector<boost::thread*> threads;
  for (int i = 0; i < FLAGS_num_threads; ++i) {
    switch (i % 8) {
      case 0:
      case 1:
        threads.push_back(new boost::thread(
              randomAdding, numInsertions, skipList, &verifiers[i], maxValue));
        break;
      case 2:
        threads.push_back(new boost::thread(
              randomRemoval, numDeletions, skipList, &verifiers[i], maxValue));
        break;
      case 3:
        threads.push_back(new boost::thread(
              concurrentSkip, &skipValues[i], skipList));
        break;
      default:
        threads.push_back(new boost::thread(sumAllValues, skipList, &sums[i]));
        break;
    }
  }

  for(BOOST_AUTO(t, threads.begin());t!=threads.end();t++) 
  {
    (*t)->join();
  }
};

BOOST_AUTO_TEST_CASE(ConcurrentSkipListConcurrentAccess) {
  testConcurrentAccess(10000, 100, kMaxValue);
  testConcurrentAccess(100000, 10000, kMaxValue * 10);
  testConcurrentAccess(1000000, 100000, kMaxValue);
}
BOOST_AUTO_TEST_SUITE_END() 

}
#endif
*/

