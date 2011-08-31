/**
 * @file JobInfo.h
 * @author Zhongxia Li
 * @date Jun 24, 2011
 * @brief 
 */
#ifndef JOB_INFO_H_
#define JOB_INFO_H_

#include <string>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/rpc/session_pool.h>
#include <3rdparty/msgpack/rpc/request.h>

namespace net{
namespace aggregator{

/// types
typedef uint32_t workerid_t;
typedef unsigned int timeout_t;
typedef msgpack::rpc::request JobRequest;
typedef msgpack::rpc::session_pool session_pool_t;
typedef msgpack::rpc::session session_t;
typedef msgpack::rpc::future future_t;

/// constants
const static char REQUEST_FUNC_DELIMETER = '#';
const static workerid_t LOCAL_WORKER_ID = 0;
const static workerid_t REMOTE_WORKER_ID_START = LOCAL_WORKER_ID + 1;
const static timeout_t DEFAULT_TIME_OUT = 8; // seconds

/// WorkerCaller performs in-procedure call to local worker,
/// pass a suitable class type as invoker.
template <typename Invoker>
class WorkerCaller
{
public:
    typedef void(Invoker::*func_t)(void);

public:
    void setInvoker(Invoker* invoker)
    {
        invoker_ = invoker;
    }

    void addMethod(const std::string& func_name, func_t func)
    {
        // func_name should be unique
        funcMap_.insert(std::make_pair(func_name, func));
    }

    template <typename RequestType, typename ResultType>
    bool call(
            const std::string& func_name,
            const RequestType& request,
            ResultType& result,
            std::string& error)
    {
        typedef bool(Invoker::*real_func_t)(const RequestType& , ResultType&);

        cout << "WorkerCaller::call "<<func_name<<endl;

        if (!invoker_)
        {
            cout << "WorkerCaller: empty invoker!"<<endl;
            return false;
        }

        if (funcMap_.find(func_name) != funcMap_.end())
        {
            real_func_t func = (real_func_t) funcMap_[func_name];
            return (invoker_->*func)(request, result);
        }
        else
        {
            cout << "WorkerCaller: not found method "<<func_name<<endl;
            return false;
        }
    }

