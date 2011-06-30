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
#include <3rdparty/msgpack/rpc/request.h>

namespace net{
namespace aggregator{

/// job request type
typedef msgpack::rpc::request JobRequest;
typedef unsigned int workerid_t;
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

    msgpack::rpc::client client_;

public:
    WorkerSession(const std::string& host, uint16_t port, const workerid_t workerId)
    : workerId_(workerId)
    , workerSrv_(host, port)
    , client_(host, port)
    {}

    void setTimeOut(unsigned int sec)
    {
        client_.set_timeout(sec);
    }

    template <typename RequestType>
    msgpack::rpc::future sendRequest(const std::string& func, const RequestType& param)
    {
        return client_.call(func, param);
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
 * worker future reply
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
 * manage woker futures for 1 request
 */
class WorkerFutureHolder
{
    std::vector<WorkerFuture> futureList_;
public:
    void clear()
    {
        futureList_.clear();
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
