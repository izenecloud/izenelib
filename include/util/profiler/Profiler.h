#ifndef __PROFILER__
#define __PROFILER__

#include "TimeChecker.h"

#include <pthread.h>

#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>

//#define ATOMIC 

#ifdef ATOMIC
#include <boost/atomic.hpp>
#endif

using namespace std;

NS_IZENELIB_UTIL_BEGIN


/**
 * @details
 * The items attached to a profiler must be of the <em>same level</em> as the rest of the items to 
 * ensure the correctness of the Total time calculated from the profilers.
 *
 * For example, with the following setting:
 *  A(){ AA(); }
 *  B(){ .. }
 *
 * There are only \e two ways to measure the methods with one profiler.
 * - Either the methods A() and B() are measured
 * - Or the method AA() is measured alone
 * Measuring A() and AA() with  the same profiler will cause the Total Time of a profiler to be incorrect.
 * In the above case, /e two Profilers should be used.
 *
 */
class Profiler0 {
public:
	Profiler0();
	Profiler0(const std::string & name);
	virtual ~Profiler0();
public:
	
	class Profile {
public:
		Profile(const std::string& name, const Profiler0& profiler);
		void begin(void);
		void end(void);
public:
		std::string name_;
		std::map< pthread_t, std::pair<TimeChecker,double> > thread_;

		// counts the number of begin()-end() pairs that were called
#ifdef ATOMIC		
		boost::atomic_ulong checkCount_;
		boost::atomic<double> currentTime_;
		boost::atomic<double> totalTime_;
		boost::atomic_ulong totalRss_;
		boost::atomic_ulong totalVm_;	
#else
		unsigned long checkCount_;
		double currentTime_;
		double totalTime_;
		unsigned long totalRss_;
		unsigned long totalVm_;	
#endif
	};

public:
	bool attach(Profile& tc);

	void setName(const std::string & name) {
		name_ = name;
	}
	const std::string & getName() {
		return name_;
	}

	/**
	 * @brief   Prints the time measurement
	 * @details
	 * The following is an example of a output line
	 * -* [TEST SUITE][sorting                            :      0.1537 (avg[2] 0.0769)  (61.7470%) ]
	 *   -* TEST_SUITE:      The name of the Profiler
	 *   -* sorting:         An item to that is being profiled
	 *   -* 0.1537:          The total time that the area took to run. It adds the time of alll the threads
	 *   -* avg[2] 0.0769:   Indicates that the avage time for two threads took to run the area 0.0769
	 *   -* 61.7470%:        The percentage of time this item took, based on the Total time of 
	 *                      all the items attached to this profiler
	 */
	void print(const std::string & filePath) const;
	void print(std::ostream & os = std::cout) const;
	void print_detailed() const;

private:
	std::string name_;

	/**
	 * @brief   The list of profiles and their tier
	 * @details
	 * The profiles are given a level so that the total time may be calculated among the same level profiles.
	 * The tier of a Profile is determined by its name. For one Profile to be in tier 1, its name has to be unique.
	 * If a Profiler has a prefix ending with a '.', the profiler will belong to tier 2.
	 * E.g. The following names determine the Profiler's tier
	 *  - "DBLoading": level 1
	 *  - "DBLoading.MF": level 2
	 */
	//std::set<std::pair<unsigned int, Profile*>> profiles_;
	std::list<Profile*> profiles_;
	//std::vector<Profile*> profiles_;	
	
	void displayProfiles_( stringstream &ss, const vector<Profile*>& vp )const;
};

bool compareByCount(const Profiler0::Profile *p1, const Profiler0::Profile *p2);
bool compareByTotalTime(const Profiler0::Profile *p1, const Profiler0::Profile *p2);
bool compareByCurrentTime(const Profiler0::Profile *p1, const Profiler0::Profile *p2);
bool compareByName(const Profiler0::Profile *p1, const Profiler0::Profile *p2);
bool compareByRss(const Profiler0::Profile *p1, const Profiler0::Profile *p2);



/**
 * @details
 * The items attached to a profiler must be of the <em>same level</em> as the rest of the items to 
 * ensure the correctness of the Total time calculated from the profilers.
 *
 * For example, with the following setting:
 *  A(){ AA(); }
 *  B(){ .. }
 *
 * There are only \e two ways to measure the methods with one profiler.
 * - Either the methods A() and B() are measured
 * - Or the method AA() is measured alone
 * Measuring A() and AA() with  the same profiler will cause the Total Time of a profiler to be incorrect.
 * In the above case, /e two Profilers should be used.
 *
 */
class Profiler1 {
public:
	Profiler1();
	Profiler1(const std::string & name);
	virtual ~Profiler1();
public:
	
	class Profile {
public:
		Profile(const std::string& name, const Profiler1& profiler);
		void begin(void);
		void end(void);
public:
		std::string name_;
		std::map< pthread_t, std::pair<TimeChecker1,double> > thread_;

		// counts the number of begin()-end() pairs that were called
#ifdef ATOMIC
		boost::atomic_ulong checkCount_;
		boost::atomic<double> currentTime_;
		boost::atomic<double> totalTime_;
		boost::atomic_ulong totalRss_;
		boost::atomic_ulong totalVm_;	
#else
		unsigned long checkCount_;
		double currentTime_;
		double totalTime_;
		unsigned long totalRss_;
		unsigned long totalVm_; 
#endif
		
	};

public:
	bool attach(Profile& tc);

	void setName(const std::string & name) {
		name_ = name;
	}
	const std::string & getName() {
		return name_;
	}

	/**
	 * @brief   Prints the time measurement
	 * @details
	 * The following is an example of a output line
	 * -* [TEST SUITE][sorting                            :      0.1537 (avg[2] 0.0769)  (61.7470%) ]
	 *   -* TEST_SUITE:      The name of the Profiler
	 *   -* sorting:         An item to that is being profiled
	 *   -* 0.1537:          The total time that the area took to run. It adds the time of alll the threads
	 *   -* avg[2] 0.0769:   Indicates that the avage time for two threads took to run the area 0.0769
	 *   -* 61.7470%:        The percentage of time this item took, based on the Total time of 
	 *                      all the items attached to this profiler
	 */
	void print(const std::string & filePath) const;
	void print(std::ostream & os = std::cout) const;
	void print_detailed() const;

private:
	std::string name_;

	/**
	 * @brief   The list of profiles and their tier
	 * @details
	 * The profiles are given a level so that the total time may be calculated among the same level profiles.
	 * The tier of a Profile is determined by its name. For one Profile to be in tier 1, its name has to be unique.
	 * If a Profiler has a prefix ending with a '.', the profiler will belong to tier 2.
	 * E.g. The following names determine the Profiler's tier
	 *  - "DBLoading": level 1
	 *  - "DBLoading.MF": level 2
	 */
	//std::set<std::pair<unsigned int, Profile*>> profiles_;
	std::list<Profile*> profiles_;
	//std::vector<Profile*> profiles_;	
	
	void displayProfiles_( stringstream &ss, const vector<Profile*>& vp )const;
};

bool compareByCount1(const Profiler1::Profile *p1, const Profiler1::Profile *p2);
bool compareByTotalTime1(const Profiler1::Profile *p1, const Profiler1::Profile *p2);
bool compareByCurrentTime1(const Profiler1::Profile *p1, const Profiler1::Profile *p2);
bool compareByName1(const Profiler1::Profile *p1, const Profiler1::Profile *p2);
bool compareByRss1(const Profiler1::Profile *p1, const Profiler1::Profile *p2);

NS_IZENELIB_UTIL_END

#endif//__PROFILER__
