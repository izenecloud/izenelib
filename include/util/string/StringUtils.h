#ifndef IZENELIB_UTIL_STRINGUTILS_H
#define IZENELIB_UTIL_STRINGUTILS_H

#include <util/ustring/UString.h>

namespace izenelib{ namespace util{

template<typename StringType, typename Container>
void Split(
    const StringType& szText, 
    Container& out, 
    const StringType& delimiter)
{
    StringType str(szText);
    size_t n = 0, nOld=0;
    while (n != StringType::npos)
    {
        n = str.find(delimiter,n);
        if (n != StringType::npos)
        {
            if (n != nOld)
                out.push_back(str.substr(nOld, n-nOld));
            n += delimiter.length();
            nOld = n;
        }
    }
    out.push_back(str.substr(nOld, str.length()-nOld));
}

template<typename Container>
void Split(
    const UString& szText, 
    Container& out, 
    UString::EncodingType encoding, 
    char delimiter = ' ')
{
    UString sep(" ",encoding);
    sep[0] = delimiter;
    Split(szText,out,sep);
}

template<typename StringType, typename Container>
void Split(
    const StringType& szText, 
    Container& out, 
    char Separator = ' ')
{
    StringType sep(" ");
    sep[0] = Separator;
    return Split<StringType, Container>(szText, out, sep);
}

void TrimLeft(std::string& s );

void TrimRight( std::string& s );

void Trim( std::string& s );

void TrimLeft(izenelib::util::UString& s );

void TrimRight( izenelib::util::UString& s );

void Trim( izenelib::util::UString& s );


template<typename StringType>
bool IsPrefix(const StringType & prefix, const StringType & str)
{
///compare of UString is not correct
//    return (str.length() >= prefix.length() &&
//        str.compare(0, prefix.length(), prefix) == 0);
    return (str.length() >= prefix.length() &&
        str.substr(0, prefix.length()).compare(prefix) == 0);
}

template<typename StringType>
bool IsSuffix(const StringType & suffix, const StringType & str)
{
///compare of UString is not correct
//    return (str.length() >= suffix.length() &&
//        str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
    return (str.length() >= suffix.length() &&
        str.substr(str.length() - suffix.length(), suffix.length()).compare(suffix) == 0 );
}

template<typename StringType>
bool IsSubString(const StringType & substring, const StringType & str)
{
    return (str.length() >= substring.length() &&
        str.find(substring) != StringType::npos);
}

}}

#endif
