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

#include "AggregatorConfig.h"

#include <boost/shared_ptr.hpp>
#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/rpc/request.h>

namespace net{
namespace aggregator{

typedef msgpack::rpc::request JobRequest;


/**
 * A connection to a worker.
 */
struct WorkerSession
{
    ServerInfo workerSrv_;

    msgpack::rpc::client client_;
    msgpack::rpc::future reply_;

    WorkerSession(const std::string& host, uint16_t port)
    : workerSrv_(host, port)
    , client_(host, port)
    {}

    void setTimeOut(unsigned int sec)
    {
        client_.set_timeout(sec);
    }

    template <typename RequestType>
    void sendRequest(const std::string& func, const RequestType& param)
    {
        reply_ = client_.call(func, param);
    }

    template <typename ResultType>
    ResultType getResult()
    {
        return reply_.get<ResultType>();
    }
};

typedef boost::shared_ptr<WorkerSession> WorkerSessionPtr;

}} // end - namespace

#endif /* JOB_INFO_H_ */
