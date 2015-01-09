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
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace izenelib {
namespace driver {

class ActionHandlerBase
{
public:
    typedef boost::function<void()> callback_t;
    virtual ~ActionHandlerBase()
    {}

    virtual void invoke(Request& request, Response& response, Poller poller) = 0;
    virtual void invoke_async(Request& request, Response& response, Poller poller, callback_t cb) = 0;
    virtual void invoke_raw(Request& request, Response& response,
        std::string& raw_request, std::string& raw_response,
        Poller poller, const std::string& path, int method) = 0;
    virtual void invoke_raw_async(Request& request, Response& response,
        std::string& raw_request, std::string& raw_response,
        Poller poller, callback_t cb, const std::string& path, int method) = 0;
    virtual bool is_async() const = 0;
};


template<typename ControllerType>
class ActionHandler : public ActionHandlerBase
{
    typedef void (ControllerType::*handler_type)();

public:

    ActionHandler(const ControllerType& prototype,
                  handler_type handler, bool is_async = false)
    : prototype_(prototype), handler_(handler), is_async_(is_async)
    {}

    bool is_async() const
    {
        return is_async_;
    }

    void invoke(Request& request, Response& response, Poller poller)
    {
        ControllerType controller(prototype_);
        controller.initializeRequestContext(request, response);
        controller.setPoller(poller);
        if (controller.preprocess())
        {
            (controller.*handler_)();
            controller.postprocess();
        }
    }

    void invoke_async(Request& request, Response& response, Poller poller, callback_t cb)
    {
        boost::shared_ptr<ControllerType> controller(new ControllerType(prototype_));
        controller->initializeRequestContext(request, response);
        controller->setPoller(poller);
        controller->set_callback(cb);
        if (controller->preprocess())
        {
            ((*controller).*handler_)();
            // in async call, the postprocess should be called by handler
            // after it finished in async.
            //controller.postprocess();
        }
    }

    void invoke_raw(Request& request, Response& response,
        std::string& raw_request, std::string& raw_response,
        Poller poller, const std::string& path, int method)
    {
        ControllerType controller(prototype_);
        controller.initializeRequestContext(request, response);
        controller.initializeRawContext(raw_request, raw_response);
        controller.setPoller(poller);
        controller.set_path(path);
        controller.set_method(method);
        if (controller.preprocess())
        {
            (controller.*handler_)();
            controller.postprocess();
        }
    }

    void invoke_raw_async(Request& request, Response& response,
        std::string& raw_request, std::string& raw_response,
        Poller poller, callback_t cb, const std::string& path, int method)
    {
        boost::shared_ptr<ControllerType> controller(new ControllerType(prototype_));
        controller->initializeRequestContext(request, response);
        controller->initializeRawContext(raw_request, raw_response);
        controller->setPoller(poller);
        controller->set_callback(cb);
        controller->set_path(path);
        controller->set_method(method);
        if (controller->preprocess())
        {
            ((*controller).*handler_)();
        }
    }


private:
    ControllerType prototype_;
    handler_type handler_;
    bool is_async_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_ACTION_HANDLER_H
