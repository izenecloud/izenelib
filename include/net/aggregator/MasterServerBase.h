/**
 * @file MasterServerBase.h
 * @author Zhongxia Li
 * @date Sep 1, 2011
 * @brief 
 */
#ifndef MASTER_SERVER_BASE_H_
#define MASTER_SERVER_BASE_H_

#include <3rdparty/msgpack/msgpack.hpp>
#include <3rdparty/msgpack/rpc/client.h>
#include <3rdparty/msgpack/rpc/request.h>
#include <3rdparty/msgpack/rpc/server.h>

#include <util/singleton.h>

namespace net{
namespace aggregator{


struct NotifyMSG
{
    std::string method;
    std::string identity;

    std::string error;

    MSGPACK_DEFINE(method,identity,error);
};

class Notifier
{
public:
    bool notify(NotifyMSG& msg)
    {
        if (!hasMaster_)
            return true;

        try
        {
            msgpack::rpc::client cli(host_, port_);
            cli.notify(msg.method, msg);
            cli.get_loop()->flush();
        }
        catch (std::exception& e)
        {
            std::cerr<<"**Failed to nofity Master : "<<e.what()<<endl;
            return false;
        }

        return true;
    }

    void setMasterServerInfo(const std::string& host, uint16_t port)
    {
        hasMaster_ = true;
        host_ = host;
        port_ = port;
    }

protected:
    bool hasMaster_;
    std::string host_;
    uint16_t port_;
};

typedef Singleton<Notifier> MasterNotifierSingleton;

class MasterServerBase : public msgpack::rpc::server::base
{
public:
    void start(const std::string& host, uint16_t port, unsigned int threadnum=4)
    {
        instance.listen(host, port);
        instance.start(threadnum);
    }

    void dispatch(msgpack::rpc::request req)
    {
        try
        {
            std::string method;
            req.method().convert(&method);

            msgpack::type::tuple<NotifyMSG> params;
            req.params().convert(&params);
            NotifyMSG msg = params.get<0>();

            if (!notify(msg))
            {
                cout << "#[Master] notified, "<<msg.error<<endl;
                req.error(msg.error);
            }
        }
        catch (msgpack::type_error& e)
        {
            req.error(msgpack::rpc::ARGUMENT_ERROR);
            cout << "#[Master] notified, Argument error!"<<endl;
            return;
        }
        catch (std::exception& e)
        {
            cout << "#[Master] notified, "<<e.what()<<endl;
            req.error(std::string(e.what()));
            return;
        }
    }

protected:
    virtual bool notify(NotifyMSG& msg) = 0;
};

}
}

#endif /* MASTER_SERVER_BASE_H_ */
