#include <net/aggregator/WorkerServer.h>
#include <net/aggregator/WorkerRouter.h>
#include <net/aggregator/WorkerHandler.h>

#include <stdexcept>
#include <iostream>

namespace net{
namespace aggregator{

WorkerServer::WorkerServer(
    const WorkerRouter& router,
    const std::string& host,
    uint16_t port,
    unsigned int threadNum)
: router_(router)
, srvInfo_(host, port)
, threadNum_(threadNum)
{
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
    instance.start(threadNum_);
}

void WorkerServer::stop()
{
    if (instance.is_running())
    {
        instance.end();
        instance.join();
    }
}

void WorkerServer::join()
{
    instance.join();
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
            handler->invoke(req);
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
