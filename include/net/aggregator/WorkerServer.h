/**
 * @file WorkerServer.h
 * @brief an rpc server running on worker node, it handles requests from master node
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-21
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_SERVER_H_
#define IZENE_NET_AGGREGATOR_WORKER_SERVER_H_

#include "Typedef.h"
#include <3rdparty/msgpack/rpc/server.h>

namespace net{
namespace aggregator{

class WorkerRouter;

class WorkerServer : public msgpack::rpc::server::base
{
public:
    WorkerServer(
        const WorkerRouter& router,
        const std::string& host,
        uint16_t port,
        unsigned int threadNum = 4);

    ~WorkerServer();

    void start();
    void stop();
    void join();

    const ServerInfo& getServerInfo() const { return srvInfo_; }

    virtual void dispatch(msgpack::rpc::request req);

protected:
    const WorkerRouter& router_;

    ServerInfo srvInfo_;
    unsigned int threadNum_;
};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_SERVER_H_
