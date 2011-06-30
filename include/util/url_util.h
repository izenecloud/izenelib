#ifndef IZENELIB_UTIL_URLUTIL_H_
#define IZENELIB_UTIL_URLUTIL_H_

#include <types.h>

#include <boost/algorithm/string.hpp>
namespace izenelib{
namespace util{

class UrlUtil
{
    
    public:
    static bool GetSite(const std::string& url, std::string& site)
    {
        std::string wurl = url;
        if(boost::algorithm::starts_with(wurl, "http://"))
        {
            wurl = wurl.substr(7);
        }
        std::size_t found = wurl.find_first_of('/');
        if(found==std::string::npos) site = wurl;
        else
        {
            site = wurl.substr(0, found);
        }
        return true;
    }
    
    static bool GetBaseSite(const std::string& url, std::string& base_site)
    {
        std::string site;
        if(!GetSite(url, site)) return false;
        static std::string suffix[5] = {".com", ".com.cn", ".cn", ".org", ".net"};
        uint32_t suffix_count = sizeof(suffix)/sizeof(std::string);
        for(uint32_t i=0;i<suffix_count;i++)
        {
            if(boost::algorithm::ends_with(site, suffix[i]))
            {
                std::vector<std::string> vec1;
                boost::algorithm::split( vec1, suffix[i], boost::algorithm::is_any_of("\\.") );
                std::vector<std::string> vec2;
                boost::algorithm::split( vec2, site, boost::algorithm::is_any_of("\\.") );
                uint32_t start2 = vec2.size()-vec1.size();
                base_site = "";
                for(uint32_t i=start2;i<vec2.size();i++)
                {
                    if(i>start2) base_site+=".";
                    base_site+=vec2[i];
                }
                return true;
            }
        }
        return false;
    }
    
};

}
}
#endif
