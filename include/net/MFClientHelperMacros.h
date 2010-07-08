#ifndef INCLUDE_NET_MFCLIENT_HELPER_MACROS_H
#define INCLUDE_NET_MFCLIENT_HELPER_MACROS_H
/**
 * @file include/net/MFClientHelperMacros.h
 * @author Ian Yang
 * @date Created <2010-07-08 11:23:24>
 */

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/enum_params.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>

#define MF_CLIENT_IMPL_CONNECT(agentInfo, serviceName) \
const std::string kMFClientImplAgentInfo(agentInfo); \
const std::string kMFClientImplServiceName(serviceName)

#define MF_CLIENT_IMPL_SET_serialize(r, req, ind, arg) \
::message_framework::mf_serialize(arg, req, ind);

#define MF_CLIENT_IMPL_SET(args) \
::messageframework::ServiceRequestInfoPtr req( \
    new ::messageframework::ServiceRequestInfo \
); \
req->setServiceName(kMFClientImplServiceName); \
BOOST_PP_SEQ_FOR_EACH_I(MF_CLIENT_IMPL_SET_serialize,req,args) \

#define MF_CLIENT_IMPL_RETURN_deserialize(r, res, ind, arg) \
::messageframework::mf_deserialize(arg, res, ind);

#define MF_CLIENT_IMPL_RETURN_res(args) \
ServiceResultPtr res; \
if (requestService(kMFClientImplAgentInfo, kMFClientImplServiceName, req, res)) \
{ \
    BOOST_PP_SEQ_FOR_EACH_I(MF_CLIENT_IMPL_RETURN_deserialize,res,args) \
    return true; \
} \
return false

#define MF_CLIENT_IMPL_RETURN_nores(args) \
return requestService(kMFClientImplAgentInfo, kMFClientImplServiceName, req)

#define MF_CLIENT_IMPL_RETURN(args) \
BOOST_PP_IF( \
    BOOST_PP_EQUAL(0, BOOST_PP_SEQ_SIZE(args)), \
    MF_CLIENT_IMPL_RETURN_nores, \
    MF_CLIENT_IMPL_RETURN_res \
)(args)

#endif // INCLUDE_NET_MFCLIENT_HELPER_MACROS_H
