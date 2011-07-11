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

/// job request type
typedef msgpack::rpc::request JobRequest;
typedef unsigned int workerid_t;
typedef msgpack::rpc::session_pool session_pool_t;
typedef msgpack::rpc::future future_t;

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

/**
 * A connection to a worker.
 */
class WorkerSession
{
    workerid_t workerId_;   // unique id for each worker server
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
     * @param sec session timeout in seconds
     * @return future delayed reply.
     */
    template <typename RequestType>
    msgpack::rpc::future sendRequest(
            session_pool_t& sessionPool,
            const std::string& func, const RequestType& param, unsigned int sec)
    {
        msgpack::rpc::session session =
                sessionPool.get_session(workerSrv_.host_, workerSrv_.port_);

        // timeout is set for session, i.e., if send 2 or more requests through the same session, the time for timeout
        // is the total time for processing these 2 or more requests, but not for processing each request respectively.
        // For the sessions for a worker got from a same session pool are the same session, each request should share
        // a different session pool.
        session.set_timeout(sec);
        return session.call(func, param);
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

/**
 * A future object, returned immediately after sent a request through a worker session,
 * is used to get result from the worker in the future.
 */
class WorkerFuture
{
    workerid_t workerId_;
    ServerInfo workerSrv_;

    future_t future_;

    bool state_;
    std::string error_;

public:
    WorkerFuture(
            const workerid_t workerid,
            const ServerInfo workerSrv,
            const future_t& future)
    : workerId_(workerid)
    , workerSrv_(workerSrv)
    , future_(future)
    , state_(true)
    , error_("fine")
    {

    }

    template <typename ResultType>
    ResultType getResult()
    {
        return future_.get<ResultType>();
    }

    workerid_t getWorkerId() const
    {
        return workerId_;
    }

    const ServerInfo& getServerInfo() const
    {
        return workerSrv_;
    }

    bool getState()
    {
        return state_;
    }

    void setError(const std::string& err, bool state=false)
    {
        state_ = state;
        error_ = err;
    }

    const std::string& getError() const
    {
        return error_;
    }
};

/**
 * Manage all worker futures returned by worker sessions for one request.
 * A session pool will be shared for all worker sessions in one request, so that their can be processed concurrently.
 */
class WorkerFutureHolder
{
    std::vector<WorkerFuture> futureList_; // pipeline
    session_pool_t sessionPool_;

public:
    void clear()
    {
        futureList_.clear();
    }

    session_pool_t& getSessionPool()
    {
        return sessionPool_;
    }


    void addWorkerFuture(const WorkerFuture& workerFuture)
    {
        futureList_.push_back(workerFuture);
    }

    std::vector<WorkerFuture>& getFutureList()
    {
        return futureList_;
    }
};

}} // end - namespace

#endif /* JOB_INFO_H_ */
