/**
 * @file util/driver/DriverConnection.cpp
 * @author Ian Yang
 * @date Created <2010-05-26 16:12:07>
 */
#include <util/driver/DriverConnection.h>

#include <util/ClockTimer.h>

#include <util/driver/writers/JsonWriter.h>
#include <util/driver/readers/JsonReader.h>
#include <util/driver/Keys.h>

#include <boost/bind.hpp>
#include <iostream>

using namespace boost::asio;

namespace izenelib {
namespace driver {

DriverConnection::DriverConnection(
    io_service& ioService,
    const router_ptr& router,
    firewall_type firewall
)
: socket_(ioService),
  poller_(socket_),
  router_(router),
  nextContext_(),
  firewall_(firewall),
  reader_(new driver::JsonReader()),
  writer_(new driver::JsonWriter())
{
    // Hard code Json reader and writer now.
    // TODO: In future, we can support select reader and writer according to
    // request paramenters such as "content-type" and "accept"
}

void DriverConnection::start()
{
    // Firwall is setup and failed to pass the check
    if (firewall_ && !firewall_(socket_.remote_endpoint().address()))
    {
        context_ptr context = createContext();
        context->setSequence(0);
        asyncWriteError(context, "Access denied.");
    }
    else
    {
        asyncReadFormHeader();
    }
}

void DriverConnection::asyncReadFormHeader()
{
    boost::asio::async_read(
        socket_,
        nextContext_.formHeaderBuffer(),
        boost::bind(
            &DriverConnection::afterReadFormHeader,
            shared_from_this(),
            boost::asio::placeholders::error
        )
    );
}

void DriverConnection::afterReadFormHeader(const boost::system::error_code& e)
{
    // start timer
    nextContext_.serverTimer.restart();

    if (!e)
    {
        if (nextContext_.sequence() == 0 ||
            nextContext_.formPayloadSize() == 0)
        {
            // header size 0 indicates eof
            shutdownReceive();
        }
        else if (nextContext_.formPayloadSize() > kLimitSize)
        {
            context_ptr context = createContext();
            asyncWriteError(context, "Size exceeds limit.");
            shutdownReceive();
        }
        else
        {
            // allocate enough space to receive payload
            nextContext_.formPayload.resize(nextContext_.formPayloadSize());
            asyncReadFormPayload();
        }
    }
    else
    {
        // sequence has not been read yet, set to 0 explicitly.
        nextContext_.setSequence(0);
        onReadError(e);
    }
}

void DriverConnection::asyncReadFormPayload()
{
    boost::asio::async_read(
        socket_,
        nextContext_.formPayloadBuffer(),
        boost::bind(
            &DriverConnection::afterReadFormPayload,
            shared_from_this(),
            boost::asio::placeholders::error
        )
    );
}

void DriverConnection::afterReadFormPayload(
    const boost::system::error_code& e
)
{
    if (!e)
    {
        context_ptr context = createContext();

        poller_.poll(
            boost::bind(
                &DriverConnection::handleRequest,
                shared_from_this(),
                context,
                boost::asio::placeholders::error
            )
        );

        // read next form
        asyncReadFormHeader();
    }
    else
    {
        onReadError(e);
    }
}

void DriverConnection::handleRequest(
    const context_ptr& context,
    const boost::system::error_code& e)
{
    if (!e)
    {
        Value requestValue;
        if (reader_->read(context->formPayload, requestValue))
        {
            if (requestValue.type() == driver::Value::kObjectType)
            {
                context->request.assignTmp(requestValue);
                driver::Router::handler_ptr handler = router_->find(
                    context->request.controller(),
                    context->request.action()
                );

                if (handler)
                {
                    try
                    {
                        // prepare request
                        context->request.header()[driver::Keys::remote_ip] =
                            socket_.remote_endpoint().address().to_string();

                        // prepare response
                        context->response.setSuccess(true); // assume success

                        izenelib::util::ClockTimer processTimer;

                        handler->invoke(
                            context->request.controller(),
                            context->request.action(),
                            context->request,
                            context->response,
                            poller_);

                        if (asBool(context->request.header()[driver::Keys::check_time]))
                        {
                            context->response[driver::Keys::timers][driver::Keys::process_time]
                                = processTimer.elapsed();
                        }

                        asyncWriteResponse(context);
                    }
                    catch (const std::exception& e)
                    {
                        asyncWriteError(context, e.what());
                    }
                }
                else
                {
                    asyncWriteError(context, "Handler not found");
                }
            }
            else
            {
                // malformed request
                asyncWriteError(context,
                                "Malformed request: "
                                "require an object as input.");
            }
        }
        else
        {
            // malformed request
            asyncWriteError(context, reader_->errorMessages());
        }
    }

    // Error if send end is closed, just ignore it
}

void DriverConnection::asyncWriteResponse(const context_ptr& context)
{
    if (asBool(context->request.header()[driver::Keys::check_time]))
    {
        context->response[driver::Keys::timers][driver::Keys::total_server_time]
            = context->serverTimer.elapsed();
    }
    writer_->write(context->response.get(), context->formPayload);
    context->setFormPayloadSize(context->formPayload.size());

    boost::asio::async_write(
        socket_,
        context->buffer(),
        boost::bind(
            &DriverConnection::afterWriteResponse,
            shared_from_this(),
            context
        )
    );
}

void DriverConnection::asyncWriteError(
    const context_ptr& context,
    const std::string& message
)
{
    context->response.addError(message);

    asyncWriteResponse(context);
}

void DriverConnection::afterWriteResponse(const context_ptr&)
{}

void DriverConnection::shutdownReceive()
{
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
}

void DriverConnection::onReadError(
    const boost::system::error_code& e
)
{
    if (e != boost::asio::error::eof)
    {
        context_ptr context = createContext();
        asyncWriteError(context, e.message());
    }
}

DriverConnection::context_ptr DriverConnection::createContext()
{
    context_ptr context(new DriverConnectionContext());
    context->swap(nextContext_);
    return context;
}

DriverConnectionFactory::DriverConnectionFactory(const router_ptr& router)
: router_(router)
{}

void DriverConnectionFactory::setFirewall(firewall_type firewall)
{
    firewall_ = firewall;
}

}
}

