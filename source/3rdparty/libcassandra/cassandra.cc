/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include <time.h>
#include <netinet/in.h>

#include <string>
#include <set>
#include <sstream>
#include <iostream>

#include "libcassandra/genthrift/Cassandra.h"

#include "libcassandra/cassandra.h"
#include "libcassandra/exception.h"
#include "libcassandra/indexed_slices_query.h"
#include "libcassandra/keyspace.h"
#include "libcassandra/keyspace_definition.h"
#include "libcassandra/util_functions.h"

using namespace std;
using namespace org::apache::cassandra;
using namespace libcassandra;

Cassandra::Cassandra()
    :thrift_client(NULL),
     host(),
     port(0),
     cluster_name(),
     server_version(),
     current_keyspace(),
     key_spaces(),
     token_map()
{
    reloadKeyspaces();
}

Cassandra::Cassandra(
    CassandraClient *in_thrift_client,
    const string &in_host,
    int in_port
)
    :thrift_client(in_thrift_client),
     host(in_host),
     port(in_port),
     cluster_name(),
     server_version(),
     current_keyspace(),
     key_spaces(),
     token_map()
{
    reloadKeyspaces();
}

Cassandra::Cassandra(
    CassandraClient *in_thrift_client,
    const string &in_host,
    int in_port,
    const string& keyspace
)
    :thrift_client(in_thrift_client),
     host(in_host),
     port(in_port),
     cluster_name(),
     server_version(),
     current_keyspace(keyspace),
     key_spaces(),
     token_map()
{
    reloadKeyspaces();
}

Cassandra::~Cassandra()
{
    delete thrift_client;
}

CassandraClient *Cassandra::getCassandra()
{
    return thrift_client;
}

void Cassandra::login(const string& user, const string& password)
{
    AuthenticationRequest req;
    std::map<std::string, std::string> credentials;
    credentials["username"]= user;
    credentials["password"]= password;
    req.__set_credentials(credentials);
    thrift_client->login(req);
}

void Cassandra::setKeyspace(const string& ks_name)
{
    current_keyspace.assign(ks_name);
    thrift_client->set_keyspace(ks_name);
}

const string& Cassandra::getCurrentKeyspace() const
{
    return current_keyspace;
}

void Cassandra::insertColumn(const string& value,
                             const string& key,
                             const string& column_family,
                             const string& super_column_name,
                             const string& column_name,
                             int64_t time_stamp,
                             ConsistencyLevel::type level,
                             int32_t ttl)
{
    ColumnParent col_parent;
    col_parent.__set_column_family(column_family);
    if (! super_column_name.empty())
    {
        col_parent.__set_super_column(super_column_name);
    }
    if (time_stamp == -1)
    {
        time_stamp = createTimestamp();
    }
    Column col;
    col.__set_name(column_name);
    col.__set_value(value);
    col.__set_timestamp(time_stamp);
    if (ttl)
    {
        col.__set_ttl(ttl);
    }
    /*
     * actually perform the insert
     * TODO - validate the ColumnParent before the insert
     */
    thrift_client->insert(key, col_parent, col, level);
}

void Cassandra::insertColumn(const string& value,
                             const string& key,
                             const string& column_family,
                             const string& column_name,
                             int64_t time_stamp,
                             ConsistencyLevel::type level,
                             int32_t ttl)
{
    insertColumn(value, key, column_family, "", column_name, time_stamp, level, ttl);
}

void Cassandra::insertColumn(const int64_t value,
                             const string& key,
                             const string& column_family,
                             const string& super_column_name,
                             const string& column_name,
                             int64_t time_stamp,
                             ConsistencyLevel::type level,
                             int32_t ttl)
{
    insertColumn(serializeLong(value), key, column_family, super_column_name, column_name, time_stamp, level, ttl);
}

