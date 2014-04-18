#include <net/aggregator/WorkerServer.h>
#include <net/aggregator/WorkerRouter.h>
#include <net/aggregator/WorkerHandler.h>

#include <stdexcept>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace net{
namespace aggregator{

void WorkerServerThreadPool::init(size_t poolsize)
{
    if (pool_)
        return;
    if (poolsize == 0)
        poolsize = 1;
    pool_.reset(new boost::threadpool::pool(poolsize));
}

void WorkerServerThreadPool::schedule_task(const boost::threadpool::pool::task_type& task)
{
    //if (pool_->active() == 0)
    //{
    //    std::cout << "currently no active in rpc worker server.========= " << std::endl;
    //}
    pool_->schedule(task);
    //if (pool_->pending() > 1)
    //{
    //    std::cout << "some task pending in rpc worker server: " << pool_->pending() << std::endl;
    //}
}

void WorkerServerThreadPool::stop()
{
    if (pool_)
    {
        pool_->clear();
        pool_->wait();
    }
}

WorkerServer::WorkerServer(
    const WorkerRouter& router,
    const std::string& host,
    uint16_t port,
    unsigned int threadNum)
: router_(router)
, srvInfo_(host, port)
, threadNum_(threadNum)
{
    if (threadNum_ < 2)
        threadNum_ = 2;
    worker_pool_.init(threadNum_);
}

WorkerServer::~WorkerServer()
{
    stop();
}

void WorkerServer::start()
{
    if (srvInfo_.host_.empty() || srvInfo_.port_ <= 0)
        throw std::runtime_error("invalid host or port for Worker server!");

    instance.listen(srvInfo_.host_, srvInfo_.port_);
    instance.start(threadNum_/2);
}

void WorkerServer::stop()
{
    if (instance.is_running())
    {
        instance.end();
        instance.join();
    }
    worker_pool_.stop();
}

void WorkerServer::join()
{
    instance.join();
}

void handler_wrapper(WorkerHandlerBase* handler, msgpack::rpc::request& req)
{
    try
    {
        handler->invoke(req);
    }
    catch (const msgpack::type_error& e)
    {
        std::cerr << "[WorkerServer] " << e.what() << std::endl;
        req.error(msgpack::rpc::ARGUMENT_ERROR);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[WorkerServer] " << e.what() << std::endl;
        req.error(std::string(e.what()));
    }
}

void WorkerServer::dispatch(msgpack::rpc::request req)
{
    try
    {
        std::string method;
        req.method().convert(&method);

        WorkerHandlerBase* handler = router_.find(method);
        if (handler)
        {
            //handler->invoke(req);
            worker_pool_.schedule_task(boost::bind(handler_wrapper, handler, req));
        }
        else
        {
            std::cerr << "[WorkerServer] Method not found: " << method << std::endl;
            req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
    }
    catch (const msgpack::type_error& e)
    {
        std::cerr << "[WorkerServer] " << e.what() << std::endl;
        req.error(msgpack::rpc::ARGUMENT_ERROR);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[WorkerServer] " << e.what() << std::endl;
        req.error(std::string(e.what()));
    }
}

}} // end - namespace
