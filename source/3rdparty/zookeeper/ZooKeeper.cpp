#include <ZooKeeper.hpp>
#include <ZooKeeperWatcher.hpp>

#include <zk_adaptor.h>
#include <zookeeper.jute.h>

#include "string.h" // memset
#include <algorithm>


using namespace zookeeper;

namespace {

using namespace std;

void watcher_callback(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    ZooKeeperWatcher* zkWatcher = static_cast<ZooKeeperWatcher*>(watcherCtx);
    zkWatcher->eventHandle(type, state, path);
}

}

ZooKeeper::ZooKeeper(const std::string& hosts, const int recvTimeout, void* watcher)
:hosts_(hosts)
,recvTimeout_(recvTimeout)
,sessionId_(0)
,context_(watcher)
,flags_(0)
{
    connect();
}

ZooKeeper::~ZooKeeper()
{
    disconnect();
}

void ZooKeeper::setZNodeWatcher(const std::string &path, ZooKeeperWatcher* zkWatcher)
{
    zoo_set_context(zk_, zkWatcher);
}

bool ZooKeeper::isConnected()
{
    if (zk_ && zk_->state == ZOO_CONNECTED_STATE)
    {
        return true;
    }

    return false;
}

bool ZooKeeper::connect()
{
    zk_ = zookeeper_init(
                hosts_.c_str(),
                &watcher_callback,
                recvTimeout_,
                sessionId_,
                context_,
                flags_ );

    if (zk_ == NULL)
        return false;

    return true;
}

void ZooKeeper::disconnect()
{
    if (NULL != zk_)
    {
        zookeeper_close(zk_);
        zk_ = NULL;
    }
}

bool ZooKeeper::createZNode(const std::string &path, const std::string &data, int flags)
{
    memset( newNodePath_, 0, MAX_PATH_LENGTH );

    int rc = zoo_create(
                 zk_,
                 path.c_str(),
                 data.c_str(),
                 data.length(),
                 &ZOO_OPEN_ACL_UNSAFE,
                 flags,
                 newNodePath_,
                 MAX_PATH_LENGTH );

    switch (rc)
    {
    case ZOK:
        return true;
        break;
    case ZNONODE:
        //the parent node does not exist. todo, handle errors
        break;
    case ZNODEEXISTS:
        //the node already exists
        break;
    case ZNOAUTH:
        //the client does not have permission.
        break;
    case ZNOCHILDRENFOREPHEMERALS:
        //cannot create children of ephemeral nodes.
        break;
    case ZBADARGUMENTS:
        //invalid input parameters
        break;
    case ZINVALIDSTATE:
        //zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
        break;
    case ZMARSHALLINGERROR:
        //failed to marshall a request; possibly, out of memory
        break;
    default:
        break;
    }

    return false;
}

bool ZooKeeper::deleteZNode(const string &path, int version)
{
    int rc = zoo_delete(zk_, path.c_str(), version);

    switch (rc)
    {
    case ZOK:
        return true;
        break;
    case ZNONODE:
        //the node does not exist. todo, handle errors
        break;
    case ZNOAUTH:
        //the client does not have permission.
        break;
    case ZBADVERSION:
        //expected version does not match actual version.:
        break;
    case ZNOTEMPTY:
        //children are present; node cannot be deleted.
        break;
    case ZBADARGUMENTS:
        //invalid input parameters
        break;
    case ZINVALIDSTATE:
        //zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
        break;
    case ZMARSHALLINGERROR:
        //failed to marshall a request; possibly, out of memory
        break;
    default:
        break;
    }

    return false;
}

bool ZooKeeper::isZNodeExists(const std::string &path)
{
    struct Stat stat;
    memset(&stat, 0, sizeof(Stat));

    int rc = zoo_exists(zk_, path.c_str(), 0, &stat);

    if (rc == ZOK)
    {
        return true;
    }

    return false;
}

bool ZooKeeper::getZNodeData(const std::string &path, std::string& data)
{
    memset( buffer_, 0, MAX_DATA_LENGTH );
    struct Stat stat; // xxx, return to caller
    memset( &stat, 0, sizeof(Stat) );

    int buffer_len = MAX_DATA_LENGTH - 1;
    int rc = zoo_get(
            zk_,
            path.c_str(),
            0,
            buffer_,
            &buffer_len,
            &stat );

    if (rc != ZOK)
    {
        return false;
    }

    string ret(buffer_, buffer_ + buffer_len);
    data.swap(ret);
    return true;
}

bool ZooKeeper::setZNodeData(const std::string &path, const std::string& data, int version)
{
    int rc = zoo_set( zk_,
                  path.c_str(),
                  data.c_str(),
                  data.length(),
                  version );

    if (rc == ZOK)
    {
        return true;
    }

    return false;
}

void ZooKeeper::getZNodeChildren(const std::string &path, std::vector<std::string>& childrens)
{
    String_vector children;
    memset( &children, 0, sizeof(children) );

    int rc = zoo_get_children(
                    zk_,
                    path.c_str(),
                    0,
                    &children );

    if (rc == ZOK)
    {
        for (int i = 0; i < children.count; ++i)
        {
            //convert each child's path from relative to absolute
            std::string absPath(path);
            if (path != "/")
            {
                absPath.append( "/" );
            }
            absPath.append( children.data[i] );
            childrens.push_back( absPath );
        }

        //make sure the order is always deterministic
        std::sort( childrens.begin(), childrens.end() );
    }
}





