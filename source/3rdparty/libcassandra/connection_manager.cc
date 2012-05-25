#include "libcassandra/connection_manager.h"
#include "libcassandra/cassandra_client.h"

using org::apache::cassandra::CassandraClient;

namespace libcassandra
{

CassandraConnectionManager::CassandraConnectionManager()
    : port_(0), pool_size_(0), last_connect_(0), connected_(false)
{}

CassandraConnectionManager::~CassandraConnectionManager()
{
    clear();
}

time_t CassandraConnectionManager::createTimeStamp() const
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void CassandraConnectionManager::init(
        const std::string& host,
        int port,
        size_t pool_size)
{
    host_ = host;
    port_ = port;
    pool_size_ = pool_size;
    last_connect_ = 0;
    reconnect();
}

void CassandraConnectionManager::clear()
{
    std::list<MyCassandraClient *>::iterator it = clients_.begin();
    for (; it != clients_.end(); ++it)
    {
        delete *it;
    }
    clients_.clear();
}

bool CassandraConnectionManager::reconnect()
{
    time_t new_timestamp = createTimeStamp();
    boost::unique_lock<boost::mutex> lock(mutex_);
    if (new_timestamp - last_connect_ < 10000000)
        return connected_;

    last_connect_ = new_timestamp;
    connected_ = false;
    clear();
    for (size_t i = 0; i < pool_size_; ++i)
    {
        clients_.push_back(new MyCassandraClient(host_, port_));
        clients_.back()->open();
    }
    connected_ = true;

    return true;
}

MyCassandraClient * CassandraConnectionManager::borrowClient()
{
    MyCassandraClient* client = NULL;
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (clients_.empty())
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
    cond_.notify_one();
}

}
