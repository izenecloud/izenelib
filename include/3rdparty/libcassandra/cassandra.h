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

#include <boost/shared_ptr.hpp>

#include "genthrift/cassandra_types.h"

#include "indexed_slices_query.h"

namespace libcassandra
{

class Keyspace;
class Cassandra
{
public:
    Cassandra();

    Cassandra(const std::string& keyspace);

    ~Cassandra();

    enum FailoverPolicy
    {
        FAIL_FAST= 0, /* return error as is to user */
        ON_FAIL_TRY_ONE_NEXT_AVAILABLE, /* try 1 random server before returning to user */
        ON_FAIL_TRY_ALL_AVAILABLE /* try all available servers in cluster before return to user */
    };

    /**
     * Log for the current session
     * @param[in] credentials to use for authentication
     */
    void login(const std::map<std::string, std::string>& credentials);

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
     * @return the keyspace definition associated with this session
     */
    const org::apache::cassandra::KsDef& getCurrentKeyspaceDefinition() const;

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
    const std::vector<org::apache::cassandra::KsDef>& getKeyspaces();

    /**
     * Insert a column, possibly inside a supercolumn
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] ttl time to live
     * @param[in] level consistency level
     */
    void insertColumn(
            const std::string& value,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            const int64_t time_stamp = -1,
            int32_t ttl = 0,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Insert a column, directly in a columnfamily
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] ttl time to live
     * @param[in] level consistency level
     */
    void insertColumn(
            const std::string& value,
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name,
            const int64_t time_stamp = -1,
            int32_t ttl = 0,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Insert a column, possibly inside a supercolumn
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] ttl time to live
     * @param[in] level consistency level
     */
    void insertColumn(
            const int64_t value,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            const int64_t time_stamp = -1,
            int32_t ttl = 0,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Insert a column, directly in a columnfamily
     *
     * @param[in] value the column value
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] ttl time to live
     * @param[in] level consistency level
     */
    void insertColumn(
            const int64_t value,
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name,
            const int64_t time_stamp = -1,
            int32_t ttl = 0,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Increment a counter column, possibly inside a supercolumn
     *
     * @param[in] value the counter column value
     * @param[in] key the counter column key
     * @param[in] counter_column_family the counter column family
     * @param[in] counter_super_column_name the counter super column name
     * @param[in] counter_column_name the counter column name
     * @param[in] level consistency level
     */
    void incCounter(
            int64_t value,
            const std::string& key,
            const std::string& counter_column_family,
            const std::string& counter_super_column_name,
            const std::string& counter_column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Increment a counter column
     *
     * @param[in] value the counter column value
     * @param[in] key the counter column key
     * @param[in] counter_column_family the counter column family
     * @param[in] counter_column_name the counter column name
     * @param[in] level consistency level
     */
    void incCounter(
            int64_t value,
            const std::string& key,
            const std::string& counter_column_family,
            const std::string& counter_column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Removes all the columns that match the given column path
     *
     * @param[in] key the column or super column key
     * @param[in] col_path the path to the column or super column
     * @param[in] is_counter whether to remove counter or normal columns
     * @param[in] level consistency level
     */
    void remove(
            const std::string& key,
            const org::apache::cassandra::ColumnPath& col_path,
            bool is_counter = false,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Removes all the columns that match the given arguments
     * Can remove all under a column family, an individual column or supercolumn under a column family, or an individual column under a supercolumn
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] is_counter whether to remove counter or normal columns
     * @param[in] level consistency level
     */
    void remove(
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            bool is_counter = false,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);


    /**
     * Remove a column, possibly inside a supercolumn
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] column_name the column name
     * @param[in] is_counter whether to remove counter or normal columns
     * @param[in] level consistency level
     */
    void removeColumn(
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            bool is_counter = false,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);


    /**
     * Remove a column
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name
     * @param[in] is_counter whether to remove counter or normal columns
     * @param[in] level consistency level
     */
    void removeColumn(
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name,
            bool is_counter = false,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Remove a super column and all columns under it
     *
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name
     * @param[in] is_counter whether to remove counter or normal columns
     * @param[in] level consistency level
     */
    void removeSuperColumn(
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            bool is_counter = false,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Rertieve a column.
     *
     * @param[in] column the returned column
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     * @param[in] level consistency level
     */
    void getColumn(
            org::apache::cassandra::Column& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Retrieve a column
     *
     * @param[in] column the returned column
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     * @param[in] level consistency level
     */
    void getColumn(
            org::apache::cassandra::Column& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Retrieve a column value
     *
     * @param[in] ret the value for the column that corresponds to the given parameters
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     */
    void getColumnValue(
            std::string& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] ret the value for the column that corresponds to the given parameters
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     */
    void getColumnValue(
            std::string& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] ret the value for the column that corresponds to the given parameters
     *            but as an integer
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] super_column_name the super column name (optional)
     * @param[in] column_name the column name (optional)
     */
    void getIntegerColumnValue(
            int64_t& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const std::string& column_name);

    /**
     * Retrieve a column value
     *
     * @param[in] ret the value for the column that corresponds to the given parameters
     *            but as an integer
     * @param[in] key the column key
     * @param[in] column_family the column family
     * @param[in] column_name the column name (optional)
     */
    void getIntegerColumnValue(
            int64_t& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& column_name);

    void getSuperColumn(
            org::apache::cassandra::SuperColumn& ret,
            const std::string& key,
            const std::string& column_family,
            const std::string& super_column_name,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getRawSlice(
            std::vector<org::apache::cassandra::ColumnOrSuperColumn>& ret,
            const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getSlice(
            std::vector<org::apache::cassandra::Column>& ret,
            const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getSuperSlice(
            std::vector<org::apache::cassandra::SuperColumn>& ret,
            const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getMultiSlice(
            std::map<std::string, std::vector<org::apache::cassandra::ColumnOrSuperColumn> >& ret,
            const std::vector<std::string>& key_list,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getRawRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::ColumnOrSuperColumn> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::KeyRange& range,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getRawRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::ColumnOrSuperColumn> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const std::string& start,
            const std::string& finish,
            const int32_t row_count,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::Column> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::KeyRange& range,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::Column> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const std::string& start,
            const std::string& finish,
            const int32_t row_count,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getSuperRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::SuperColumn> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::KeyRange& range,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getSuperRangeSlices(
            std::map<std::string, std::vector<org::apache::cassandra::SuperColumn> >& ret,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const std::string& start,
            const std::string& finish,
            const int32_t count,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Get a list of slices using the given query object
     * @param[in] ret a map of row keys to column names and values
     * @param[in] query object that encapuslates everything needed
     *                  for a query using secondary indexes
     */
    void getIndexedSlices(
            std::map<std::string, std::map<std::string, std::string> >& ret,
            const IndexedSlicesQuery& query);

    /**
     * @return number of columns in a row or super column
     */
    int32_t getCount(
            const std::string& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    void getMultiCount(
            std::map<std::string, int32_t>& ret,
            const std::vector<std::string>& key,
            const org::apache::cassandra::ColumnParent& col_parent,
            const org::apache::cassandra::SlicePredicate& pred,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

    /**
     * Create a keyspace
     * @param[in] ks_def object representing defintion for keyspace to create
     * @return the schema ID for the keyspace created
     */
    std::string createKeyspace(const org::apache::cassandra::KsDef& ks_def);

    /**
     * Update a keyspace
     * @param[in] ks_def object representing defintion for keyspace to update
     * @return the schema ID for the keyspace updated
     */
    std::string updateKeyspace(const org::apache::cassandra::KsDef& ks_def);

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
    std::string createColumnFamily(const org::apache::cassandra::CfDef& cf_def);

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
    std::string updateColumnFamily(const org::apache::cassandra::CfDef& cf_def);

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
    void executeCqlQuery(
            org::apache::cassandra::CqlResult& result,
            const std::string& query,
            const org::apache::cassandra::Compression::type compression = org::apache::cassandra::Compression::GZIP);

    void batchMutate(
            const std::map<std::string, std::map<std::string, std::vector<org::apache::cassandra::Mutation> > >& mutation_map,
            const org::apache::cassandra::ConsistencyLevel::type level = org::apache::cassandra::ConsistencyLevel::QUORUM);

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
     * Finds the given keyspace in the list of keyspace definitions
     * @return true if found; false otherwise
     */
    uint32_t findKeyspace(const std::string& name) const;

    /**
     * Finds the given column family in the given keyspace definition
     * @return true if found; false otherwise
     */
    uint32_t findColumnFamily(uint32_t ks_num, const std::string& name) const;

private:
    std::string cluster_name_;
    std::string server_version_;
    std::string current_keyspace_;
    uint32_t current_ks_num_;
    std::vector<org::apache::cassandra::KsDef> key_spaces_;
    std::map<std::string, std::string> token_map_;
};

} /* end namespace libcassandra */

#endif /* __LIBCASSANDRA_CASSANDRA_H */
