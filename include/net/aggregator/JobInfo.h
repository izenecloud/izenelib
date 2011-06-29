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
    ServerInfo workerSrv_;

    msgpack::rpc::client client_;
    msgpack::rpc::future reply_;

    bool state_;
    std::string error_;

public:
    WorkerSession(const std::string& host, uint16_t port)
    : workerSrv_(host, port)
    , client_(host, port)
    , state_(true)
    , error_()
    {}

    void setTimeOut(unsigned int sec)
    {
        client_.set_timeout(sec);
    }

    template <typename RequestType>
    void sendRequest(const std::string& func, const RequestType& param)
    {
        state_ = true;
        reply_ = client_.call(func, param);
    }

    template <typename ResultType>
    ResultType getResult()
    {
        return reply_.get<ResultType>();
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

    const ServerInfo& getServerInfo() const
    {
        return workerSrv_;
    }
};

typedef boost::shared_ptr<WorkerSession> WorkerSessionPtr;

}} // end - namespace

#endif /* JOB_INFO_H_ */
