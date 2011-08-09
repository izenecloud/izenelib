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
 * @brief In subclass, function addHandlers() must be implemented to extend service APIs,
 * we can use help macros to add handle functions which are defined in subclass.
 * @example
 *
class ConcreteWorker : public JobWorker<ConcreteWorker>
{
public:
    ConcreteWorker(const std::string& host, uint16_t port, ...)
    :JobWorker<ConcreteWorker>(host, port)
    , ...
    {
    }

public:
    //pure virtual
    void addHandlers()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( ConcreteWorker )

        ADD_WORKER_HANDLER( functionName1 )
        ADD_WORKER_HANDLER( functionName2 )

        ADD_WORKER_HANDLER_LIST_END()
    }

    bool functionName1 (JobRequest& req)
    {
        WORKER_HANDLE_1_1(req, Data, processFunction1, DataResult)
        return true;
    }

    bool functionName2 (JobRequest& req)
    {
        WORKER_HANDLE_1_1(req, Data, processFunction2, DataResult)
        return true;
    }

private:
    void  processFunction1(Data& param, DataResult& res)
    {
        // do something
    }

    void  processFunction2(Data& param, DataResult& res)
    {
        // do something
    }
};

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

    /// local worker
    JobWorker()
    {}


public:
    /**
     * @brief implement this function using concrete data type, if worker need to be called locally.
     * if it's not work as a server, no need to Start().
     *
         bool call(
            const std::string& func,
            const Data& request,
            DataResult& result,
            std::string& error) ;
     *
     */

public:
    void start(unsigned int threadnum=4)
    {
        addHandlers(); // add before run

        instance.listen(srvInfo_.host_, srvInfo_.port_);
        instance.start(threadnum);
    }

    void startServer(const std::string& host, uint16_t port, unsigned int threadnum=4)
    {
        addHandlers(); // add before started

        srvInfo_.host_ = host;
        srvInfo_.port_ = port;
        instance.listen(host, port);
        instance.start(threadnum);
    }

    void run(unsigned int threadnum=4)
    {
        start(threadnum);
        join();
    }

    void join()
    {
        instance.join();
    }

    void end()
    {
        instance.end();
    }

    const ServerInfo& getServerInfo() const
    {
        return srvInfo_;
    }

    virtual bool preHandle(const std::string& key)
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
            std::string key, method;
            size_t pos = method_plus.find(REQUEST_FUNC_DELIMETER);
            if (pos == string::npos)
            {
                req.error(std::string("method error!"));
                return;
            }
            else
            {
                key = method_plus.substr(0,pos);
                if (pos+1 < method_plus.size())
                    method = method_plus.substr(pos+1);
                cout <<method_plus<< " key: " << key<<", method: "<<method<<endl;
            }

            if (!preHandle(key))
            {
                std::string error = "Failed to handle: " + key + "";
                req.error(error);
                return;
            }

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

    Maptype handlerList_;
};

}} // end - namespace

#endif /* JOB_WORKER_H_ */