void Cassandra::insertColumn(const int64_t value,
                             const string& key,
                             const string& column_family,
                             const string& column_name,
                             int64_t time_stamp,
                             ConsistencyLevel::type level,
                             int32_t ttl)
{
    insertColumn(serializeLong(value), key, column_family, "", column_name, time_stamp, level, ttl);
}

void Cassandra::remove(const string &key,
                       const ColumnPath &col_path,
                       ConsistencyLevel::type level)
{
    thrift_client->remove(key, col_path, createTimestamp(), level);
}

void Cassandra::remove(const string& key,
                       const string& column_family,
                       const string& super_column_name,
                       const string& column_name,
                       ConsistencyLevel::type level)
{
    ColumnPath col_path;
    col_path.__set_column_family(column_family);
    if (! super_column_name.empty())
    {
        col_path.__set_super_column(super_column_name);
    }
    if (! column_name.empty())
    {
        col_path.__set_column(column_name);
    }
    remove(key, col_path, level);
}

void Cassandra::removeColumn(const string& key,
                       const string& column_family,
                       const string& super_column_name,
                       const string& column_name,
                       ConsistencyLevel::type level)
{
    remove(key, column_family, super_column_name, column_name, level);
}

void Cassandra::removeColumn(const string& key,
                             const string& column_family,
                             const string& column_name,
                             ConsistencyLevel::type level)
{
    remove(key, column_family, "", column_name, level);
}

void Cassandra::removeSuperColumn(const string& key,
                                  const string& column_family,
                                  const string& super_column_name,
                                  ConsistencyLevel::type level)
{
    remove(key, column_family, super_column_name, "", level);
}

Column Cassandra::getColumn(const string& key,
                            const string& column_family,
                            const string& super_column_name,
                            const string& column_name,
                            ConsistencyLevel::type level)
{
    ColumnPath col_path;
    col_path.__set_column_family(column_family);
    if (! super_column_name.empty())
    {
        col_path.__set_super_column(super_column_name);
    }
    col_path.__set_column(column_name);
    ColumnOrSuperColumn cosc;
    /* TODO - validate column path */
    thrift_client->get(cosc, key, col_path, level);
    if (cosc.column.name.empty())
    {
        /* throw an exception */
        throw(InvalidRequestException());
    }
    return cosc.column;
}

Column Cassandra::getColumn(const string& key,
                            const string& column_family,
                            const string& column_name,
                            ConsistencyLevel::type level)
{
    return getColumn(key, column_family, "", column_name, level);
}

string Cassandra::getColumnValue(const string& key,
                                 const string& column_family,
                                 const string& super_column_name,
                                 const string& column_name)
{
    return getColumn(key, column_family, super_column_name, column_name).value;
}

string Cassandra::getColumnValue(const string& key,
                                 const string& column_family,
                                 const string& column_name)
{
    return getColumn(key, column_family, column_name).value;
}

int64_t Cassandra::getIntegerColumnValue(const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name)
{
    string ret= getColumn(key, column_family, super_column_name, column_name).value;
    return deserializeLong(ret);
}

int64_t Cassandra::getIntegerColumnValue(const string& key,
        const string& column_family,
        const string& column_name)
{
    string ret= getColumn(key, column_family, column_name).value;
    return deserializeLong(ret);
}

SuperColumn Cassandra::getSuperColumn(const string& key,
                                      const string& column_family,
                                      const string& super_column_name,
                                      ConsistencyLevel::type level)
{
    ColumnPath col_path;
    col_path.__set_column_family(column_family);
    col_path.__set_super_column(super_column_name);
    ColumnOrSuperColumn cosc;
    /* TODO - validate super column path */
    thrift_client->get(cosc, key, col_path, level);
    if (cosc.super_column.name.empty())
    {
        /* throw an exception */
        throw(InvalidRequestException());
    }
    return cosc.super_column;
}

