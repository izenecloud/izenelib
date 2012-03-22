/**
 * @file WorkerRouter.h
 * @brief map from key string to each WorkerHandlerBase instance
 * @author Jun Jiang
 * @date 2012-03-20
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_ROUTER_H_
#define IZENE_NET_AGGREGATOR_WORKER_ROUTER_H_

#include <string>
#include <map>

namespace net{
namespace aggregator{

class WorkerHandlerBase;

class WorkerRouter
{
public:
    ~WorkerRouter();

    /**
     * @return NULL if not found
     */
    WorkerHandlerBase* find(const std::string& key) const;

    /**
     * @return true for success, false for failure
     * @post if true is returned, @p handler would be owned and destoryed by WorkerRouter
     */
    bool add(const std::string& key, WorkerHandlerBase* handler);

private:
    typedef std::map<std::string, WorkerHandlerBase*> map_type;
    map_type table_;
};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_ROUTER_H_
