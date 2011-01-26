#include <util/profiler/Profiler.h>
#include <util/profiler/YString.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;
NS_IZENELIB_UTIL_BEGIN

//================================== Profiler::Profile ====================================

const size_t LENGTH = 130;
const size_t INFO_LENGTH = 90; //size of the information part
const size_t TIME_LENGTH = 45;

Profiler0::Profiler0() {
}

Profiler0::Profiler0(const string & name) :
	name_(name) {
}

Profiler0::~Profiler0() {
}

bool Profiler0::attach(Profiler0::Profile& profile) {
	//size_t pos = profile.name_.find( '.' );
	profiles_.push_back( &profile);
	return true;
}

void Profiler0::print(const std::string & filePath) const {
	ofstream fout(filePath.c_str(), ios_base::out | ios_base::trunc);

	print(fout);

	fout.close();
}

void Profiler0::print(ostream & os) const {

	list<Profile*>::const_iterator it;

	stringstream ss;

	ss.setf(ios::fixed, ios::floatfield);
	streamsize defaultSize = ss.precision( 4);

	string str, str2;

	// setting the title bar
	str.assign(LENGTH - strlen(" Time Measurement "), '-');
	str[0] = '[';
	str[str.length()-1] = ']';
	str.replace(LENGTH/2 - (strlen(" Time Measurement ")/2), 0,
			" Time Measurement ");
	ss << "[" << name_ << "]" << str << endl;;

	// prints out the description of each column
	str.assign(name_.length(), ' ');
	int len = min(strlen("GROUP NAME"), name_.length() );
	str.replace( 0, len, "GROUP NAME", len);
	ss << "[" << str << "]";

	str.assign(INFO_LENGTH, ' ');
	str.replace( 0, strlen("PROFILE NAME"), "PROFILE NAME");
	ss << "[" << str << "]["
			<< "CURRENTIME, TOTAL TIME , CALL COUNT, NUMBER OF THREADS, RSS, VM]"
			<< endl;

	vector<Profile*> vpro;
	vector<Profile*>::const_iterator pit;
	map<YString, double> namestat;
	for (it = profiles_.begin(); it != profiles_.end(); it++) {
		vpro.push_back(*it);
		YString name = (*it)->name_;
		size_t pos = name.find(":");
		YString namekey = YString( name.substr(0, pos) ).trim();
		namekey.to_lower();
		namestat[namekey] += (*it)->totalTime_;
	}

	ss<<"\n   total \n"<<endl;
	map<YString, double>::iterator tit;
	for (tit = namestat.begin(); tit != namestat.end(); tit++) {
		str.assign(INFO_LENGTH, ' ');
		string sname = tit->first + " (total)";
		str.replace( 0, sname.length(), sname);

		stringstream sstmp;
		sstmp << namestat[tit->first];
		str2.assign(TIME_LENGTH, ' ');
		str2.replace( 0, sstmp.str().length(), sstmp.str() );
		ss << "[" << name_ << "][" << str << "][ " << str2 << " ]" << endl;
	}
	//ss<<"\n   sorted by called Sequence\n"<<endl;
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by name \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByName);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by total time  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByTotalTime);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by count  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByCount);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by memory  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByRss);
	//displayProfiles_(ss, vpro);

	ss<<"\n   sorted by current time  \n"<<endl;
	sort(vpro.begin(), vpro.end(), compareByCurrentTime);
	displayProfiles_(ss, vpro);

	ss.unsetf(ios::floatfield);
	ss.precision(defaultSize);

	os << ss.str();
}

void Profiler0::print_detailed() const {
	// iterate for all profiles
	//list<Profile*>::const_iterator it = profiles_.begin();
	list<Profile*>::const_iterator it = profiles_.begin();
	while (it != profiles_.end() ) {
		// print out name of profile
		cout << "name: " << (*it)->name_ << endl;
		// iterate for all tid
		map< pthread_t, pair<TimeChecker,double> >::iterator it2 = (*it)->thread_.begin();
		double totaltime = 0.0;
		while (it2 != (*it)->thread_.end() ) {
			// print out time information
			cout << "    elapsed time: " << it2->second.second;
			cout << ", id: " << it2->first << " ";
			cout << endl;
			// accumulate time
			totaltime += it2->second.second;
			// next
			it2++;
		}
		// print out total time
		cout << "    * total time: " << totaltime;
		cout << ", average time: " << (totaltime/(*it)->thread_.size());
		cout << endl;
		// next
		it++;
	}
}

