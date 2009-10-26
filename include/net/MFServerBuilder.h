#ifndef NET_MFSERVER_BUILDER_H
#define NET_MFSERVER_BUILDER_H
/**
 * @file net/MFServerBuilder.h
 * @author Ian Yang
 * @date Created <2009-10-23 09:19:36>
 * @date Updated <2009-10-26 11:27:15>
 * @brief builder to create MFServer
 */

namespace messageframework {

template<typename ServerType>
class MFServerBuilder
{
public:
    typedef typename ServerType::service_handle_type service_handle_type;

    void createServer(unsigned int serverPort,
                      const std::string& controllerIP,
                      unsigned int controllerPort,
                      unsigned int nThreads)
    {
        server_.reset(
            new ServerType(serverPort, controllerIP, controllerPort, nThreads)
        );
    }

    void setServiceHandle(const boost::shared_ptr<service_handle_type>& handle)
    {
        if (server_ && handle)
        {
            server_->setServiceHandle(handle);
        }
        else
        {
            server_.reset();
        }
    }

    void setAgentInfo(const std::string& agentInfo)
    {
        if (server_)
        {
            server_->setAgentInfo(agentInfo);
        }
    }

    void addService(const std::string& name,
                    const ServiceItem<service_handle_type>& callback)
    {
        if (server_ && !server_->addService())
        {
            server_.reset();
        }
    }

    void addAllServices()
    {
        if (server_ && !service_handle_type::addServicesToServer(*server_))
        {
            server_.reset();
        }
    }

    const boost::shared_ptr<ServerType> getServer() const
    {
        return server_;
    }

private:
    boost::shared_ptr<ServerType> server_;
};

}

#endif // NET_MFSERVER_BUILDER_H
