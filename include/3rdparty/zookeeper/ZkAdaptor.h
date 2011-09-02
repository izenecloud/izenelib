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

#ifndef __ZK_ADAPTER_H__
#define __ZK_ADAPTER_H__

#include <string>
#include <vector>

extern "C"
{
#include "zookeeper.h"
}


using std::string;
using std::vector;

/**
 * \brief A cluster related exception.
 */
class ZooKeeperException : public std::exception
{
public:

    /**
     * \brief Constructor.
     *
     * @param msg the detailed message associated with this exception
     */
    ZooKeeperException(const string& msg) :
            message_(msg),
            zkErrorCode_(0) {}

    /**
     * \brief Constructor.
     *
     * @param msg the detailed message associated with this exception
     * @param errorCode the ZK error code associated with this exception
     */
    ZooKeeperException(const string &msg, int errorCode) :
            zkErrorCode_(errorCode)
    {
        char tmp[100];
        sprintf( tmp, " (ZK error code: %d)", errorCode );
        message_ = msg + tmp;
    }

    /**
     * \brief Destructor.
     */
    ~ZooKeeperException() throw() {}

    /**
     * \brief Returns detailed description of the exception.
     */
    const char *what() const throw()
    {
        return message_.c_str();
    }

    /**
     * \brief Returns the ZK error code.
     */
    int getZKErrorCode() const
    {
        return zkErrorCode_;
    }

private:

    /**
     * The detailed message associated with this exception.
     */
    string message_;

    /**
     * The optional error code received from ZK.
     */
    int zkErrorCode_;

};

/**
 * \brief This class encapsulates configuration of a ZK client.
 */
struct ZooKeeperConfig
{

    enum ZOOKEEPER_LOG_LEVEL
    {
        INFO,
        DEBUG,
        WARN,
        ERROR
    };

    /**
     * \brief Constructor.
     *
     * @param hosts the comma separated list of host and port pairs of ZK nodes
     * @param leaseTimeout the lease timeout (heartbeat)
     * @param autoReconnect whether to allow for auto-reconnect
     * @param connectTimeout the connect timeout, in milliseconds;
     */
    ZooKeeperConfig(const string &hosts,
                    int leaseTimeout,
                    bool autoReconnect = true,
                    long long int connectTimeout = 15000)
            : hosts_(hosts),
            leaseTimeout_(leaseTimeout),
            autoReconnect_(autoReconnect),
            connectTimeout_(connectTimeout),
            logLevel_(ERROR) {}

    /**
     * \brief Returns the list of ZK hosts to connect to.
     */
    string getHosts() const
    {
        return hosts_;
    }

    /**
     * \brief Returns the lease timeout.
     */
    int getLeaseTimeout() const
    {
        return leaseTimeout_;
    }

    /**
     * \brief Returns whether {@link ZooKeeperAdapter} should attempt
     * \brief to automatically reconnect in case of a connection failure.
     */
    bool getAutoReconnect() const
    {
        return autoReconnect_;
    }

    /**
     * \brief Gets the connect timeout.
     *
     * @return the connect timeout
     */
    long long int getConnectTimeout() const
    {
        return connectTimeout_;
    }



    /**
     * The host addresses of ZK nodes.
     */
    const string hosts_;

    /**
     * The ZK lease timeout.
     */
    const int leaseTimeout_;

    /**
     * True if this adapater should attempt to autoreconnect in case
     * the current session has been dropped.
     */
    const bool autoReconnect_;

    /**
     * How long to wait, in milliseconds, before a connection
     * is established to ZK.
     */
    const long long int connectTimeout_;

    /**
     * Log level of ZK
     */

    ZOOKEEPER_LOG_LEVEL logLevel_;

};

/**
 * \brief This is a wrapper around ZK C synchrounous API.
 */
class ZooKeeperAdapter
{
public:
    /**
     * \brief Constructor.
     * Attempts to create a ZK adapter, optionally connecting
     * to the ZK. Note, that if the connection is to be established
     * and the given listener is NULL, some events may be lost,
     * as they may arrive asynchronously before this method finishes.
     *
     * @param config the ZK configuration
     * @throw ZooKeeperException if cannot establish connection to the given ZK
     */
    ZooKeeperAdapter(ZooKeeperConfig config) throw(ZooKeeperException);

