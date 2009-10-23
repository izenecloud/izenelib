#ifndef NET_SERVICE_HANDLE_HELPER_MACROS_H
#define NET_SERVICE_HANDLE_HELPER_MACROS_H
/**
 * @file net/ServiceHandleHelperMacros.h
 * @author Ian Yang
 * @date Created <2009-10-23 10:30:12>
 * @date Updated <2009-10-23 19:53:29>
 * @brief helper macros to simplify registering services in MFServer.
 *
 * Usage Example:
 * \code
 * #include <net/ServiceHandleHelperMacros.h>
 * namespace mf = messageframework;
 * class MyServiceHandle : public mf::ServiceHandleBase {
 *     MF_SERVICE_HANDLE_LIST(MyServiceHandle);
 *     MF_SERVICE_HANDLE(handleSearch);
 *     MF_SERVICE_HANDLE(handleIndex);
 *     MF_SERVICE_HANDLE_LIST_END();
 * public:
 *     bool handleSearch(mf::MessageServer& server,
 *                       mf::ServiceRequestInfoPtr& request);
 *     bool handleIndex(mf::MessageServer& server,
 *                      mf::ServiceRequestInfoPtr& request);
 * };
 * \endcode
 */
#include "ServiceHandleBase.h"
#include "ServiceItem.h"

#define MF_SERVICE_HANDLE_LIST(HandlerType)                         \
public:                                                             \
    typedef HandlerType ServiceHandlerType;                         \
    template<typename T>                                            \
    static bool addServicesToServer(T& server)                      \
    {                                                               \
        typedef ::messageframework::ServiceItem<ServiceHandlerType> \
            item_type

#define MF_SERVICE_HANDLE(handleFunction)                           \
        do {                                                        \
            item_type item(&ServiceHandlerType::handleFunction);    \
            if (!server.addService(#handleFunction, item))          \
            {                                                       \
                return false;                                       \
            }                                                       \
        } while (false)

#define MF_SERVICE_HANDLE_LIST_END()            \
        return true;                            \
    }                                           \
private:                                        \
    typedef int MFDummyTypedefForSemiColon_

#endif // NET_SERVICE_HANDLE_HELPER_MACROS_H
