#include <util/profiler/TimeChecker.h>

NS_IZENELIB_UTIL_BEGIN

TimeChecker::TimeChecker() {
	svm_ = evm_ = srss_ = erss_ = 0;
}

TimeChecker::~TimeChecker() {
}



void TimeChecker::begin(void) {
#if defined(WIN32)
	begin_ = GetTickCount();
#else
	gettimeofday(&begin_, 0);
#endif	
	getMemInfo_(svm_, srss_);
}

void TimeChecker::end(void) {
#if defined(WIN32)
	end_ = GetTickCount();
#else
	gettimeofday(&end_, 0);
#endif
	getMemInfo_(evm_, erss_);
}


double TimeChecker::diff(void) {
#if defined(WIN32)
	return (double)(end_ - begin_) / 1000;
#else
	return ((double)end_.tv_sec + (double)end_.tv_usec / 1000000)
			- ((double)begin_.tv_sec + (double)begin_.tv_usec / 1000000);
#endif
}

unsigned long TimeChecker::vm_diff(void) {
	return evm_ - svm_;

}

unsigned long TimeChecker::rss_diff(void) {
	return erss_ - srss_;
}



TimeChecker1::TimeChecker1() {
	svm_ = evm_ = srss_ = erss_ = 0;
}

TimeChecker1::~TimeChecker1() {
}



void TimeChecker1::begin(void) {
#if defined(WIN32)
	begin_ = GetTickCount();
#else
	gettimeofday(&begin_, 0);
#endif	
	getMemInfo_(svm_, srss_);
}

void TimeChecker1::end(void) {
#if defined(WIN32)
	end_ = GetTickCount();
#else
	gettimeofday(&end_, 0);
#endif
	getMemInfo_(evm_, erss_);
}


double TimeChecker1::diff(void) {
#if defined(WIN32)
	return (double)(end_ - begin_) / 1000;
#else
	return ((double)end_.tv_sec + (double)end_.tv_usec / 1000000)
			- ((double)begin_.tv_sec + (double)begin_.tv_usec / 1000000);
#endif
}

unsigned long TimeChecker1::vm_diff(void) {
	return evm_ - svm_;

}

unsigned long TimeChecker1::rss_diff(void) {
	return erss_ - srss_;
}

NS_IZENELIB_UTIL_END // namespace wiselib
