/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ZkAdaptor.hpp>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <algorithm>

/**
 * \brief This class provides logic for checking if a request can be retried.
 */
class RetryHandler
{
public:
    RetryHandler(const ZooKeeperConfig &zkConfig) : zkConfig_(zkConfig)
    {
        if (zkConfig.getAutoReconnect())
            retries = 2;
        else
            retries = 0;
    }

    /**
     * \brief Attempts to fix a side effect of the given RC.
     *
     * @param rc the ZK error code
     * @return whether the error code has been handled and the caller should
     *         retry an operation the caused this error
     */
    bool handleRC(int rc)
    {
        //check if the given error code is recoverable
        if (!retryOnError(rc))
            return false;

        std::cerr << "[zktreeuti] Number of retries left: " << retries << std::endl;
        if (retries-- > 0)
            return true;
        else
            return false;
    }

private:
    /**
     * The ZK config.
     */
    const ZooKeeperConfig &zkConfig_;

    /**
     * The number of outstanding retries.
     */
    int retries;

    /**
     * Checks whether the given error entitles this adapter
     * to retry the previous operation.
     *
     * @param zkErrorCode one of the ZK error code
     */
    static bool retryOnError(int zkErrorCode)
    {
        return (zkErrorCode == ZCONNECTIONLOSS || zkErrorCode == ZOPERATIONTIMEOUT);
    }
};


// =======================================================================

ZooKeeperAdapter::ZooKeeperAdapter(ZooKeeperConfig config) throw(ZooKeeperException) :
        zkConfig_(config),
        zkHandle_(NULL)
{
    // Enforce setting up appropriate ZK log level
    if (zkConfig_.logLevel_ == ZooKeeperConfig::DEBUG)
    {
        zoo_set_debug_level( ZOO_LOG_LEVEL_DEBUG );
    }
    else if (zkConfig_.logLevel_ == ZooKeeperConfig::INFO)
    {
        zoo_set_debug_level( ZOO_LOG_LEVEL_INFO );
    }
    else if (zkConfig_.logLevel_ == ZooKeeperConfig::WARN)
    {
        zoo_set_debug_level( ZOO_LOG_LEVEL_WARN );
    }
    else
    {
        zoo_set_debug_level( ZOO_LOG_LEVEL_ERROR );
    }

    // Establish the connection
    reconnect();
}

ZooKeeperAdapter::~ZooKeeperAdapter()
{
    try
    {
        disconnect();
    }
    catch (std::exception &e)
    {
        std::cerr << "An exception while disconnecting from ZK: "
                  << e.what()
                  << std::endl;
    }
}

void ZooKeeperAdapter::validatePath(const string &path) throw(ZooKeeperException)
{
    if (path.find ("/") != 0)
    {
        std::ostringstream oss;
        oss << "Node path must start with '/' but" "it was '"
        << path
        << "'";
        throw ZooKeeperException (oss.str());
    }
    if (path.length() > 1)
    {
        if (path.rfind ("/") == path.length() - 1)
        {
            std::ostringstream oss;
            oss << "Node path must not end with '/' but it was '"
            << path
            << "'";
            throw ZooKeeperException (oss.str());
        }
        if (path.find( "//" ) != string::npos)
        {
            std::ostringstream oss;
            oss << "Node path must not contain '//' but it was '"
            << path
            << "'";
            throw ZooKeeperException (oss.str());
        }
    }
}

void ZooKeeperAdapter::disconnect()
{
    if (zkHandle_ != NULL)
    {
        zookeeper_close (zkHandle_);
        zkHandle_ = NULL;
    }
}

