#ifndef IZENELIB_DRIVER_ACTION_HANDLER_H
#define IZENELIB_DRIVER_ACTION_HANDLER_H
/**
 * @file izenelib/driver/ActionHandler.h
 * @author Ian Yang
 * @date Created <2010-06-18 18:51:48>
 * @brief Context for action handler
 */
#include "Request.h"
#include "Response.h"
#include "Poller.h"

namespace izenelib {
namespace driver {

class ActionHandlerBase
{
public:
    virtual ~ActionHandlerBase()
    {}

    virtual void invoke(const std::string& controller, const std::string& action, Request& request, Response& response, Poller poller) = 0;
};


template<typename ControllerType>
class ActionHandler : public ActionHandlerBase
{
    typedef void (ControllerType::*handler_type)();

public:
    ActionHandler(const ControllerType& prototype,
                  handler_type handler)
    : prototype_(prototype), handler_(handler)
    {}

    void invoke(const std::string& controller_name, const std::string& action, Request& request, Response& response, Poller poller)
    {
        ControllerType controller(prototype_);
        controller.initializeRequestContext(controller_name, action, request, response);
        controller.setPoller(poller);
        if (controller.preprocess())
        {
            (controller.*handler_)();
            controller.postprocess();
        }
    }

private:
    ControllerType prototype_;
    handler_type handler_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_ACTION_HANDLER_H
