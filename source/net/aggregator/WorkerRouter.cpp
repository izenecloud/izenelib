#include <net/aggregator/WorkerRouter.h>
#include <net/aggregator/WorkerHandler.h>

#include <glog/logging.h>

namespace net{
namespace aggregator{

WorkerRouter::~WorkerRouter()
{
    for (map_type::iterator it = table_.begin();
        it != table_.end(); ++it)
    {
        delete it->second;
    }
}

WorkerHandlerBase* WorkerRouter::find(const std::string& key) const
{
    map_type::const_iterator it = table_.find(key);

    if (it != table_.end())
        return it->second;

    return NULL;
}

bool WorkerRouter::add(const std::string& key, WorkerHandlerBase* handler)
{
    map_type::value_type keyHandler(key, handler);

    if (table_.insert(keyHandler).second)
        return true;

    LOG(ERROR) << "failed to add handler for duplicated key: " << key;
    return false;
}

}} // end - namespace