    template <typename RequestType0, typename RequestType1, typename ResultType>
    bool call(
            const std::string& func_name,
            RequestType0& request0,
            RequestType1& request1,
            ResultType& result,
            std::string& error)
    {
        typedef bool(Invoker::*real_func_t)(RequestType0&, RequestType1&, ResultType&);

        cout << "WorkerCaller::call "<<func_name<<endl;

        if (!invoker_)
        {
            cout << "WorkerCaller: empty invoker!"<<endl;
            return false;
        }

        if (funcMap_.find(func_name) != funcMap_.end())
        {
            real_func_t func = (real_func_t) funcMap_[func_name];
            return (invoker_->*func)(request0, request1, result);
        }
        else
        {
            cout << "WorkerCaller: not found method "<<func_name<<endl;
            return false;
        }
    }

private:
    std::map<std::string, func_t> funcMap_;
    Invoker* invoker_;
};

class EmptyInvoker {};
typedef WorkerCaller<EmptyInvoker> MockWorkerCaller;

#define ADD_WORKER_CALLER_METHOD(WorkerCallerType, WorkerCallerObj, InvokerType, Method) \
{                                                                                         \
    WorkerCallerType::func_t func = (WorkerCallerType::func_t) &InvokerType::Method;      \
    WorkerCallerObj->addMethod(#Method,  func);                                         \
}

/// server info
struct ServerInfo
{
    std::string host_;
    uint16_t port_;

    ServerInfo(const std::string& host, uint16_t port)
    :host_(host), port_(port)
    {
    }

    ServerInfo(){}
};

template <typename RequestType, typename ResultType>
class RequestGroup
{
public:
    typedef boost::shared_ptr<RequestType> RequestParamPtr;
    typedef boost::shared_ptr<ResultType> ResultParamPtr;
    typedef typename std::vector<std::pair<workerid_t, RequestParamPtr> >::iterator request_iterator_t;

    //std::vector<std::pair<workerid_t, RequestParamPtr> > requestList_;

    std::vector<workerid_t> workeridList_;
    std::vector<RequestType*> requestList_;
    std::vector<ResultType*> resultList_;
    ResultType* resultItem_; // to which all sub results will be merged. xxx

    void addRequest(workerid_t workerid, RequestType* request, ResultType* result = NULL)
    {
        //requestList_.push_back(std::make_pair(workerid, request));
        workeridList_.push_back(workerid);
        requestList_.push_back(request);
        resultList_.push_back(result);
    }

    // remove, xxx
    void setResultItemForMerging(ResultType* result)
    {
        resultItem_ = result;
    }

    bool check()
    {
        if (requestList_.size() > 0 && resultItem_)
            return true;

        return false;
    }
};


/**
 * A worker session is a client of a worker server.
 */
class WorkerSession
{
    workerid_t workerId_;
    ServerInfo workerSrv_;

public:
    WorkerSession(const std::string& host, uint16_t port, const workerid_t workerId)
    : workerId_(workerId)
    , workerSrv_(host, port)
    {}

    /**
     *
     * @param sessionPool session pool shared for the worker session.
     * one session pool should be shared with all worker sessions for one request.
     * @param func remote function
     * @param param parameter for remote function
     * @param result [INOUT]
     * @param sec session timeout in seconds
     * @return future delayed reply.
     */
    template <typename RequestType, typename ResultType>
    msgpack::rpc::future sendRequest(
            session_pool_t& sessionPool,
            const std::string& func, const RequestType& param, const ResultType& result, unsigned int sec)
    {
        msgpack::rpc::session session =
                sessionPool.get_session(workerSrv_.host_, workerSrv_.port_);

        // timeout is set for session, i.e., if send 2 or more requests through the same session, the time for timeout
        // is the total time for processing these 2 or more requests, but not for processing each request respectively.
        // Because the sessions for a worker got from a same session pool are the same session, each request should share
        // a different session pool.
        session.set_timeout(sec);
        return session.call(func, param, result);
    }

    workerid_t getWorkerId() const
    {
        return workerId_;
    }

    const ServerInfo& getServerInfo() const
    {
        return workerSrv_;
    }
};

typedef boost::shared_ptr<WorkerSession> WorkerSessionPtr;


///**
// * A future object, returned immediately after sent a request through a worker session,
// * is used to get result from the worker in the future.
// */
//class WorkerFuture
//{
//    workerid_t workerId_;
//    ServerInfo workerSrv_;
//
//    future_t future_;
//
//    bool state_;
//    std::string error_;
//
//public:
//    WorkerFuture(
//            const workerid_t workerid,
//            const ServerInfo workerSrv,
//            const future_t& future)
//    : workerId_(workerid)
//    , workerSrv_(workerSrv)
//    , future_(future)
//    , state_(true)
//    , error_("fine")
//    {
//
//    }
//
//    template <typename ResultType>
//    ResultType getResult()
//    {
//        return future_.get<ResultType>();
//    }
//
//    workerid_t getWorkerId() const
//    {
//        return workerId_;
//    }
//
//    const ServerInfo& getServerInfo() const
//    {
//        return workerSrv_;
//    }
//
//    bool getState()
//    {
//        return state_;
//    }
//
//    void setError(const std::string& err, bool state=false)
//    {
//        state_ = state;
//        error_ = err;
//    }
//
//    const std::string& getError() const
//    {
//        return error_;
//    }
//};
//
///**
// * Manage all worker futures returned by worker sessions for one request.
// * A session pool will be shared for all worker sessions in one request, so that their can be processed concurrently.
// */
//class WorkerFutureHolder
//{
//    std::vector<WorkerFuture> futureList_;
//    session_pool_t sessionPool_;
//
//public:
//    void clear()
//    {
//        futureList_.clear();
//    }
//
//    session_pool_t& getSessionPool()
//    {
//        return sessionPool_;
//    }
//
//
//    void addWorkerFuture(const WorkerFuture& workerFuture)
//    {
//        futureList_.push_back(workerFuture);
//    }
//
//    std::vector<WorkerFuture>& getFutureList()
//    {
//        return futureList_;
//    }
//};
//
//typedef boost::shared_ptr<WorkerFutureHolder> WorkerFutureHolderPtr;

}} // end - namespace

#endif /* JOB_INFO_H_ */
