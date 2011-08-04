#ifndef IZENELIB_NET_AGGREGATOR_UTIL_H_
#define IZENELIB_NET_AGGREGATOR_UTIL_H_

#include <util/id_util.h>

namespace net{
namespace aggregator{
typedef uint32_t workerid_t;
typedef uint32_t docid_t;
typedef uint64_t wdocid_t;
class Util
{
public:
    static wdocid_t GetWDocId(workerid_t wid, docid_t did)
    {
        return izenelib::util::IdUtil::Get64(wid, did);
    }
    
    static std::pair<workerid_t, docid_t>  GetWorkerAndDocId(wdocid_t id)
    {
        return izenelib::util::IdUtil::Get32(id);
    }

};


}}

#endif /* AGGREGATOR_CONFIG_H_ */
