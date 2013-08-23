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
#include <boost/threadpool.hpp>

namespace net{
namespace aggregator{

class WorkerRouter;

class WorkerServerThreadPool
{
public:
    typedef boost::shared_ptr<boost::threadpool::pool> threadpool_ptr;
    void init(size_t poolsize);
    void schedule_task(const boost::threadpool::pool::task_type& task);
    void stop();

private:
    threadpool_ptr pool_;
};

class WorkerServer : public msgpack::rpc::server::base
{
public:
    WorkerServer(
        const WorkerRouter& router,
        const std::string& host,
        uint16_t port,
        unsigned int threadNum = 30);

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
    WorkerServerThreadPool  worker_pool_;
};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_SERVER_H_