//================================== Profiler0::Profile ====================================

Profiler0::Profile::Profile(const string& name, const Profiler0& profiler) :
	checkCount_( 0), currentTime_(0.0), totalTime_(0.0) {
	// set name
	name_ = name;
	// attach this to profiler
	const_cast<Profiler0&>(profiler).attach( *this);
}

void Profiler0::Profile::begin(void) {
	// get tid
	pthread_t tid = pthread_self();
	// find TimeChecker for tid
	map< pthread_t, pair<TimeChecker,double> >::iterator it = thread_.find(tid);
	if (it == thread_.end() ) {
		// add new time checker for tid
		TimeChecker tc;
		pair<map< pthread_t, pair<TimeChecker,double> >::iterator, bool> ret;
		ret = thread_.insert(pair< pthread_t, pair< TimeChecker, double> >(tid,
				pair< TimeChecker, double >(tc, 0.0)) );
		// assign iterator
		it = ret.first;
	}

	// begin TimeChecker
	it->second.first.begin();
}

void Profiler0::Profile::end(void) {
	// get tid
	pthread_t tid = pthread_self();
	// find TimeChecker for tid
	map< pthread_t, pair<TimeChecker,double> >::iterator it = thread_.find(tid);
	if (it == thread_.end() ) {
		throw std::runtime_error( "[izenelib::util::Profiler::Profile]: begin(), not called before end()" );
	}
	// end TimeChecker
	it->second.first.end();
	// accumulate time
	it->second.second += it->second.first.diff();
#ifdef ATOMIC
	currentTime_.store(it->second.first.diff());
	double t = totalTime_.load();
	t += it->second.first.diff();
	totalTime_.exchange(t);
#else	
	currentTime_ = it->second.first.diff();
	totalTime_ += it->second.first.diff();
#endif	
	totalRss_ += it->second.first.rss_diff();
	totalVm_ += it->second.first.vm_diff();
	
	++checkCount_;
}

void Profiler0::displayProfiles_(stringstream &ss, const vector<Profile*>& vp) const {
	string str, str2;
	vector<Profile*>::const_iterator pit;
	for (pit = vp.begin(); pit != vp.end(); pit++) {
		str.assign(INFO_LENGTH, ' ');
		str.replace( 0, (*pit)->name_.length(), (*pit)->name_);
		stringstream sstmp;
		sstmp << (*pit)->currentTime_ << " , "<<(*pit)->totalTime_ << " , " << (*pit)->checkCount_ << " , " << (*pit)->thread_.size();
		sstmp <<" , "<< (*pit)->totalRss_ << " , " << (*pit)->totalVm_;

		str2.assign(TIME_LENGTH, ' ');
		str2.replace( 0, sstmp.str().length(), sstmp.str() );

		ss << "[" << name_ << "][" << str << "][ " << str2 << " ]" << endl;

		//TODO: save code from exception, dividing with 0
		// description 
		// [<Profiler name>][<Profile name>]   : <time> (avg[<number of threads.] <average time>, callCnt:<numberof calls>) <percentage>

		// ss << "[" << name_ << "][" << str << "][ " << *it_profile 
		//<< ", " << (*it)->checkCount_ << ", " << (*it)->thread_.size() << " ]" << endl;
	}
}

bool compareByTotalTime(const Profiler0::Profile *p1,
		const Profiler0::Profile *p2) {
	return (p1->totalTime_ > p2->totalTime_);
}

bool compareByCurrentTime(const Profiler0::Profile *p1,
		const Profiler0::Profile *p2) {
	return (p1->currentTime_ > p2->currentTime_);
}

bool compareByCount(const Profiler0::Profile *p1, const Profiler0::Profile *p2) {
	return (p1->checkCount_ > p2->checkCount_);
}

bool compareByName(const Profiler0::Profile *p1, const Profiler0::Profile *p2) {
	return (p1->name_.compare(p2->name_) == -1 );
}

bool compareByRss(const Profiler0::Profile *p1, const Profiler0::Profile *p2) {
	return (p1->totalRss_ > p2->totalRss_ );
}

