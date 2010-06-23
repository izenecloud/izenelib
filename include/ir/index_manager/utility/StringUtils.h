/**
* @file        StringUtils.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Collections of string utilities functions
*/
#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <ir/index_manager/utility/system.h>
#include <string>
#include <vector>
#include <util/ustring/UString.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template <typename TString> TString toLower(const TString& str)
{
    TString szResp(str);
    std::transform(szResp.begin(),szResp.end(), szResp.begin(), (int(*)(int))std::tolower);
    return szResp;
}


template<typename _TX>
std::string append(const std::string& szLeft,const _TX xRight)
{
    std::stringstream ss;
    ss << szLeft << xRight;
    return ss.str();
}


template<typename TString, typename TSep>
std::vector<TString> split(const TString& szText, TSep Separator = " ")
{
    std::vector<TString> vec;
    TString str(szText);
    TString sep(Separator);
    size_t n = 0, nOld=0;
    while (n != TString::npos)
    {
        n = str.find(sep,n);
        if (n != TString::npos)
        {
            if (n != nOld)
                vec.push_back(str.substr(nOld, n-nOld));
            n += sep.length();
            nOld = n;
        }
    }
    vec.push_back(str.substr(nOld, str.length()-nOld));

    return vec;
}

template<typename TString>
std::vector<TString> split(const TString& szText, char Separator = ' ')
{
    TString sep(" ");
    sep[0] = Separator;
    return split<TString, TString>(szText, sep);
}

void trimleft(std::string& s );

void trimright( std::string& s );

void trim( std::string& s );

void trimleft(izenelib::util::UString& s );

void trimright( izenelib::util::UString& s );

void trim( izenelib::util::UString& s );

int datetime_to_int(std::string& s);


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

}

NS_IZENELIB_IR_END

#endif
