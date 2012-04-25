#ifndef _SF1V5_RPOFILER_GROUP_H_
#define _SF1V5_RPOFILER_GROUP_H_

#include <boost/unordered_map.hpp>
#include <util/profiler/Profiler_h.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

NS_IZENELIB_UTIL_BEGIN


#ifdef SF1_MEMCHECK
#define Profiler0 Profiler
#else
#define Profiler1 Profiler
#endif

class ProfilerGroup //: public Singleton<ProfilerGroup>
{
private:
    ProfilerGroup() {
    }

    ProfilerGroup(const ProfilerGroup & mainProcessProfilerObj);
    ProfilerGroup & operator=(const ProfilerGroup & mainProcessProfilerObj);

public:
    static ProfilerGroup & instance() {
        static ProfilerGroup obj;
        return obj;
    }

public:

    izenelib::util::Profiler & profilerInstance(const std::string & name) {
        Profiler & ref = profilerMap_[name];
        if (ref.getName().empty() ) {
            ref.setName(name);
        }

        return ref;
    }

    void print(std::ostream & os = std::cout) const {
        // Print Current Time
        char time_string[40];
#ifdef WIN32
        // TODO : Need to check if it works correctly.
        SYSTEMTIME currentTime;
        ::GetLocalTime(&currentTime);
        sprintf(time_string, "%4ld-%02ld-%02ld %02ld:%02ld:%02ld",
                currentTime.wYear, currentTime.wMonth, currentTime.wDay,
                currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
#else
        struct timeval timeval;
        gettimeofday(&timeval, 0);
        struct tm* currentTime;
        currentTime = localtime( &timeval.tv_sec);

        sprintf(time_string, "%4d-%02d-%02d %02d:%02d:%02d",
                currentTime->tm_year + 1900, currentTime->tm_mon + 1,
                currentTime->tm_mday, currentTime->tm_hour,
                currentTime->tm_min, currentTime->tm_sec);
#endif
        os << "::::::::::::::::::::::::::::[ PROFILING TIME : "<< time_string
                << " ]::::::::::::::::::::::::::::" << std::endl;
        boost::unordered_map<std::string, Profiler>::const_iterator it;
        for (it = profilerMap_.begin(); it != profilerMap_.end(); it++) {
            it->second.print(os);
            os << std::endl;
        }
    }

private:
    // name | Profiler
    boost::unordered_map<std::string, Profiler> profilerMap_;
};

NS_IZENELIB_UTIL_END

class scoped_profiler
{
    izenelib::util::Profiler::Profile& profile_;
public:
    scoped_profiler(izenelib::util::Profiler::Profile&  pid):profile_(pid)
    {
        profile_.begin();
    }
    ~scoped_profiler(){
        profile_.end();
    }
};

/////////////////////////////////////////////////////////////////////
// Macros of ProfilerGroup
/////////////////////////////////////////////////////////////////////

#ifdef SF1_TIME_CHECK

#define CREATE_PROFILER( PROFILER_ID , PROCESS_NAME , PROFILE_MSG ) \
        static izenelib::util::Profiler::Profile PROFILER_ID ( PROFILE_MSG ,            \
            izenelib::util::ProfilerGroup::instance().profilerInstance( PROCESS_NAME ));
#define START_PROFILER( PROFILER_ID ) PROFILER_ID.begin();
#define STOP_PROFILER(  PROFILER_ID ) PROFILER_ID.end();
#define REPORT_PROFILE_TO_SCREEN() izenelib::util::ProfilerGroup::instance().print();
#define REPORT_PROFILE_TO_FILE( FILENAME ) \
        std::ofstream profilerFile( FILENAME , ios_base::app ); \
        std::ostream  profilerFileBuffer( profilerFile.rdbuf() ); \
        izenelib::util::ProfilerGroup::instance().print( profilerFileBuffer ); \
        profilerFile.close();
#define SCOPED_PROFILER( PROFILER_ID ) \
        scoped_profiler temp_profiler(PROFILER_ID);

#define CREATE_SCOPED_PROFILER(PROFILER_ID , PROCESS_NAME , PROFILE_MSG ) \
    static izenelib::util::Profiler::Profile PROFILER_ID ( PROFILE_MSG , \
            izenelib::util::ProfilerGroup::instance().profilerInstance( PROCESS_NAME )); \
            scoped_profiler temp_profiler(PROFILER_ID);

//report to sstream
#define REPORT_PROFILE_TO_SS( ss ) \
        izenelib::util::ProfilerGroup::instance().print( ss ); \

#else

#define CREATE_PROFILER( PROFILER_ID , PROCESS_NAME , PROFILE_MSG )
#define START_PROFILER( PROFILER_ID )
#define STOP_PROFILER( PROFILER_ID )
#define REPORT_PROFILE_TO_SCREEN()
#define REPORT_PROFILE_TO_FILE( FILENAME )
#define SCOPED_PROFILER(PROFILER_ID)
#define CREATE_SCOPED_PROFILER(PROFILER_ID , PROCESS_NAME , PROFILE_MSG )
#define REPORT_PROFILE_TO_SS( ss )
#endif

#endif  //_SF1V5_RPOFILER_GROUP_H_