/////////////////////////////////////////////////////////////////////////////////////


Profiler1::Profiler1() {
}

Profiler1::Profiler1(const string & name) :
	name_(name) {
}

Profiler1::~Profiler1() {
}

bool Profiler1::attach(Profiler1::Profile& profile) {
	//size_t pos = profile.name_.find( '.' );
	profiles_.push_back( &profile);
	return true;
}

void Profiler1::print(const std::string & filePath) const {
	ofstream fout(filePath.c_str(), ios_base::out | ios_base::trunc);

	print(fout);

	fout.close();
}

void Profiler1::print(ostream & os) const {

	list<Profile*>::const_iterator it;

	stringstream ss;

	ss.setf(ios::fixed, ios::floatfield);
	streamsize defaultSize = ss.precision( 4);

	string str, str2;

	// setting the title bar
	str.assign(LENGTH - strlen(" Time Measurement "), '-');
	str[0] = '[';
	str[str.length()-1] = ']';
	str.replace(LENGTH/2 - (strlen(" Time Measurement ")/2), 0,
			" Time Measurement ");
	ss << "[" << name_ << "]" << str << endl;;

	// prints out the description of each column
	str.assign(name_.length(), ' ');
	int len = min(strlen("GROUP NAME"), name_.length() );
	str.replace( 0, len, "GROUP NAME", len);
	ss << "[" << str << "]";

	str.assign(INFO_LENGTH, ' ');
	str.replace( 0, strlen("PROFILE NAME"), "PROFILE NAME");
	ss << "[" << str << "]["
			<< "CURREN TIME, TOTAL TIME , CALL COUNT, NUMBER OF THREADS, RSS, VM]"
			<< endl;

	vector<Profile*> vpro;
	vector<Profile*>::const_iterator pit;
	map<YString, double> namestat;
	for (it = profiles_.begin(); it != profiles_.end(); it++) {
		vpro.push_back(*it);
		YString name = (*it)->name_;
		size_t pos = name.find(":");
		YString namekey = YString( name.substr(0, pos) ).trim();
		namekey.to_lower();
		namestat[namekey] += (*it)->totalTime_;
	}

	ss<<"\n   total \n"<<endl;
	map<YString, double>::iterator tit;
	for (tit = namestat.begin(); tit != namestat.end(); tit++) {
		str.assign(INFO_LENGTH, ' ');
		string sname = tit->first + " (total)";
		str.replace( 0, sname.length(), sname);

		stringstream sstmp;
		sstmp << namestat[tit->first];
		str2.assign(TIME_LENGTH, ' ');
		str2.replace( 0, sstmp.str().length(), sstmp.str() );
		ss << "[" << name_ << "][" << str << "][ " << str2 << " ]" << endl;
	}
	//ss<<"\n   sorted by called Sequence\n"<<endl;
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by name \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByName1);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by total time  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByTotalTime1);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by count  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByCount1);
	//displayProfiles_(ss, vpro);

	//ss<<"\n   sorted by memory  \n"<<endl;
	//sort(vpro.begin(), vpro.end(), compareByRss1);
	//displayProfiles_(ss, vpro);

	ss<<"\n   sorted by current time  \n"<<endl;
	sort(vpro.begin(), vpro.end(), compareByCurrentTime1);
	displayProfiles_(ss, vpro);

	ss.unsetf(ios::floatfield);
	ss.precision(defaultSize);

	os << ss.str();
}

void Profiler1::print_detailed() const {
	// iterate for all profiles
	//list<Profile*>::const_iterator it = profiles_.begin();
	list<Profile*>::const_iterator it = profiles_.begin();
	while (it != profiles_.end() ) {
		// print out name of profile
		cout << "name: " << (*it)->name_ << endl;
		// iterate for all tid
		map< pthread_t, pair<TimeChecker1,double> >::iterator it2 = (*it)->thread_.begin();
		double totaltime = 0.0;
		while (it2 != (*it)->thread_.end() ) {
			// print out time information
			cout << "    elapsed time: " << it2->second.second;
			cout << ", id: " << it2->first << " ";
			cout << endl;
			// accumulate time
			totaltime += it2->second.second;
			// next
			it2++;
		}
		// print out total time
		cout << "    * total time: " << totaltime;
		cout << ", average time: " << (totaltime/(*it)->thread_.size());
		cout << endl;
		// next
		it++;
	}
}