void ZooKeeperAdapter::reconnect() throw(ZooKeeperException)
{
    // Clear the connection state
    disconnect();

    // Establish a new connection to ZooKeeper
    zkHandle_ = zookeeper_init( zkConfig_.getHosts().c_str(),
                                NULL,
                                zkConfig_.getLeaseTimeout(),
                                0,
                                NULL,
                                0);
    if (zkHandle_ == NULL)
    {
        // Invalid handle returned
        std::ostringstream oss;
        oss << "Unable to connect to ZK running at '"
        << zkConfig_.getHosts()
        << "'";
        throw ZooKeeperException (oss.str());
    }

    // Enter into connect loop
    int64_t connWaitTime = zkConfig_.getConnectTimeout();
    while (1)
    {
        int state = zoo_state (zkHandle_);
        if (state == ZOO_CONNECTED_STATE)
        {
            // connected
            std::cerr << "Connected! zkHandle_: "
                      << zkHandle_
                      << std::endl;
            return;
        }
        else if ( state && state != ZOO_CONNECTING_STATE)
        {
            // Not connecting any more... some other issue
            std::ostringstream oss;
            oss << "Unable to connect to ZK running at '"
            << zkConfig_.getHosts()
            << "'; state="
            << state;
            throw ZooKeeperException (oss.str());
        }

        // Still connecting, wait and come back
        struct timeval now;
        gettimeofday( &now, NULL );
        int64_t milliSecs = -(now.tv_sec * 1000LL + now.tv_usec / 1000);
        std::cerr << "About to wait 1 sec" << std::endl;
        sleep (1);
        gettimeofday( &now, NULL );
        milliSecs += now.tv_sec * 1000LL + now.tv_usec / 1000;
        connWaitTime -= milliSecs;
        // Timed out !!!
        if (connWaitTime <= 0)
            break;
    }

    // Timed out while connecting
    std::ostringstream oss;
    oss << "Timed out while connecting to ZK running at '"
    << zkConfig_.getHosts()
    << "'";
    throw ZooKeeperException (oss.str());
}

void ZooKeeperAdapter::verifyConnection() throw(ZooKeeperException)
{
    // Check connection state
    int state = zoo_state (zkHandle_);
    if (state != ZOO_CONNECTED_STATE)
    {
        if (zkConfig_.getAutoReconnect())
        {
            // Trying to reconnect
            std::cerr << "Trying to reconnect..." << std::endl;
            reconnect();
        }
        else
        {
            std::ostringstream oss;
            oss << "Disconnected from ZK running at '"
            << zkConfig_.getHosts()
            << "'; state="
            << state;
            throw ZooKeeperException (oss.str());
        }
    }
}

bool ZooKeeperAdapter::createNode(const string &path,
                                  const string &value,
                                  int flags,
                                  bool createAncestors) throw(ZooKeeperException)
{
    const int MAX_PATH_LENGTH = 1024;
    char realPath[MAX_PATH_LENGTH];
    realPath[0] = 0;

    int rc;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        rc = zoo_create( zkHandle_,
                         path.c_str(),
                         value.c_str(),
                         value.length(),
                         &ZOO_OPEN_ACL_UNSAFE,
                         flags,
                         realPath,
                         MAX_PATH_LENGTH );
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK) // check return status
    {
        if (rc == ZNODEEXISTS)
        {
            //the node already exists
            std::cerr << "ZK node " << path << " already exists" << std::endl;
            return false;
        }
        else if (rc == ZNONODE && createAncestors)
        {
            std::cerr << "Intermediate ZK node missing in path " << path << std::endl;
            //one of the ancestors doesn't exist so lets start from the root
            //and make sure the whole path exists, creating missing nodes if
            //necessary
            for (string::size_type pos = 1; pos != string::npos; )
            {
                pos = path.find( "/", pos );
                if (pos != string::npos)
                {
                    try
                    {
                        createNode( path.substr( 0, pos ), "", 0, true );
                    }
                    catch (ZooKeeperException &e)
                    {
                        throw ZooKeeperException( string("Unable to create " "node ") + path, rc );
                    }
                    pos++;
                }
                else
                {
                    // No more path components
                    return createNode( path, value, flags, false );
                }
            }
        }

        // Unexpected error during create
        std::cerr << "Error in creating ZK node " << path << std::endl;
        throw ZooKeeperException( string("Unable to create node ") + path, rc );
    }

    // Success
    std::cerr << realPath << " has been created" << std::endl;
    return true;
}

