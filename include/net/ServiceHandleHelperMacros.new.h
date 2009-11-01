#ifndef NET_SERVICE_HANDLE_HELPER_MACROS_H
#define NET_SERVICE_HANDLE_HELPER_MACROS_H
/**
 * @file net/ServiceHandleHelperMacros.h
 * @author Ian Yang
 * @date Created <2009-10-23 10:30:12>
 * @date Updated <2009-11-02 02:59:52>
 * @brief helper macros to simplify registering services in MFServer.
 *
 * Declaration usage example. functions are listed in adjacent parentheses.
 * \code
 * #include <net/ServiceHandleHelperMacros.h>
 * class MyServiceHandle : public mf::ServiceHandleBase {
 *     MF_SERVICE_HANDLE_LIST(
 *         SIAServiceHandler,
 *         (handleFunc1)
 *         (handleFunc2)
 *         (...)
 *     );
 * };
 * \endcode
 *
 * Implementation example. GET and RETURN must occur in pair
 * \code
 * MF_SERVICE_HANDLE_IMPL(MyServiceHandle, handleFunc1)
 * {
 *     int arg1 = 0;
 *     int arg2 = 0;
 *     int sum = 0;
 *     int diff = 0;
 *     MF_SERVICE_HANDLE_GET((arg1)(arg2));
 *     sum = arg1 + arg2;
 *     diff = arg1 - arg2;
 *     MF_SERVICE_HANDLE_RETURN((sum)(diff));
 * }
 * \endcode
 */
// #include "ServiceHandleBase.h"
// #include "ServiceItem.h"

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/enum_params.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>

/**
 * @param r not used
 * @param data not used
 * @param h handler function
 */
#define MF_SERVICE_HANDLE_add_handler(r, data, h)           \
{                                                           \
    item_type item(&ServiceHandlerType::h);                 \
    if (!server.addService(BOOST_PP_STRINGIZE(h), item))    \
    {                                                       \
        return false;                                       \
    }                                                       \
}

/**
 * @param r not used
 * @param data not used
 * @param h handler function
 */
#define MF_SERVICE_HANDLE_declare(r, data, h)               \
bool h(::messageframework::MessageServer& server,           \
       ::messageframework::ServiceRequestInfoPtr& request);

/**
 * @param ht handler type name
 * @param hs seq of handler functions
 */
#define MF_SERVICE_HANDLE_LIST(ht, hs)                              \
public:                                                             \
    typedef ht ServiceHandlerType;                                  \
    template<typename T>                                            \
    static bool addServicesToServer(T& server)                      \
    {                                                               \
        typedef ::messageframework::ServiceItem<ServiceHandlerType> \
            item_type;                                              \
        BOOST_PP_SEQ_FOR_EACH(                                      \
            MF_SERVICE_HANDLE_add_handler,ht,hs                     \
        )                                                           \
        return true;                                                \
    }                                                               \
    BOOST_PP_SEQ_FOR_EACH(MF_SERVICE_HANDLE_declare,ht,hs)          \
private:                                                            \
    typedef int MFDummyTypedefForSemiColon_

#define MF_SERVICE_HANDLE_IMPL(ht,h)                            \
bool ht::h(::messageframework::MessageServer& server,           \
           ::messageframework::ServiceRequestInfoPtr& request)

#define MF_SERVICE_HANDLE_adjust_buffer_num() if (n==0) n=1;

#define MF_SERVICE_HANDLE_deserialize(r, data, ind, arg)    \
::messageframework::mf_deserialize(arg, request, i + ind);

#define MF_SERVICE_HANDLE_GET(args)                                 \
unsigned int n = request->getBufferNum();                           \
unsigned int step = BOOST_PP_SEQ_SIZE(args);                        \
if (step == 0)                                                      \
{                                                                   \
    if (n == 0)                                                     \
    {                                                               \
        n == 1;                                                     \
    }                                                               \
    step = 1;                                                       \
}                                                                   \
for (unsigned int i = 0; i < n; i += step)                          \
{                                                                   \
    BOOST_PP_SEQ_FOR_EACH_I(MF_SERVICE_HANDLE_deserialize,~,args)   \
    typedef unsigned int MFDummyTypedefForSemiColon_

#define MF_SERVICE_HANDLE_serialize(r, data, ind, arg)          \
::messageframework::mf_serialize(arg, request, i/step + ind);

#define MF_SERVICE_HANDLE_RETURN_true(args) \
}                                           \
return true

#define MF_SERVICE_HANDLE_RETURN_args(args)                     \
    BOOST_PP_SEQ_FOR_EACH_I(MF_SERVICE_HANDLE_serialize,~,args) \
}                                                               \
request->setBufferNum(n/step*BOOST_PP_SEQ_SIZE(args));          \
return server.putResultOfService(request)

#define MF_SERVICE_HANDLE_RETURN(args)          \
BOOST_PP_IF(                                    \
    BOOST_PP_EQUAL(0, BOOST_PP_SEQ_SIZE(args)), \
    MF_SERVICE_HANDLE_RETURN_true,              \
    MF_SERVICE_HANDLE_RETURN_args               \
)(args)


#define MF_SERVICE_HANDLE_declare_var(r, data, ind, arg)    \
arg BOOST_PP_CAT(t, ind);

#define MF_SERVICE_HANDLE_type_to_var(r, base, ind, arg)    \
(BOOST_PP_CAT(t, BOOST_PP_ADD(base, ind)))

#define MF_SERVICE_HANDLE(func, in, out)                        \
BOOST_PP_SEQ_FOR_EACH_I(MF_SERVICE_HANDLE_declare_var,t,in out) \
MF_SERVICE_HANDLE_GET(                                          \
    BOOST_PP_SEQ_FOR_EACH_I(MF_SERVICE_HANDLE_type_to_var,0,in) \
);                                                              \
func(BOOST_PP_ENUM_PARAMS(BOOST_PP_SEQ_SIZE(in out),t));        \
MF_SERVICE_HANDLE_RETURN(                                       \
    BOOST_PP_SEQ_FOR_EACH_I(                                    \
        MF_SERVICE_HANDLE_type_to_var,                          \
        BOOST_PP_SEQ_SIZE(in),                                  \
        out                                                     \
    )                                                           \
)

#define MF_SERVICE_HANDLE_OUT(func, out) MF_SERVICE_HANDLE(func,,out)

#endif // NET_SERVICE_HANDLE_HELPER_MACROS_H
