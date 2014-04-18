#include <ZooKeeper.hpp>
#include <ZooKeeperWatcher.hpp>
#include <ZooKeeperEvent.hpp>

#include <zk_adaptor.h>
#include <zookeeper.jute.h>

#include "string.h" // memset
#include "unistd.h" // usleep
#include <algorithm>
#include <sstream>


using namespace izenelib::zookeeper;

namespace {

using namespace std;

void watcher_callback(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (watcherCtx != NULL)
    {
        ZooKeeperWatcher* zkWatcher = static_cast<ZooKeeperWatcher*>(watcherCtx);
        zkWatcher->handleEvent(type, state, path);
    }
}

}

static FILE* gs_logFile = 0;

#define CHECK_RETURN_ERROR(err, ERROR_TYPE)  \
    if (err == (int)ERROR_TYPE)  \
        return #ERROR_TYPE

ZooKeeper::ZooKeeper(const std::string& hosts, const int recvTimeout, bool isAutoReconnect)
:hosts_(hosts)
,recvTimeout_(recvTimeout)
,sessionId_(0)
,flags_(0)
,zk_(NULL)
,zkError_(ZERR_OK)
{
    // only one watcher can be set at a time for a client.
    uniqueWatcher_ = new ZooKeeperWatcher();
    context_ = uniqueWatcher_;

    memset( realNodePath_, 0, MAX_PATH_LENGTH );
    memset( buffer_, 0, MAX_DATA_LENGTH );

    setLogFile("./zookeeper.log"); //output to file, or stderr

    connect(isAutoReconnect);
}

ZooKeeper::~ZooKeeper()
{
    disconnect();

    if (uniqueWatcher_)
        delete uniqueWatcher_;

    remove("./zookeeper.log");  //xxx
}

void ZooKeeper::registerEventHandler(ZooKeeperEventHandler* evtHandler)
{
    uniqueWatcher_->registerEventHandler(evtHandler);
}

bool ZooKeeper::isConnected()
{
    //cout << "isConnected " << ZooKeeperEvent::state2String(zk_->state) <<end;
    boost::lock_guard<boost::mutex> g(mutex_);
    if (zk_ && zk_->state == ZOO_CONNECTED_STATE)
    {
        return true;
    }

    return false;
}

void ZooKeeper::setDebugLevel(ZooLogLevel logLevel)
{
    zoo_set_debug_level(logLevel);
}

void ZooKeeper::setLogFile(const std::string& logFile)
{
    gs_logFile = fopen(logFile.c_str(), "w");
    zoo_set_log_stream(gs_logFile);
}

std::string ZooKeeper::error2String(ZKErrorType zkerror)
{
    CHECK_RETURN_ERROR(zkerror, ZOK);
    CHECK_RETURN_ERROR(zkerror, ZSYSTEMERROR);
    CHECK_RETURN_ERROR(zkerror, ZRUNTIMEINCONSISTENCY);
    CHECK_RETURN_ERROR(zkerror, ZDATAINCONSISTENCY);
    CHECK_RETURN_ERROR(zkerror, ZCONNECTIONLOSS);
    CHECK_RETURN_ERROR(zkerror, ZMARSHALLINGERROR);
    CHECK_RETURN_ERROR(zkerror, ZUNIMPLEMENTED);
    CHECK_RETURN_ERROR(zkerror, ZOPERATIONTIMEOUT);
    CHECK_RETURN_ERROR(zkerror, ZBADARGUMENTS);
    CHECK_RETURN_ERROR(zkerror, ZINVALIDSTATE);

    CHECK_RETURN_ERROR(zkerror, ZAPIERROR);
    CHECK_RETURN_ERROR(zkerror, ZNONODE);
    CHECK_RETURN_ERROR(zkerror, ZNOAUTH);
    CHECK_RETURN_ERROR(zkerror, ZBADVERSION);
    CHECK_RETURN_ERROR(zkerror, ZNOCHILDRENFOREPHEMERALS);
    CHECK_RETURN_ERROR(zkerror, ZNODEEXISTS);
    CHECK_RETURN_ERROR(zkerror, ZNOTEMPTY);
    CHECK_RETURN_ERROR(zkerror, ZSESSIONEXPIRED);
    CHECK_RETURN_ERROR(zkerror, ZINVALIDCALLBACK);
    CHECK_RETURN_ERROR(zkerror, ZINVALIDACL);
    CHECK_RETURN_ERROR(zkerror, ZAUTHFAILED);
    CHECK_RETURN_ERROR(zkerror, ZCLOSING);
    CHECK_RETURN_ERROR(zkerror, ZNOTHING);
    CHECK_RETURN_ERROR(zkerror, ZSESSIONMOVED);

    std::stringstream ss;
    ss << "UNKNOWN (error="<<(int)zkerror<<")";
    return ss.str();
}