vector<Column> Cassandra::getSliceNames(const string& key,
                                        const ColumnParent& col_parent,
                                        SlicePredicate& pred,
                                        ConsistencyLevel::type level)
{
    vector<ColumnOrSuperColumn> ret_cosc;
    vector<Column> result;
    /* damn you thrift! */
    pred.__isset.column_names= true;
    thrift_client->get_slice(ret_cosc, key, col_parent, pred, level);
    for (vector<ColumnOrSuperColumn>::const_iterator it= ret_cosc.begin();
            it != ret_cosc.end();
            ++it)
    {
        if (! it->column.name.empty())
        {
            result.push_back(it->column);
        }
    }
    return result;
}

vector<Column> Cassandra::getSliceRange(const string& key,
                                        const ColumnParent& col_parent,
                                        SlicePredicate& pred,
                                        ConsistencyLevel::type level)
{
    vector<ColumnOrSuperColumn> ret_cosc;
    vector<Column> result;
    /* damn you thrift! */
    pred.__isset.slice_range= true;
    thrift_client->get_slice(ret_cosc, key, col_parent, pred, level);
    for (vector<ColumnOrSuperColumn>::const_iterator it= ret_cosc.begin();
            it != ret_cosc.end();
            ++it)
    {
        if (! it->column.name.empty())
        {
            result.push_back(it->column);
        }
    }
    return result;
}

map<string, vector<Column> > Cassandra::getRangeSlice(const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const string& start,
        const string& finish,
        const int32_t row_count,
        ConsistencyLevel::type level)
{
    map<string, vector<Column> > ret;
    vector<KeySlice> key_slices;
    KeyRange key_range;
    key_range.__set_start_key(start);
    key_range.__set_end_key(finish);
    key_range.__set_count(row_count);
    thrift_client->get_range_slices(key_slices,
                                    col_parent,
                                    pred,
                                    key_range,
                                    level);
    if (! key_slices.empty())
    {
        for (vector<KeySlice>::const_iterator it= key_slices.begin();
                it != key_slices.end();
                ++it)
        {
            ret.insert(make_pair(it->key, getColumnList(it->columns)));
        }
    }
    return ret;
}

map<string, vector<SuperColumn> > Cassandra::getSuperRangeSlice(const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const string& start,
        const string& finish,
        const int32_t row_count,
        ConsistencyLevel::type level)
{
    map<string, vector<SuperColumn> > ret;
    vector<KeySlice> key_slices;
    KeyRange key_range;
    key_range.__set_start_key(start);
    key_range.__set_end_key(finish);
    key_range.__set_count(row_count);
    thrift_client->get_range_slices(key_slices,
                                    col_parent,
                                    pred,
                                    key_range,
                                    level);
    if (! key_slices.empty())
    {
        for (vector<KeySlice>::const_iterator it= key_slices.begin();
                it != key_slices.end();
                ++it)
        {
            ret.insert(make_pair(it->key, getSuperColumnList(it->columns)));
        }
    }
    return ret;
}

map<string, map<string, string> >
Cassandra::getIndexedSlices(const IndexedSlicesQuery& query)
{
    map<string, map<string, string> > ret_map;
    vector<KeySlice> ret;
    SlicePredicate thrift_slice_pred= createSlicePredicateObject(query);
    ColumnParent thrift_col_parent;
    thrift_col_parent.column_family.assign(query.getColumnFamily());
    thrift_client->get_indexed_slices(ret,
                                      thrift_col_parent,
                                      query.getIndexClause(),
                                      thrift_slice_pred,
                                      query.getConsistencyLevel());

    for (vector<KeySlice>::const_iterator it= ret.begin();
            it != ret.end();
            ++it)
    {
        vector<Column> thrift_cols= getColumnList(it->columns);
        map<string, string> rows;
        for (vector<Column>::const_iterator inner_it= thrift_cols.begin();
                inner_it != thrift_cols.end();
                ++inner_it)
        {
            rows.insert(make_pair(inner_it->name, inner_it->value));
        }
        ret_map.insert(make_pair(it->key, rows));
    }

    return ret_map;
}