bool ZooKeeperAdapter::deleteNode(const string &path,
                                  bool recursive,
                                  int version) throw(ZooKeeperException)
{
    // Validate the zk path
    validatePath( path );

    int rc;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        rc = zoo_delete( zkHandle_, path.c_str(), version );
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK) //check return status
    {
        if (rc == ZNONODE)
        {
            std::cerr << "ZK Node "
                      << path
                      << " does not exist"
                      << std::endl;
            return false;
        }
        if (rc == ZNOTEMPTY && recursive)
        {
            std::cerr << "ZK Node "
                      << path
                      << " not empty; deleting..."
                      << std::endl;
            //get all children and delete them recursively...
            vector<string> nodeList = getNodeChildren (path);
            for (vector<string>::const_iterator i = nodeList.begin();
                    i != nodeList.end();
                    ++i)
            {
                deleteNode( *i, true );
            }
            //...and finally attempt to delete the node again
            return deleteNode( path, false );
        }

        // Unexpected return without success
        std::cerr << "Unable to delete ZK node " << path << std::endl;
        throw ZooKeeperException( string("Unable to delete node ") + path, rc );
    }

    // success
    std::cerr<< path << " has been deleted" << std::endl;
    return true;
}

vector< string > ZooKeeperAdapter::getNodeChildren (const string &path) throw (ZooKeeperException)
{
    // Validate the zk path
    validatePath( path );

    String_vector children;
    memset( &children, 0, sizeof(children) );
    int rc;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        rc = zoo_get_children( zkHandle_,
                               path.c_str(),
                               0,
                               &children );
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK) // check return code
    {
        std::cerr << "Error in fetching children of " << path << std::endl;
        throw ZooKeeperException( string("Unable to get children of node ") + path, rc );
    }
    else
    {
        vector< string > nodeList;
        for (int i = 0; i < children.count; ++i)
        {
            //convert each child's path from relative to absolute
            string absPath(path);
            if (path != "/")
            {
                absPath.append( "/" );
            }
            absPath.append( children.data[i] );
            nodeList.push_back( absPath );
        }

        //make sure the order is always deterministic
        sort( nodeList.begin(), nodeList.end() );
        deallocate_String_vector(&children);
        return nodeList;
    }
}

bool ZooKeeperAdapter::nodeExists(const string &path) throw(ZooKeeperException)
{
    // Validate the zk path
    validatePath( path );

    struct Stat tmpStat;
    struct Stat* stat = &tmpStat;
    memset( stat, 0, sizeof(Stat) );

    int rc;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        rc = zoo_exists( zkHandle_,
                         path.c_str(),
                         0,
                         stat );
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK)
    {
        if (rc == ZNONODE)
            return false;
        // Some error
        std::cerr << "Error in checking existance of " << path << std::endl;
        throw ZooKeeperException( string("Unable to check existence of node ") + path, rc );
    }
    else
    {
        return true;
    }
}

string ZooKeeperAdapter::getNodeData(const string &path) throw(ZooKeeperException)
{
    // Validate the zk path
    validatePath( path );

    const int MAX_DATA_LENGTH = 128 * 1024;
    char buffer[MAX_DATA_LENGTH];
    memset( buffer, 0, MAX_DATA_LENGTH );
    struct Stat tmpStat;
    struct Stat* stat = &tmpStat;
    memset( stat, 0, sizeof(Stat) );

    int rc;
    int len;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        len = MAX_DATA_LENGTH - 1;
        rc = zoo_get( zkHandle_,
                      path.c_str(),
                      0,
                      buffer, &len, stat );
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK) // checl return code
    {
        std::cerr << "Error in fetching value of " << path << std::endl;
        throw ZooKeeperException( string("Unable to get data of node ") + path, rc );
    }

    // return data
    return string( buffer, buffer + len );
}

void ZooKeeperAdapter::setNodeData(const string &path,
                                   const string &value,
                                   int version) throw(ZooKeeperException)
{
    // Validate the zk path
    validatePath( path );

    int rc;
    RetryHandler rh(zkConfig_);
    do
    {
        verifyConnection();
        rc = zoo_set( zkHandle_,
                      path.c_str(),
                      value.c_str(),
                      value.length(),
                      version);
    }
    while (rc != ZOK && rh.handleRC(rc));
    if (rc != ZOK) // check return code
    {
        std::cerr << "Error in setting value of " << path << std::endl;
        throw ZooKeeperException( string("Unable to set data for node ") + path, rc );
    }
    // success
}