int ZooKeeper::getState()
{
    boost::lock_guard<boost::mutex> g(mutex_);
    return (zk_ ? zk_->state : 0);
}

std::string ZooKeeper::getStateString()
{
    return ZooKeeperEvent::state2String(getState());
}

void ZooKeeper::connect(bool isAutoReconnect)
{
    //disconnect(); // close last connection

    boost::unique_lock<boost::mutex> g(mutex_);
    if (zk_ != NULL)
        return;
    zk_ = zookeeper_init(
                hosts_.c_str(),
                &watcher_callback,
                recvTimeout_,
                sessionId_,
                context_,
                flags_ );

    if (zk_ == NULL)
    {
        std::string ex = "Unable to connect to ZooKeeper running at (servers): ";
        ex += hosts_;
        if (hosts_.empty())
            ex += "(uninitialized address)";
        throw ZooKeeperException (ex);
    }

    if (!isAutoReconnect)
    {
        return;
    }

    // xxx, reconnect, microseconds (us)
    int32_t timeout = 4000000;
    int32_t step = 500000;
    int32_t wait = 0;
    while(zk_->state != ZOO_CONNECTED_STATE)
    {
        if ( wait > timeout )
            break;
        cond_.timed_wait(g, boost::posix_time::microseconds(step));
        wait += step;
    }
    return;
}

void ZooKeeper::disconnect()
{
    zhandle_t* tmp = NULL;
    {
        boost::lock_guard<boost::mutex> g(mutex_);
        tmp = zk_;
        zk_ = NULL;
    }
    if (NULL != tmp)
    {
        zookeeper_close(tmp);
    }
}

/// Synchronous API

