#include <ir/index_manager/utility/StringUtils.h>

#include <string>
#include <assert.h>

using namespace std;

NS_IZENELIB_IR_BEGIN
namespace indexmanager{

void trimleft(string& s )
{
    string::iterator it;

    for ( it = s.begin(); it != s.end(); it++ )
        if ( !std::isspace(*it))
            break;

    s.erase( s.begin(), it );
}

void trimright( string& s )
{
    string::difference_type dt;
    string::reverse_iterator it;

    for ( it = s.rbegin(); it != s.rend(); it++ )
        if ( !std::isspace( *it )  && !std::iscntrl( *it ) )
            break;

    dt = s.rend() - it;

    s.erase( s.begin() + dt, s.end() );
}

void trim( string& s )
{
    trimleft( s );
    trimright( s );
}

void trimleft(wiselib::UString& s )
{
    wiselib::UString::iterator it;

    for ( it = s.begin(); it != s.end(); it++ )
        if ( !std::isspace(*it))
            break;
    s.erase( s.begin(), --it );
}

void trimright( wiselib::UString& s )
{
    wiselib::UString::reverse_iterator it = s.rbegin();

    size_t i = 0;
    for ( ; it != s.rend(); it++, i++ )
        if ( !std::isspace( *it )  && !std::iscntrl( *it ) )
            break;
    size_t dt = s.length() - i;
    s.erase( s.begin() + dt, s.end() );
}

void trim( wiselib::UString& s )
{
    trimleft( s );
    trimright( s );
}


int datetime_to_int(string& s)
{
    ///TODO the format should be Y/m/d H:M:S
    vector<string> ps = split(s," ");
    vector<string> datestr = split(ps[0], "/");
    vector<string> timestr = split(ps[1], ":");

    int nYear = atoi(datestr[0].c_str());
    int nMonth = atoi(datestr[1].c_str());
    int nDay = atoi(datestr[2].c_str());

    struct tm atm;
    atm.tm_sec = timestr.size() == 3 ? atoi(timestr[2].c_str()) : 0;
    atm.tm_min = timestr.size() > 2 ? atoi(timestr[1].c_str()) : 0;
    atm.tm_hour = timestr.size() > 1 ? atoi(timestr[0].c_str()) :0;
    assert(nDay >= 1 && nDay <= 31);
    atm.tm_mday = nDay;
    assert(nMonth >= 1 && nMonth <= 12);
    atm.tm_mon = nMonth;        // tm_mon is 0 based
    assert(nYear >= 1900);
    atm.tm_year = nYear - 1900;     // tm_year is 1900 based
    atm.tm_isdst = -1;
#ifdef WIN32
    return _mktime64(&atm);
#else
    return (int32_t)mktime(&atm);
#endif


}


}
NS_IZENELIB_IR_END