    /**
     * \brief Destructor.
     */
    ~ZooKeeperAdapter();

    /**
     * \brief Returns the current config.
     */
    const ZooKeeperConfig &getZooKeeperConfig() const
    {
        return zkConfig_;
    }

    /**
     * \brief Restablishes connection to the ZK.
     * If this adapter is already connected, the current connection
     * will be dropped and a new connection will be established.
     *
     * @throw ZooKeeperException if cannot establish connection to the ZK
     */
    void reconnect() throw(ZooKeeperException);

    /**
     * \brief Disconnects from the ZK and unregisters {@link #zkHandle_}.
     */
    void disconnect();

    /**
     * \brief Creates a new node identified by the given path.
     * This method will optionally attempt to create all missing ancestors.
     *
     * @param path the absolute path name of the node to be created
     * @param value the initial value to be associated with the node
     * @param flags the ZK flags of the node to be created
     * @param createAncestors if true and there are some missing ancestor nodes,
     *        this method will attempt to create them
     *
     * @return true if the node has been successfully created; false otherwise
     * @throw ZooKeeperException if the operation has failed
     */
    bool createNode(const string &path,
                    const string &value = "",
                    int flags = 0,
                    bool createAncestors = true) throw(ZooKeeperException);

    /**
     * \brief Deletes a node identified by the given path.
     *
     * @param path the absolute path name of the node to be deleted
     * @param recursive if true this method will attempt to remove
     *                  all children of the given node if any exist
     * @param version the expected version of the node. The function will
     *                fail if the actual version of the node does not match
     *                the expected version
     *
     * @return true if the node has been deleted; false otherwise
     * @throw ZooKeeperException if the operation has failed
     */
    bool deleteNode(const string &path,
                    bool recursive = false,
                    int version = -1) throw(ZooKeeperException);

    /**
     * \brief Retrieves list of all children of the given node.
     *
     * @param path the absolute path name of the node for which to get children
     * @return the list of absolute paths of child nodes, possibly empty
     * @throw ZooKeeperException if the operation has failed
     */
    vector<string> getNodeChildren( const string &path) throw(ZooKeeperException);

    /**
     * \brief Check the existance of path to a znode.
     *
     * @param path the absolute path name of the znode
     * @return TRUE if the znode exists; FALSE otherwise
     * @throw ZooKeeperException if the operation has failed
     */
    bool nodeExists(const string &path) throw(ZooKeeperException);

    /**
     * \brief Gets the given node's data.
     *
     * @param path the absolute path name of the node to get data from
     *
     * @return the node's data
     * @throw ZooKeeperException if the operation has failed
     */
    string getNodeData(const string &path) throw(ZooKeeperException);

    /**
     * \brief Sets the given node's data.
     *
     * @param path the absolute path name of the node to get data from
     * @param value the node's data to be set
     * @param version the expected version of the node. The function will
     *                fail if the actual version of the node does not match
     *                the expected version
     *
     * @throw ZooKeeperException if the operation has failed
     */
    void setNodeData(const string &path,
                     const string &value,
                     int version = -1) throw(ZooKeeperException);

    /**
     * \brief Validates the given path to a node in ZK.
     *
     * @param the path to be validated
     *
     * @throw ZooKeeperException if the given path is not valid
     *        (for instance it doesn't start with "/")
     */
    static void validatePath(const string &path) throw(ZooKeeperException);

private:

    /**
     * Verifies whether the connection is established,
     * optionally auto reconnecting.
     *
     * @throw ZooKeeperConnection if this client is disconnected
     *        and auto-reconnect failed or was not allowed
     */
    void verifyConnection() throw(ZooKeeperException);

private:

    /**
     * The current ZK configuration.
     */
    const ZooKeeperConfig zkConfig_;

    /**
     * The current ZK session.
     */
    zhandle_t *zkHandle_;
};


#endif /* __ZK_ADAPTER_H__ */
