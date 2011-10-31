/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#ifndef __LIBCASSANDRA_CASSANDRA_H
#define __LIBCASSANDRA_CASSANDRA_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>

#include "genthrift/cassandra_types.h"

#include "indexed_slices_query.h"
#include "keyspace_definition.h"

namespace org
{
namespace apache
{
namespace cassandra
{
class CassandraClient;
}
}
}

namespace libcassandra
{

class Keyspace;

class Cassandra
{

public:

    Cassandra();
    Cassandra(org::apache::cassandra::CassandraClient *in_thrift_client,
              const std::string &in_host,
              int in_port);
    Cassandra(org::apache::cassandra::CassandraClient *in_thrift_client,
              const std::string &in_host,
              int in_port,
              const std::string& keyspace);
    ~Cassandra();

    enum FailoverPolicy
    {
        FAIL_FAST= 0, /* return error as is to user */
        ON_FAIL_TRY_ONE_NEXT_AVAILABLE, /* try 1 random server before returning to user */
        ON_FAIL_TRY_ALL_AVAILABLE /* try all available servers in cluster before return to user */
    };

    /**
     * @return the underlying cassandra thrift client.
     */
    org::apache::cassandra::CassandraClient *getCassandra();

    /**
     * Log for the current session
     * @param[in] user to use for authentication
     * @param[in] password to use for authentication
     */
    void login(const std::string& user, const std::string& password);

    /**
     * @return the keyspace associated with this session
     */
    const std::string& getCurrentKeyspace() const;

    /**
     * set the keyspace for the current connection
     * @param[in] ks_name name of the keyspace to specify for current session
     */
    void setKeyspace(const std::string& ks_name);

    /**
     * reload all the keyspace definitions
     */
    void reloadKeyspaces();

    /**
     * @return all the keyspace definitions.
     */
    const std::vector<KeyspaceDefinition>& getKeyspaces();

    /**
     * Insert a column, possibly inside a supercolumn
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] level consistency level
     * @param[in] ttl time to live
     */
    void insertColumn(const std::string& value,
                      const std::string& key,
                      const std::string& column_family,
                      const std::string& super_column_name,
                      const std::string& column_name,
                      const int64_t time_stamp = -1,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM,
                      int32_t ttl = 0);

    /**
     * Insert a column, directly in a columnfamily
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] level consistency level
     * @param[in] ttl time to live
     */
    void insertColumn(const std::string& value,
                      const std::string& key,
                      const std::string& column_family,
                      const std::string& column_name,
                      const int64_t time_stamp = -1,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM,
                      int32_t ttl = 0);

    /**
     * Insert a column, possibly inside a supercolumn
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] level consistency level
     * @param[in] ttl time to live
     */
    void insertColumn(const int64_t value,
                      const std::string& key,
                      const std::string& column_family,
                      const std::string& super_column_name,
                      const std::string& column_name,
                      const int64_t time_stamp = -1,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM,
                      int32_t ttl = 0);

    /**
     * Insert a column, directly in a columnfamily
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] level consistency level
     * @param[in] ttl time to live
     */
    void insertColumn(const int64_t value,
                      const std::string& key,
                      const std::string& column_family,
                      const std::string& column_name,
                      const int64_t time_stamp = -1,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM,
                      int32_t ttl = 0);

    /**
     * Removes all the columns that match the given column path
     *
     * @param[in] key the column or super column key
     * @param[in] col_path the path to the column or super column
     * @param[in] level consistency level
     */
    void remove(const std::string& key,
                const org::apache::cassandra::ColumnPath& col_path,
                const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Removes all the columns that match the given arguments
     * Can remove all under a column family, an individual column or supercolumn under a column family, or an individual column under a supercolumn
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] level consistency level
     */
    void remove(const std::string& key,
                      const std::string& column_family,
                      const std::string& super_column_name,
                      const std::string& column_name,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);


    /**
     * Remove a column, possibly inside a supercolumn
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] level consistency level
     */
    void removeColumn(const std::string& key,
                      const std::string& column_family,
                      const std::string& super_column_name,
                      const std::string& column_name,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);


