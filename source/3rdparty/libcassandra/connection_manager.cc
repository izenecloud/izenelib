#include "libcassandra/connection_manager.h"
#include "libcassandra/cassandra_client.h"

using org::apache::cassandra::CassandraClient;

namespace libcassandra
{

CassandraConnectionManager::CassandraConnectionManager()
{}

CassandraConnectionManager::~CassandraConnectionManager()
{
    clear();
}

void CassandraConnectionManager::init(
    const std::string& host,
    int port,
    size_t pool_size)
{
    clear();
    for(size_t i = 0; i < pool_size; ++i)
    {
        MyCassandraClient* client = new MyCassandraClient(host,port);
        client->open();
        clients_.push_back(client);
    }
}

void CassandraConnectionManager::clear()
{
    std::list<MyCassandraClient*>::iterator it = clients_.begin();
    for(;it != clients_.end(); ++it)
    {
        delete *it;
    }
}

MyCassandraClient * CassandraConnectionManager::borrowClient()
{
    MyCassandraClient* client = NULL;
    boost::unique_lock<boost::mutex> lock(mutex_);
    while(clients_.empty())
    {
        cond_.wait(lock);
    }
    client = clients_.front();
    clients_.pop_front();

    return client;
}

void CassandraConnectionManager::releaseClient(MyCassandraClient * client)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    clients_.push_back(client);
}

}