int32_t Cassandra::getCount(const string& key,
                            const ColumnParent& col_parent,
                            const SlicePredicate& pred,
                            ConsistencyLevel::type level)
{
    return (thrift_client->get_count(key, col_parent, pred, level));
}

void Cassandra::reloadKeyspaces()
{
    key_spaces.clear();

    vector<KsDef> thrift_ks_defs;
    thrift_client->describe_keyspaces(thrift_ks_defs);
    bool current_valid = false;

    for (vector<KsDef>::const_iterator it= thrift_ks_defs.begin();
            it != thrift_ks_defs.end();
            ++it)
    {
        const KsDef& thrift_entry= *it;
        if (thrift_entry.name == current_keyspace)
            current_valid = true;

        KeyspaceDefinition entry(thrift_entry.name,
                thrift_entry.strategy_class,
                thrift_entry.strategy_options,
                thrift_entry.replication_factor,
                thrift_entry.cf_defs);
        key_spaces.push_back(entry);
    }
    if (!current_valid)
        current_keyspace.clear();
}

const vector<KeyspaceDefinition>& Cassandra::getKeyspaces()
{
    return key_spaces;
}

string Cassandra::createColumnFamily(const ColumnFamilyDefinition& cf_def)
{
    string schema_id;
    CfDef thrift_cf_def= createCfDefObject(cf_def);
    thrift_client->system_add_column_family(schema_id, thrift_cf_def);
    return schema_id;
}

string Cassandra::dropColumnFamily(const string& cf_name)
{
    string schema_id;
    thrift_client->system_drop_column_family(schema_id, cf_name);
    return schema_id;
}

std::string Cassandra::updateColumnFamily(const ColumnFamilyDefinition& cf_def)
{
    string schema_id;
    CfDef thrift_cf_def= createCfDefObject(cf_def);
    thrift_client->system_update_column_family(schema_id, thrift_cf_def);
    return schema_id;
}

string Cassandra::createKeyspace(const KeyspaceDefinition& ks_def)
{
    string ret;
    KsDef thrift_ks_def= createKsDefObject(ks_def);
    thrift_client->system_add_keyspace(ret, thrift_ks_def);
    if (!findKeyspace(ks_def.getName()))
    {
        key_spaces.push_back(ks_def);
    }
    return ret;
}

void Cassandra::truncateColumnFamily(const string& cf_name)
{
    thrift_client->truncate(cf_name);
}

void Cassandra::executeCqlQuery(CqlResult& result, const string& query, Compression::type compression)
{
    thrift_client->execute_cql_query(result, query, compression);
}

string Cassandra::dropKeyspace(const string& ks_name)
{
    string ret;
    thrift_client->system_drop_keyspace(ret, ks_name);
    for (vector<KeyspaceDefinition>::iterator it= key_spaces.begin();
            it != key_spaces.end();
            ++it)
    {
        if (ks_name == it->getName())
        {
            key_spaces.erase(it);
            break;
        }
    }
    if (current_keyspace == ks_name)
        current_keyspace.clear();

    return ret;
}

const string& Cassandra::getClusterName()
{
    if (cluster_name.empty())
    {
        thrift_client->describe_cluster_name(cluster_name);
    }
    return cluster_name;
}

const string& Cassandra::getServerVersion()
{
    if (server_version.empty())
    {
        thrift_client->describe_version(server_version);
    }
    return server_version;
}

const string& Cassandra::getHost() const
{
    return host;
}

int Cassandra::getPort() const
{
    return port;
}

bool Cassandra::findKeyspace(const string& name) const
{
    for (vector<KeyspaceDefinition>::const_iterator it= key_spaces.begin();
            it != key_spaces.end();
            ++it)
    {
        if (name == it->getName())
        {
            return true;
        }
    }
    return false;
}