//================================== Profiler1::Profile ====================================

Profiler1::Profile::Profile(const string& name, const Profiler1& profiler) :
	checkCount_( 0), currentTime_(0.0), totalTime_(0.0) {
	// set name
	name_ = name;
	// attach this to profiler
	const_cast<Profiler1&>(profiler).attach( *this);
}

void Profiler1::Profile::begin(void) {
	// get tid
	pthread_t tid = pthread_self();
	// find TimeChecker for tid
	map< pthread_t, pair<TimeChecker1,double> >::iterator it = thread_.find(tid);
	if (it == thread_.end() ) {
		// add new time checker for tid
		TimeChecker1 tc;
		pair<map< pthread_t, pair<TimeChecker1,double> >::iterator, bool> ret;
		ret = thread_.insert(pair< pthread_t, pair< TimeChecker1, double> >(tid,
				pair< TimeChecker1, double >(tc, 0.0)) );
		// assign iterator
		it = ret.first;
	}

	// begin TimeChecker
	it->second.first.begin();
}

void Profiler1::Profile::end(void) {
	// get tid
	pthread_t tid = pthread_self();
	// find TimeChecker for tid
	map< pthread_t, pair<TimeChecker1,double> >::iterator it = thread_.find(tid);
	if (it == thread_.end() ) {
		throw std::runtime_error( "[izenelib::util::Profiler::Profile]: begin(), not called before end()" );
	}
	// end TimeChecker
	it->second.first.end();
	// accumulate time
	it->second.second += it->second.first.diff();

#ifdef ATOMIC
	currentTime_.store(it->second.first.diff());
	double t = totalTime_.load();
	t += it->second.first.diff();
	totalTime_.exchange(t);
#else	
	currentTime_ = it->second.first.diff();
	totalTime_ += it->second.first.diff();
#endif	
	totalRss_ += it->second.first.rss_diff();
	totalVm_ += it->second.first.vm_diff();
	checkCount_++;
}

void Profiler1::displayProfiles_(stringstream &ss, const vector<Profile*>& vp) const {
	string str, str2;
	vector<Profile*>::const_iterator pit;
	for (pit = vp.begin(); pit != vp.end(); pit++) {
		str.assign(INFO_LENGTH, ' ');
		str.replace( 0, (*pit)->name_.length(), (*pit)->name_);
		stringstream sstmp;
		sstmp << (*pit)->currentTime_ << " , "<<(*pit)->totalTime_ << " , " << (*pit)->checkCount_ << " , " << (*pit)->thread_.size();
		sstmp <<" , "<< (*pit)->totalRss_ << " , " << (*pit)->totalVm_;

		str2.assign(TIME_LENGTH, ' ');
		str2.replace( 0, sstmp.str().length(), sstmp.str() );

		ss << "[" << name_ << "][" << str << "][ " << str2 << " ]" << endl;

		//TODO: save code from exception, dividing with 0
		// description 
		// [<Profiler name>][<Profile name>]   : <time> (avg[<number of threads.] <average time>, callCnt:<numberof calls>) <percentage>

		// ss << "[" << name_ << "][" << str << "][ " << *it_profile 
		//<< ", " << (*it)->checkCount_ << ", " << (*it)->thread_.size() << " ]" << endl;
	}
}

bool compareByTotalTime1(const Profiler1::Profile *p1,
		const Profiler1::Profile *p2) {
	return (p1->totalTime_ > p2->totalTime_);
}

bool compareByCurrentTime1(const Profiler1::Profile *p1,
		const Profiler1::Profile *p2) {
	return (p1->currentTime_ > p2->currentTime_);
}

bool compareByCount1(const Profiler1::Profile *p1, const Profiler1::Profile *p2) {
	return (p1->checkCount_ > p2->checkCount_);
}

bool compareByName1(const Profiler1::Profile *p1, const Profiler1::Profile *p2) {
	return (p1->name_.compare(p2->name_) == -1 );
}

bool compareByRss1(const Profiler1::Profile *p1, const Profiler1::Profile *p2) {
	return (p1->totalRss_ > p2->totalRss_ );
}

NS_IZENELIB_UTIL_END

