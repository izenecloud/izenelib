/**
 * @file Typedef.h
 * @author Zhongxia Li
 * @date Jun 24, 2011
 * @brief 
 */
#ifndef IZENELIB_NET_AGGREGATOR_TYPEDEF_H_
#define IZENELIB_NET_AGGREGATOR_TYPEDEF_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <3rdparty/msgpack/rpc/session_pool.h>

namespace net{
namespace aggregator{

/// types
typedef uint32_t workerid_t;
typedef unsigned int timeout_t;

/// constants
const static timeout_t DEFAULT_TIME_OUT = 30; // seconds

struct ServerInfo
{
    std::string host_;
    uint16_t port_;

    ServerInfo(const std::string& host, uint16_t port)
    :host_(host), port_(port)
    {
    }
};

struct WorkerServerInfo : public ServerInfo
{
    workerid_t workerid_;
    bool isLocal_;

    WorkerServerInfo(const std::string& host, uint16_t port, uint32_t workerid, bool isLocal=false)
    : ServerInfo(host, port), workerid_(workerid), isLocal_(isLocal)
    {
    }
};

template <typename RequestType, typename ResultType>
class RequestGroup
{
public:
    typedef boost::shared_ptr<RequestType> RequestParamPtr;
    typedef boost::shared_ptr<ResultType> ResultParamPtr;

    std::vector<workerid_t> workeridList_;
    std::vector<RequestType*> requestList_;
    std::vector<ResultType*> resultList_;

    void addRequest(workerid_t workerid, RequestType* request, ResultType* result = NULL)
    {
        workeridList_.push_back(workerid);
        requestList_.push_back(request);
        resultList_.push_back(result);
    }

    bool check()
    {
        if (requestList_.size() > 0)
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
            msgpack::rpc::session_pool& sessionPool,
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


}} // end - namespace

#endif /* IZENELIB_NET_AGGREGATOR_TYPEDEF_H_ */
