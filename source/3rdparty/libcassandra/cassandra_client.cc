#include "libcassandra/cassandra_client.h"

#include <glog/logging.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;

namespace libcassandra
{

MyCassandraClient::MyCassandraClient(const std::string& host, int port)
    :host_(host)
    ,port_(port)
    ,thrift_client_(NULL)
{
}

MyCassandraClient::~MyCassandraClient()
{
    close();
    if (thrift_client_) delete thrift_client_;
}

CassandraClient* MyCassandraClient::getCassandra(const std::string& keyspace)
{
    getCassandra();
    if (key_space_.compare(keyspace))
    {
        thrift_client_->set_keyspace(keyspace);
        key_space_.assign(keyspace);
    }
    return thrift_client_;
}

CassandraClient* MyCassandraClient::getCassandra()
{
    if (!thrift_client_)
    {
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport_));
        thrift_client_= new(std::nothrow) CassandraClient(protocol);
    }
    return thrift_client_;
}

void MyCassandraClient::open()
{
    boost::shared_ptr<TTransport> socket(new TSocket(host_, port_));
    transport_.reset(new TFramedTransport(socket));
    transport_->open();
}

bool MyCassandraClient::isOpen()
{
    return transport_ && transport_->isOpen();
}

void MyCassandraClient::close()
{
    if (isOpen())
    {
        try
        {
            transport_->flush();
            transport_->close();
        }
        catch (const ::apache::thrift::TException& e)
        {
            LOG(ERROR) << "exception in closing thrift transport: " << e.what();
        }
    }
}

}
