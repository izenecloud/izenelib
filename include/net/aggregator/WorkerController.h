/**
 * @file WorkerController.h
 * @brief a base class for ControllerType in WorkerHandler
 * @author Zhongxia Li, Jun Jiang
 * @date 2012-03-20
 */

#ifndef IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_
#define IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_

#include "WorkerRouter.h"
#include "WorkerHandler.h"
#include <3rdparty/msgpack/rpc/request.h>

#include <string>
#include <memory>

namespace net{
namespace aggregator{

class WorkerController
{
public:
    virtual ~WorkerController() {}

    virtual bool preprocess(msgpack::rpc::request& req, std::string& error) { return true; }

    virtual bool addWorkerHandler(WorkerRouter& router) { return true; }
};

/**
 * below is an example to define @c addWorkerHandler() in @c WorkerController subclass:
 * <code>
 * class ControllerExample : public WorkerController
 * {
 * public:
 * virtual bool addWorkerHandler(WorkerRouter& router)
 * {
 *   ADD_WORKER_HANDLER_BEGIN(ControllerExample, router)
 *   ADD_WORKER_HANDLER(add)
 *   ADD_WORKER_HANDLER_END()
 * }
 *
 * WORKER_CONTROLLER_METHOD_2(add, searchWorker_->add, AddRequest, AddResult)
 * ...
 * </code>
 */

#define ADD_WORKER_HANDLER_BEGIN(ConcreteWorkerController, RouterInstance)  \
    typedef ConcreteWorkerController WorkerControllerType;                  \
    net::aggregator::WorkerRouter& _router = RouterInstance;

#define ADD_WORKER_HANDLER(Method)                                                  \
    {                                                                               \
        typedef net::aggregator::WorkerHandler<WorkerControllerType> handler_type;  \
        std::unique_ptr<handler_type> ptr(new handler_type(                           \
            *this, &WorkerControllerType::Method));                                 \
        if (!_router.add(#Method, ptr.get()))                                       \
            return false;                                                           \
        ptr.release();                                                              \
    }

#define ADD_WORKER_HANDLER_END()    \
    return true;

/**
 * in @c WorkerController subclass, use below macros to define method of signature:
 * <code> void method(msgpack::rpc::request& req) </code>
 *
 * please note that, as @c Aggregator::distributeRequest() sets the "identity" value
 * as @c req.params[0], the method parameters would start from @c req.params()[1].
 */

#define WORKER_CONTROLLER_METHOD_1(Method, Call, Out)   \
void Method(msgpack::rpc::request& req)                 \
{                                                       \
    msgpack::type::tuple<msgpack::object, Out> params;  \
    req.params().convert(&params);                      \
    Out& out = params.get<1>();                         \
    Call(out);                                          \
    req.result(out);                                    \
}

#define WORKER_CONTROLLER_METHOD_2(Method, Call, In, Out)   \
void Method(msgpack::rpc::request& req)                     \
{                                                           \
    msgpack::type::tuple<msgpack::object, In, Out> params;  \
    req.params().convert(&params);                          \
    In& in = params.get<1>();                               \
    Out& out = params.get<2>();                             \
    Call(in, out);                                          \
    req.result(out);                                        \
}

#define WORKER_CONTROLLER_METHOD_3(Method, Call, In1, In2, Out)     \
void Method(msgpack::rpc::request& req)                             \
{                                                                   \
    msgpack::type::tuple<msgpack::object, In1, In2, Out> params;    \
    req.params().convert(&params);                                  \
    In1& in1 = params.get<1>();                                     \
    In2& in2 = params.get<2>();                                     \
    Out& out = params.get<3>();                                     \
    Call(in1, in2, out);                                            \
    req.result(out);                                                \
}

}} // end - namespace

#endif // IZENE_NET_AGGREGATOR_WORKER_CONTROLLER_H_
