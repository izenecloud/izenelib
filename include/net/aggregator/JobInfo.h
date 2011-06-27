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

#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/request.h>

namespace net{
namespace aggregator{

typedef msgpack::rpc::request JobRequest;


//template <typename ParamType>
//struct JobRequest
//{
//    JobRequest(ParamType& obj)
//    : paramObj_(obj)
//    {}
//
//    ParamType paramObj_;
//
//    MSGPACK_DEFINE(paramObj_);
//};
//
//template <typename ParamType>
//struct JobResult
//{
//    JobResult(ParamType& obj)
//    : paramObj_(obj)
//    {}
//
//    ParamType paramObj_;
//
//    MSGPACK_DEFINE(paramObj_);
//};


}} // end - namespace

#endif /* JOB_INFO_H_ */