    /**
     * Remove a column
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] level consistency level
     */
    void removeColumn(const std::string& key,
                      const std::string& column_family,
                      const std::string& column_name,
                      const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Remove a super column and all columns under it
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] level consistency level
     */
    void removeSuperColumn(const std::string& key,
                           const std::string& column_family,
                           const std::string& super_column_name,
                           const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Rertieve a column.
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     * @param[in] level consistency level
     * @return a column
     */
    org::apache::cassandra::Column getColumn(const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Retrieve a column
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     * @param[in] level consistency level
     * @return a column
     */
    org::apache::cassandra::Column getColumn(const std::string& key,
            const std::string& column_family,
            const std::string& column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Retrieve a column value
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     * @return the value for the column that corresponds to the given parameters
     */
    std::string getColumnValue(const std::string& key,
                               const std::string& column_family,
                               const std::string& super_column_name,
                               const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     * @return the value for the column that corresponds to the given parameters
     */
    std::string getColumnValue(const std::string& key,
                               const std::string& column_family,
                               const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     * @return the value for the column that corresponds to the given parameters
     *         but as an integer
     */
    int64_t getIntegerColumnValue(const std::string& key,
                                  const std::string& column_family,
                                  const std::string& super_column_name,
                                  const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     * @return the value for the column that corresponds to the given parameters
     *         but as an integer
     */
    int64_t getIntegerColumnValue(const std::string& key,
                                  const std::string& column_family,
                                  const std::string& column_name);

    org::apache::cassandra::SuperColumn getSuperColumn(const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    std::vector<org::apache::cassandra::Column> getSliceNames(const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    std::vector<org::apache::cassandra::Column> getSliceRange(const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    std::map<std::string, std::vector<org::apache::cassandra::Column> >
    getRangeSlice(const org::apache::cassandra::ColumnParent& col_parent,
                  const org::apache::cassandra::SlicePredicate& pred,
                  const std::string& start,
                  const std::string& finish,
                  const int32_t row_count,
                  const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    std::map<std::string, std::vector<org::apache::cassandra::SuperColumn> >
    getSuperRangeSlice(const org::apache::cassandra::ColumnParent& col_parent,
                       const org::apache::cassandra::SlicePredicate& pred,
                       const std::string& start,
                       const std::string& finish,
                       const int32_t count,
                       const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Return a list of slices using the given query object
     * @param[in] query object that encapuslates everything needed
     *                  for a query using secondary indexes
     * @return a map of row keys to column names and values
     */
    std::map<std::string, std::map<std::string, std::string> >
    getIndexedSlices(const IndexedSlicesQuery& query);

    /**
     * @return number of columns in a row or super column
     */
    int32_t getCount(const std::string& key,
                     const org::apache::cassandra::ColumnParent& col_parent,
                     const org::apache::cassandra::SlicePredicate& pred,
                     const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Create a column family
     * @param[in] cf_def object representing defintion for column family to create
     * @return the schema ID for the keyspace created
     */
    std::string createKeyspace(const KeyspaceDefinition& ks_def);

    /**
     * drop a keyspace
     * @param[in] ks_name the name of the keyspace to drop
     * @return the schema ID for the keyspace dropped
     */
    std::string dropKeyspace(const std::string& ks_name);

    /**
     * Create a column family
     * @param[in] cf_name the name of the column family to create
     * @return the schema ID for the column family created
     */
    std::string createColumnFamily(const ColumnFamilyDefinition& cf_def);

    /**
     * Drop a column family
     * @param[in] cf_name the name of the column family to drop
     * @return the schema ID for the column family dropped
     */
    std::string dropColumnFamily(const std::string& cf_name);

    /**
     * Update a column family
     * @param[in] cf_name the name of the column family to update
     * @return the schema ID for the column family updated
     */
    std::string updateColumnFamily(const ColumnFamilyDefinition& cf_def);

    /**
     * Removes all the rows from the given column family.
     * @param[in] cf_name the name of the column family to truncate
     */
    void truncateColumnFamily(const std::string& cf_name);

    /**
     * Execute the CQL query.
     * @param[in] result the result returned for the query
     * @param[in] query the CQL sentence
     * @param[in] compression the compression type
     */
    void executeCqlQuery(org::apache::cassandra::CqlResult& result, const std::string& query, const org::apache::cassandra::Compression::type compression = org::apache::cassandra::Compression::NONE);

    /**
     * @return the target server cluster name.
     */
    const std::string& getClusterName();

    /**
     * @return the server version.
     */
    const std::string& getServerVersion();

    /**
     * @return a string property from the server
     */
    void getStringProperty(std::string &return_val, const std::string &property);

    /**
     * @return hostname
     */
    const std::string& getHost() const;

    /**
     * @return port number
     */
    int getPort() const;

    /**
     * Finds the given keyspace in the list of keyspace definitions
     * @return true if found; false otherwise
     */
    bool findKeyspace(const std::string& name) const;

private:

    org::apache::cassandra::CassandraClient *thrift_client;
    std::string host;
    int port;
    std::string cluster_name;
    std::string server_version;
    std::string current_keyspace;
    std::vector<KeyspaceDefinition> key_spaces;
    std::map<std::string, std::string> token_map;

    Cassandra(const Cassandra&);
    Cassandra &operator=(const Cassandra&);

};

} /* end namespace libcassandra */

#endif /* __LIBCASSANDRA_CASSANDRA_H */