bool ZooKeeper::createZNode(const std::string &path, const std::string &data, ZNodeCreateType flags)
{
    boost::lock_guard<boost::mutex> g(mutex_);
    memset( realNodePath_, 0, MAX_PATH_LENGTH );

    if (zk_ == NULL)
        return false;
    int rc = zoo_create(
                 zk_,
                 path.c_str(),
                 data.c_str(),
                 data.length(),
                 &ZOO_OPEN_ACL_UNSAFE,
                 flags,
                 realNodePath_,
                 MAX_PATH_LENGTH );

    zkError_ = ZooKeeper::ZKErrorType(rc);

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
        //cout << "create error: existed!" <<endl;
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

std::string ZooKeeper::getLastCreatedNodePath()
{
    return std::string(realNodePath_);
}

bool ZooKeeper::deleteZNodeNoLock(const string &path, bool recursive, int version)
{
    if (zk_ == NULL)
        return false;
    int rc = zoo_delete(zk_, path.c_str(), version);

    zkError_ = ZooKeeper::ZKErrorType(rc);

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
        if (recursive)
        {
            std::vector<std::string> children;
            getZNodeChildrenNoLock(path, children);
            for (size_t i = 0; i < children.size(); i++)
            {
                deleteZNodeNoLock(children[i], true);
            }

            return deleteZNodeNoLock(path);
        }
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

bool ZooKeeper::deleteZNode(const string &path, bool recursive, int version)
{
    boost::lock_guard<boost::mutex> g(mutex_);
    return deleteZNodeNoLock(path, recursive, version);
}

bool ZooKeeper::isZNodeExists(const std::string &path, ZNodeWatchType watch)
{
    boost::lock_guard<boost::mutex> g(mutex_);
    struct Stat stat;
    memset(&stat, 0, sizeof(Stat));

    if (zk_ == NULL)
        return false;
    int rc = zoo_exists(zk_, path.c_str(), watch, &stat);

    zkError_ = ZooKeeper::ZKErrorType(rc);

    if (rc == ZOK)
    {
        return true;
    }

    return false;
}

bool ZooKeeper::getZNodeData(const std::string &path, std::string& data, ZNodeWatchType watch)
{
    boost::lock_guard<boost::mutex> g(mutex_);
    memset( buffer_, 0, MAX_DATA_LENGTH );
    struct Stat stat; // xxx, return to caller
    memset( &stat, 0, sizeof(Stat) );

    if (zk_ == NULL)
        return false;

    int buffer_len = MAX_DATA_LENGTH - 1;
    int rc = zoo_get(
            zk_,
            path.c_str(),
            watch,
            buffer_,
            &buffer_len,
            &stat );

    zkError_ = ZooKeeper::ZKErrorType(rc);

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
    boost::lock_guard<boost::mutex> g(mutex_);
    if (zk_ == NULL)
        return false;
    int rc = zoo_set( zk_,
                  path.c_str(),
                  data.c_str(),
                  data.length(),
                  version );

    zkError_ = ZooKeeper::ZKErrorType(rc);

    if (rc == ZOK)
    {
        return true;
    }

    return false;
}

bool ZooKeeper::getZNodeChildrenNoLock(const std::string &path, std::vector<std::string>& childrenList, ZNodeWatchType watch, bool inAbsPath)
{
    childrenList.clear();

    String_vector children;
    memset( &children, 0, sizeof(children) );

    if (zk_ == NULL)
        return false;
    int rc = zoo_get_children(
                    zk_,
                    path.c_str(),
                    watch,
                    &children );

    zkError_ = ZooKeeper::ZKErrorType(rc);

    if (rc == ZOK)
    {
        childrenList.reserve(children.count);
        for (int i = 0; i < children.count; ++i)
        {
            if (inAbsPath)
            {
                //convert each child's path from relative to absolute
                std::string absPath(path);
                if (path != "/")
                {
                    absPath.append( "/" );
                }
                absPath.append( children.data[i] );
                childrenList.push_back( absPath );
            }
            else
            {
                childrenList.push_back(children.data[i]);
            }
        }

        //make sure the order is always deterministic
        std::sort( childrenList.begin(), childrenList.end() );
        deallocate_String_vector(&children);
    }
    return true;

}

bool ZooKeeper::getZNodeChildren(const std::string &path, std::vector<std::string>& childrenList, ZNodeWatchType watch, bool inAbsPath)
{
    boost::lock_guard<boost::mutex> g(mutex_);
    return getZNodeChildrenNoLock(path, childrenList, watch, inAbsPath);
}

/// Asynchronous API

//bool ZooKeeper::acreateZNode(const std::string &path, const std::string &data, ZNodeCreateType flags)
//{
//    boost::lock_guard<boost::mutex> g(mutex_);
//    memset( realNodePath_, 0, MAX_PATH_LENGTH );
//    if (zk_ == NULL)
//        return false;
//
//    int rc = zoo_acreate(
//                 zk_,
//                 path.c_str(),
//                 data.c_str(),
//                 data.length(),
//                 &ZOO_OPEN_ACL_UNSAFE,
//                 flags,
//                 NULL,
//                 NULL);
//
//    zkError_ = ZooKeeper::ZKErrorType(rc);
//
//    if (rc == ZOK)
//        return true;
//
//    return false;
//}
//
/// Other

void ZooKeeper::showZKNamespace(const std::string& path, int level, std::ostream& out)
{
    if (level == 0) {
        out << "=== ZooKeeper's Hierarchical Namespace from root \""<<path<<"\" ==="<<std::endl;
    }

    // znode path
    if (level <= 0)
    {
        out << path;

        if (!isZNodeExists(path))
        {
            out << " - path not exsited!"<<std::endl;
            return;
        }
    }
    else
    {
        size_t pos = path.find_last_of('/');
        out<<std::string((level-1)*4, ' ')<<"|-- "<<path.substr(pos+1);
    }

    // znode data
    std::string data;
    getZNodeData(path, data);
    if (!data.empty())
        out<<"  ["<<data<<"]";

    out << std::endl;

    // children nodes
    std::vector<std::string> children;
    getZNodeChildren(path, children);
    for (size_t i = 0; i < children.size(); i++)
    {
        showZKNamespace(children[i], level+1, out);
    }

    if (level == 0) {
        out << "========================================================="<<std::endl;
    }
}
