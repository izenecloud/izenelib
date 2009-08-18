/**
 * @file    ServiceItem.h
 * @brief   header file of ServiceItem class
 * @author  Dohyun Yun
 * @date    2009-03-24
 * @details
 *  - Log
 */

#include <net/message_framework.h>
#include <vector>

#ifndef _SERVICE_ITEM_H_
#define _SERVICE_ITEM_H_

namespace messageframework{

/// @brief a class which contain a service information of configuration manager.
template <typename ServiceHandler>
class ServiceItem
{
    public:     

        /// @brief call back function which is called when proper service is requested.
        /// @param server message server of configuration manager.
        /// @param request Service request information class.
        bool (ServiceHandler::*callback_)(MessageServer& server, ServiceRequestInfoPtr& request);

    public:
        ServiceItem(){};

}; // end - class ConfigServiceItem

}

#endif  //_SERVICE_ITEM_H_

//eof

