#ifndef IZENELIB_UTIL_IDUTIL_H_
#define IZENELIB_UTIL_IDUTIL_H_

#include <types.h>

#include <boost/algorithm/string.hpp>
namespace izenelib{
namespace util{

class IdUtil
{
    
    public:
    static uint64_t Get64(uint32_t id1, uint32_t id2)
    {
        uint64_t r = id1;
        r = r<<32;
        r += id2;
        return r;
    }
    
    static std::pair<uint32_t, uint32_t> Get32(uint64_t id)
    {
        std::pair<uint32_t, uint32_t> r;
        r.second = (uint32_t)id;
        r.first = id>>32;
        return r;
    }
    
        
};

}
}
#endif
