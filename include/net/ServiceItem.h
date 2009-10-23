/**
 * @file    ServiceItem.h
 * @brief   header file of ServiceItem class
 * @author  Dohyun Yun
 * @date    2009-03-24
 * @details
 *  - Log
 */

#ifndef _SERVICE_ITEM_H_
#define _SERVICE_ITEM_H_

namespace messageframework {

/// @brief a class which contain a service information of configuration manager.
template <typename ServiceHandler>
struct ServiceItem
{
    /// @brief call back invoked when proper service is requested.
    /// @param server message server of configuration manager.
    /// @param request Service request information class.
    typedef bool (ServiceHandler::*callback_type)(
        MessageServer& /*server*/,
        ServiceRequestInfoPtr& /*request*/
    );

    callback_type callback_;

    explicit ServiceItem(callback_type c = 0)
    : callback_(c)
    {}

}; // end class ServiceItem

}

#endif  //_SERVICE_ITEM_H_

