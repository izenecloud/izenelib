#include <util/csv.h>

#include <iostream>

NS_IZENELIB_UTIL_BEGIN

void parse_csv(const std::string &str, std::vector<std::vector<std::string> >& ret)
{
    csv_parser c(str.begin(), str.end());
    for (csv_iterator p(c), q; p!=q; ++p)
    {
        std::vector<std::string> w(p->begin(), p->end());
        ret.push_back(std::vector<std::string>());
        swap(ret.back(), w);
    }
}
NS_IZENELIB_UTIL_END