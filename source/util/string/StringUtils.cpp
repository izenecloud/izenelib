#include <util/string/StringUtils.h>

namespace izenelib{ namespace util{

void TrimLeft(string& s )
{
    string::iterator it;

    for ( it = s.begin(); it != s.end(); it++ )
        if ( !std::isspace(*it))
            break;

    s.erase( s.begin(), it );
}

void TrimRight( string& s )
{
    string::difference_type dt;
    string::reverse_iterator it;

    for ( it = s.rbegin(); it != s.rend(); it++ )
        if ( !std::isspace( *it )  && !std::iscntrl( *it ) )
            break;

    dt = s.rend() - it;

    s.erase( s.begin() + dt, s.end() );
}

void Trim( string& s )
{
    TrimLeft( s );
    TrimRight( s );
}

void TrimLeft(izenelib::util::UString& s )
{
    izenelib::util::UString::iterator it;

    for ( it = s.begin(); it != s.end(); it++ )
        if ( !izenelib::util::UString::isThisSpaceChar(*it))
            break;
    s.erase( s.begin(), it );
}

void TrimRight( izenelib::util::UString& s )
{
    izenelib::util::UString::reverse_iterator it = s.rbegin();

    size_t i = 0;
    for ( ; it != s.rend(); it++, i++ )
        if ( !izenelib::util::UString::isThisSpaceChar( *it )  && !izenelib::util::UString::isThisControlChar( *it ) )
            break;
    size_t dt = s.length() - i;
    s.erase( s.begin() + dt, s.end() );
}

void Trim( izenelib::util::UString& s )
{
    if(s.empty())
        return;
    TrimLeft( s );
    TrimRight( s );
}

}}

