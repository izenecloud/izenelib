/**
 * @file WorkerHandler.h
 * @author Zhongxia Li
 * @date Jun 27, 2011
 * @brief 
 */
#ifndef WORKER_HANDLER_H_
#define WORKER_HANDLER_H_

namespace net{
namespace aggregator{

template <typename ConcreteWorker>
struct WorkerHandler
{
    typedef bool (ConcreteWorker::*callback_type) (
                JobRequest&
            );

    callback_type callback_;

    WorkerHandler(callback_type callback)
    : callback_(callback)
    {}
};

}} // end - namespace

#endif /* WORKER_HANDLER_H_ */
