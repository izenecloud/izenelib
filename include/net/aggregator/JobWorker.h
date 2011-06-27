/**
 * @file JobWorker.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief JobWorker performs as a rpc server, it recieves request and reply result.
 *
 */
#ifndef JOB_WORKER_H_
#define JOB_WORKER_H_

#include "AggregatorConfig.h"
#include "JobInfo.h"
#include "WorkerHandler.h"

//#include <hash_map>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/msgpack/rpc/server.h>


namespace net{
namespace aggregator{

/**
 * Job Worker Base class
 * @brief In subclass, addHandlers() must be implemented to extend service APIs.
 */
template <
typename ConcreteWorker,
typename Maptype = rde::hash_map<std::string, WorkerHandler<ConcreteWorker> >
>
class JobWorker : public msgpack::rpc::server::base
{

public:
    JobWorker(const std::string& host, uint16_t port)
    : srvInfo_(host, port)
    {}

public:
    void Start()
    {
        addHandlers(); // add before run

        instance.listen(srvInfo_.host_, srvInfo_.port_);
        instance.run(4); //xxx
    }

    /*virtual*/
    void dispatch(msgpack::rpc::request req)
    {
        try
        {
            // get requested service
            std::string method;
            req.method().convert(&method);

            cout << "worker dispatch --> " << method << endl;

            typename Maptype::iterator handler = handlerList_.find(method);
            if (handler != handlerList_.end())
            {
                // process request
                (static_cast<ConcreteWorker*>(this)->*(handler->second.callback_))(req);
            }
            else
            {
                req.error(msgpack::rpc::NO_METHOD_ERROR);
            }
        }
        catch (msgpack::type_error& e)
        {
            req.error(msgpack::rpc::ARGUMENT_ERROR);
            return;
        }
        catch (std::exception& e)
        {
            req.error(std::string(e.what()));
            return;
        }
    }

    /**
     * @brief Implement this function to add external apis (Handlers) which provide services, using addHandler().
     * TODO, define some Macros to simplify adding handlers.
     */
    virtual void addHandlers() = 0;

protected:
    void addHandler(const string& method, WorkerHandler<ConcreteWorker> handler)
    {
        handlerList_.insert(typename Maptype::value_type(method, handler));
        // handlerList_[method] = serviceHandle;
    }

protected:
    ServerInfo srvInfo_;

    Maptype handlerList_;
};

}} // end - namespace

#endif /* JOB_WORKER_H_ */
