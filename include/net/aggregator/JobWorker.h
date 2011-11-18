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
 * @brief In subclass, function addHandlers() must be implemented to register handlers,
 * we can use help macros to add handler which are defined as member fucntion of subclass.
 *
 * @example
class ConcreteWorker : public JobWorker<ConcreteWorker>
{
public:
    ConcreteWorker WorkerServer(const std::string& host, uint16_t port, unsigned int threadNum)
    : JobWorker<ConcreteWorker>(host, port, threadNum)
    {
    }

public:
    //pure virtual
    void addHandlers()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( ConcreteWorker )

        ADD_WORKER_HANDLER( Function1 )
        ADD_WORKER_HANDLER( Function2 )

        ADD_WORKER_HANDLER_LIST_END()
    }

    bool Function1 (JobRequest& req)
    {
        WORKER_HANDLE_REQUEST_1_1_(req, Data, service_->Function1, DataResult)
        return true;
    }

    bool Function1 (JobRequest& req)
    {
        WORKER_HANDLE_REQUEST_1_1_(req, Data, service_->Function2, DataResult)
        return true;
    }

private:
    WorkerServiceBase* service_;
};
 */
template <
typename ConcreteWorker,
typename Maptype = rde::hash_map<std::string, WorkerHandler<ConcreteWorker> >
>
class JobWorker : public msgpack::rpc::server::base
{

public:
    JobWorker(const std::string& host, uint16_t port, unsigned int threadNum=4)
    : srvInfo_(host, port), threadNum_(threadNum), debug_(false)
    {}

    JobWorker()
    : srvInfo_("", 0)
    {}


public:
    void init(const std::string& host, uint16_t port, unsigned int threadNum, bool debug=false)
    {
        srvInfo_.host_ = host;
        srvInfo_.port_ = port;
        threadNum_ = threadNum;
        debug_ = debug;
    }

    void start()
    {
        if (srvInfo_.host_.empty() || srvInfo_.port_ <= 0)
            throw std::runtime_error("Worker server uninitialized!");

        addHandlers(); // add before start

        instance.listen(srvInfo_.host_, srvInfo_.port_);
        instance.start(threadNum_);
    }

    void startServer(const std::string& host, uint16_t port, unsigned int threadnum=4)
    {
        addHandlers(); // add before start

        srvInfo_.host_ = host;
        srvInfo_.port_ = port;
        instance.listen(host, port);
        instance.start(threadnum);
    }

    void join()
    {
        instance.join();
    }

    void stop()
    {
        instance.end();
        instance.join();
    }

    const ServerInfo& getServerInfo() const
    {
        return srvInfo_;
    }


    virtual bool preHandle(const std::string& identity, std::string& error)
    {
        return true;
    }

    /*virtual*/
    void dispatch(msgpack::rpc::request req)
    {
        try
        {
            // get requested service
            std::string method_plus;
            req.method().convert(&method_plus);
            std::string identity, method;
            size_t pos = method_plus.find(REQUEST_FUNC_DELIMETER);
            if (pos == string::npos)
            {
                req.error(std::string("method error!"));
                return;
            }
            else
            {
                identity = method_plus.substr(0,pos);
                if (pos+1 < method_plus.size())
                    method = method_plus.substr(pos+1);
            }

            // preprocessing
            std::string error;
            if (!preHandle(identity, error))
            {
                req.error(error);
                return;
            }

            if (debug_)
                cout << "#[Worker:"<<srvInfo_.port_<<"] dispatch request: " << method << endl;

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

public:
    /**
     * @brief Implement this function to add external apis (Handlers) which provide services,
     * using addHandler(), or using Macros to simplify adding handlers.
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
    unsigned int threadNum_;

    Maptype handlerList_;

public:
    bool debug_;

};

}} // end - namespace

#endif /* JOB_WORKER_H_ */
