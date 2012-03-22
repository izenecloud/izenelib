/**
 * @file WorkerHandler.h
 * @brief a handler invoking worker methods
 * @author Jun Jiang
 * @date 2012-03-20
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_
#define IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_

#include <string>
#include <3rdparty/msgpack/rpc/request.h>

namespace net{
namespace aggregator{

class WorkerHandlerBase
{
public:
    virtual ~WorkerHandlerBase() {}

    virtual void invoke(msgpack::rpc::request& req) = 0;
};

template <typename ControllerType>
class WorkerHandler : public WorkerHandlerBase
{
    typedef void (ControllerType::*callback_type)(msgpack::rpc::request& req);

public:
    WorkerHandler(const ControllerType& controller, callback_type callback)
    : controller_(controller)
    , callback_(callback)
    {}

    virtual void invoke(msgpack::rpc::request& req)
    {
        ControllerType controller(controller_);

        std::string error;
        if (controller.preprocess(req, error))
        {
            (controller.*callback_)(req);
        }
        else
        {
            req.error(error);
        }
    }

private:
    ControllerType controller_;
    callback_type callback_;
};

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_HANDLER_H_
