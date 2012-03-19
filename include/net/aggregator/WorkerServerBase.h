/**
 * @file WorkerServerBase.h
 * @author Zhongxia Li
 * @date Jun 23, 2011
 * @brief WokerServer performs as a rpc server, it recieves requests and reply.
 *
 */
#ifndef IZENE_NET_AGGREGATOR_WORKER_SERVER_BASE_H_
#define IZENE_NET_AGGREGATOR_WORKER_SERVER_BASE_H_

#include "AggregatorConfig.h"
#include "Typedef.h"
#include "WorkerServerHandler.h"

//#include <hash_map>
#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/msgpack/rpc/server.h>


namespace net{
namespace aggregator{

/**
 * Base class for WokerServer
 * @brief A WorkerServer have to implement the abstract addServerHandlers() interface. <br>
 * virtual preprocess() method also expected if needed.
 *
 * @example

class WorkerServer : public WorkerServerBase<WorkerServer>
{
public:
    WorkerServer() {}

    WorkerServer(const std::string& host, uint16_t port, boost::shared_ptr<SearchService> searchService)
    :WorkerServerBase<WorkerServer>(host, port)
    ,searchService_(searchService)
    {
    }

public:
    //pure virtual
    void method_plus()
    {
        ADD_WORKER_HANDLER_LIST_BEGIN( WorkerServer )

        ADD_WORKER_HANDLER( Function1 )
        ADD_WORKER_HANDLER( Function2 )

        ADD_WORKER_HANDLER_LIST_END()
    }

    bool Function1 (request_t& req)
    {
        WORKER_HANDLE_1_1(req, Data, worker_->Function1, DataResult)
        return true;
    }

    bool Function1 (request_t& req)
    {
        WORKER_HANDLE_1_1(req, Data, worker_->Function2, DataResult)
        return true;
    }

private:
    Worker* worker_;
}

 *
 */

template <
typename WorkerServer,
typename Maptype = rde::hash_map<std::string, WorkerHandler<WorkerServer> >
>
class WorkerServerBase : public msgpack::rpc::server::base
{

public:
    WorkerServerBase(const std::string& host, uint16_t port, unsigned int threadNum=4)
    : srvInfo_(host, port), threadNum_(threadNum), debug_(false)
    {}

    WorkerServerBase()
    : srvInfo_("", 0)
    {}

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

        addServerHandlers(); // add before start

        instance.listen(srvInfo_.host_, srvInfo_.port_);
        instance.start(threadNum_);
    }

    void startServer(const std::string& host, uint16_t port, unsigned int threadnum=4)
    {
        addServerHandlers(); // add before start

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

public:
    /**
     * Interface for adding worker server handlers
     */
    virtual void addServerHandlers() = 0;

    /**
     * Preprocess before dispatch requests to handlers
     * @param identity typically it's collection name
     * @param error
     * @return true on success, or false.
     */
    virtual bool preprocess(const std::string& identity, std::string& error)
    {
        return true;
    }

    virtual void postprocess()
    {
    }

    /*virtual*/
    void dispatch(msgpack::rpc::request req)
    {
        try
        {
            // get requested method & identity info
            std::string method_plus;
            req.method().convert(&method_plus);

            std::string identity, method;
            parseMethodPlus(method_plus, identity, method);

            // pre process
            std::string error;
            if (!preprocess(identity, error))
            {
                req.error(error);
                postprocess();
                return;
            }

            if (debug_)
                cout << "#[Worker:"<<srvInfo_.port_<<"] dispatch request: " << method << endl;

            // handle request
            typename Maptype::iterator handler = handlerList_.find(method);
            if (handler != handlerList_.end())
            {
                // process request
                (static_cast<WorkerServer*>(this)->*(handler->second.callback_))(req);
            }
            else
            {
                req.error(msgpack::rpc::NO_METHOD_ERROR);
                std::cerr<<"[WorkerServer] Not found method: "<<method_plus<<std::endl;
            }
        }
        catch (msgpack::type_error& e)
        {
            req.error(msgpack::rpc::ARGUMENT_ERROR);
        }
        catch (std::exception& e)
        {
            req.error(std::string(e.what()));
            std::cerr<<"[WorkerServer] "<<e.what()<<std::endl;
        }

        // post process
        postprocess();
    }

protected:
    void addHandler(const string& method, WorkerHandler<WorkerServer> handler)
    {
        if (handlerList_.find(method) != handlerList_.end())
        {
            throw std::runtime_error("Duplicated function: " + method);
        }

        handlerList_.insert(typename Maptype::value_type(method, handler));
        // handlerList_[method] = serviceHandle;
    }

    /**
     * parse method plus
     * @param method_plus[IN]
     * @param identity[OUT]
     * @param method[OUT]
     */
    void parseMethodPlus(std::string method_plus, std::string& identity, std::string& method)
    {
        size_t pos = method_plus.find(REQUEST_FUNC_DELIMETER);
        if (pos == string::npos)
        {
            method = method_plus;
        }
        else
        {
            identity = method_plus.substr(0,pos);
            if (pos+1 < method_plus.size())
            {
                method = method_plus.substr(pos+1);
            }
        }
    }

protected:
    ServerInfo srvInfo_;
    unsigned int threadNum_;

    Maptype handlerList_;

public:
    bool debug_;

};

}} // end - namespace

#endif /* IZENE_NET_AGGREGATOR_WORKER_SERVER_BASE_H_ */
