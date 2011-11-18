/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include "libcassandra/cassandra.h"
#include "libcassandra/connection_manager.h"
#include "libcassandra/cassandra_client.h"
#include "libcassandra/util_functions.h"

using namespace std;
using namespace org::apache::cassandra;

namespace libcassandra
{

#define BORROW_CLIENT \
    MyCassandraClient* client = CassandraConnectionManager::instance()->borrowClient(); \
    org::apache::cassandra::CassandraClient* thrift_client = client->getCassandra(current_keyspace_); \

#define RELEASE_CLIENT \
    CassandraConnectionManager::instance()->releaseClient(client); \


Cassandra::Cassandra()
    : cluster_name_()
    , server_version_()
    , current_keyspace_()
    , current_ks_num_(-1)
    , key_spaces_()
    , token_map_()
{
}

Cassandra::Cassandra(const string& keyspace)
    : cluster_name_()
    , server_version_()
    , current_keyspace_(keyspace)
    , current_ks_num_(findKeyspace(keyspace))
    , key_spaces_()
    , token_map_()
{
}

Cassandra::~Cassandra()
{}

void Cassandra::login(const map<string, string>& credentials)
{
    AuthenticationRequest req;
    req.__set_credentials(credentials);
    BORROW_CLIENT
    thrift_client->login(req);
    RELEASE_CLIENT
}

void Cassandra::login(const string& user, const string& password)
{
    AuthenticationRequest req;
    map<string, string> credentials;
    credentials["username"]= user;
    credentials["password"]= password;
    req.__set_credentials(credentials);
    BORROW_CLIENT
    thrift_client->login(req);
    RELEASE_CLIENT
}

void Cassandra::setKeyspace(const string& ks_name)
{
    current_keyspace_.assign(ks_name);
    current_ks_num_ = findKeyspace(ks_name);
}

const string& Cassandra::getCurrentKeyspace() const
{
    return current_keyspace_;
}

const KsDef& Cassandra::getCurrentKeyspaceDefinition() const
{
    return key_spaces_.at(current_ks_num_);
}

void Cassandra::insertColumn(
        const string& value,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name,
        int64_t time_stamp,
        int32_t ttl,
        const ConsistencyLevel::type level)
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
    BORROW_CLIENT
    thrift_client->insert(key, col_parent, col, level);
    RELEASE_CLIENT
}

void Cassandra::insertColumn(
        const string& value,
        const string& key,
        const string& column_family,
        const string& column_name,
        int64_t time_stamp,
        int32_t ttl,
        const ConsistencyLevel::type level)
{
    insertColumn(value, key, column_family, "", column_name, time_stamp, ttl, level);
}

void Cassandra::insertColumn(
        const int64_t value,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name,
        int64_t time_stamp,
        int32_t ttl,
        const ConsistencyLevel::type level)
{
    insertColumn(serializeLong(value), key, column_family, super_column_name, column_name, time_stamp, ttl, level);
}

void Cassandra::insertColumn(
        const int64_t value,
        const string& key,
        const string& column_family,
        const string& column_name,
        int64_t time_stamp,
        int32_t ttl,
        const ConsistencyLevel::type level)
{
    insertColumn(serializeLong(value), key, column_family, "", column_name, time_stamp, ttl, level);
}

void Cassandra::incCounter(
        int64_t value,
        const std::string& key,
        const std::string& counter_column_family,
        const std::string& counter_super_column_name,
        const std::string& counter_column_name,
        const org::apache::cassandra::ConsistencyLevel::type level)
{
    ColumnParent col_parent;
    col_parent.__set_column_family(counter_column_family);
    if (! counter_super_column_name.empty())
    {
        col_parent.__set_super_column(counter_super_column_name);
    }
    CounterColumn col;
    col.__set_name(counter_column_name);
    col.__set_value(value);
    /*
     * actually perform the insert
     * TODO - validate the ColumnParent before the insert
     */
    BORROW_CLIENT
    thrift_client->add(key, col_parent, col, level);
    RELEASE_CLIENT
}

void Cassandra::incCounter(
        int64_t value,
        const std::string& key,
        const std::string& counter_column_family,
        const std::string& counter_column_name,
        const org::apache::cassandra::ConsistencyLevel::type level)
{
    incCounter(value, key, counter_column_family, "", counter_column_name, level);
}

void Cassandra::remove(
        const string &key,
        const ColumnPath &col_path,
        bool is_counter,
        const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    if (is_counter)
    {
        thrift_client->remove_counter(key, col_path, level);
    }
    else
    {
        thrift_client->remove(key, col_path, createTimestamp(), level);
    }
    RELEASE_CLIENT
}

void Cassandra::remove(
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name,
        bool is_counter,
        const ConsistencyLevel::type level)
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
    remove(key, col_path, is_counter, level);
}

void Cassandra::removeColumn(
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name,
        bool is_counter,
        const ConsistencyLevel::type level)
{
    remove(key, column_family, super_column_name, column_name, is_counter, level);
}

void Cassandra::removeColumn(
        const string& key,
        const string& column_family,
        const string& column_name,
        bool is_counter,
        const ConsistencyLevel::type level)
{
    remove(key, column_family, "", column_name, is_counter, level);
}

void Cassandra::removeSuperColumn(
        const string& key,
        const string& column_family,
        const string& super_column_name,
        bool is_counter,
        const ConsistencyLevel::type level)
{
    remove(key, column_family, super_column_name, "", is_counter, level);
}

void Cassandra::getColumn(
        Column& ret,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name,
        const ConsistencyLevel::type level)
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
    BORROW_CLIENT
    thrift_client->get(cosc, key, col_path, level);
    RELEASE_CLIENT

    if (cosc.column.name.empty())
    {
        /* throw an exception */
        throw(InvalidRequestException());
    }
    swap(ret, cosc.column);
}

void Cassandra::getColumn(
        Column& ret,
        const string& key,
        const string& column_family,
        const string& column_name,
        const ConsistencyLevel::type level)
{
    getColumn(ret, key, column_family, "", column_name, level);
}

void Cassandra::getColumnValue(
        string& ret,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name)
{
    Column column;
    getColumn(column, key, column_family, super_column_name, column_name);
    ret.swap(column.value);
}

void Cassandra::getColumnValue(
        string& ret,
        const string& key,
        const string& column_family,
        const string& column_name)
{
    getColumnValue(ret, key, column_family, "", column_name);
}

void Cassandra::getIntegerColumnValue(
        int64_t& ret,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const string& column_name)
{
    Column column;
    getColumn(column, key, column_family, super_column_name, column_name);
    ret = deserializeLong(column.value);
}

void Cassandra::getIntegerColumnValue(
        int64_t& ret,
        const string& key,
        const string& column_family,
        const string& column_name)
{
    getIntegerColumnValue(ret, key, column_family, "", column_name);
}

void Cassandra::getSuperColumn(
        SuperColumn& ret,
        const string& key,
        const string& column_family,
        const string& super_column_name,
        const ConsistencyLevel::type level)
{
    ColumnPath col_path;
    col_path.__set_column_family(column_family);
    col_path.__set_super_column(super_column_name);
    ColumnOrSuperColumn cosc;
    /* TODO - validate super column path */
    BORROW_CLIENT
    thrift_client->get(cosc, key, col_path, level);
    RELEASE_CLIENT

    if (cosc.super_column.name.empty())
    {
        /* throw an exception */
        throw(InvalidRequestException());
    }
    swap(ret, cosc.super_column);
}

void Cassandra::getRawSlice(
        vector<ColumnOrSuperColumn>& ret,
        const string& key,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    thrift_client->get_slice(ret, key, col_parent, pred, level);
    RELEASE_CLIENT
}

void Cassandra::getSlice(
        vector<Column>& ret,
        const string& key,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    vector<ColumnOrSuperColumn> ret_cosc;
    BORROW_CLIENT
    thrift_client->get_slice(ret_cosc, key, col_parent, pred, level);
    RELEASE_CLIENT

    for (vector<ColumnOrSuperColumn>::const_iterator it= ret_cosc.begin();
            it != ret_cosc.end();
            ++it)
    {
        if (! it->column.name.empty())
        {
            ret.push_back(it->column);
        }
    }
}

void Cassandra::getSuperSlice(
        vector<SuperColumn>& ret,
        const string& key,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    vector<ColumnOrSuperColumn> ret_cosc;
    BORROW_CLIENT
    thrift_client->get_slice(ret_cosc, key, col_parent, pred, level);
    RELEASE_CLIENT

    for (vector<ColumnOrSuperColumn>::const_iterator it= ret_cosc.begin();
            it != ret_cosc.end();
            ++it)
    {
        if (! it->super_column.name.empty())
        {
            ret.push_back(it->super_column);
        }
    }
}

void Cassandra::getMultiSlice(
        map<string, vector<ColumnOrSuperColumn> >& ret,
        const vector<string>& key_list,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    thrift_client->multiget_slice(ret, key_list, col_parent, pred, level);
    RELEASE_CLIENT
}

void Cassandra::getRawRangeSlices(
        map<string, vector<ColumnOrSuperColumn> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const KeyRange& range,
        const ConsistencyLevel::type level)
{
    vector<KeySlice> key_slices;
    BORROW_CLIENT
    thrift_client->get_range_slices(key_slices,
            col_parent,
            pred,
            range,
            level);
    RELEASE_CLIENT

    if (! key_slices.empty())
    {
        for (vector<KeySlice>::const_iterator it= key_slices.begin();
                it != key_slices.end();
                ++it)
        {
            ret[it->key] = it->columns;
        }
    }
}

void Cassandra::getRawRangeSlices(
        map<string, vector<ColumnOrSuperColumn> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const string& start,
        const string& finish,
        const int32_t row_count,
        const ConsistencyLevel::type level)
{
    KeyRange key_range;
    key_range.__set_start_key(start);
    key_range.__set_end_key(finish);
    key_range.__set_count(row_count);
    getRawRangeSlices(ret, col_parent, pred, key_range, level);
}

void Cassandra::getRangeSlices(
        map<string, vector<Column> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const KeyRange& range,
        const ConsistencyLevel::type level)
{
    vector<KeySlice> key_slices;
    BORROW_CLIENT
    thrift_client->get_range_slices(key_slices,
            col_parent,
            pred,
            range,
            level);
    RELEASE_CLIENT

    if (! key_slices.empty())
    {
        for (vector<KeySlice>::const_iterator it= key_slices.begin();
                it != key_slices.end();
                ++it)
        {
            vector<Column>& thrift_cols = ret[it->key];
            getColumnList(thrift_cols, it->columns);
        }
    }
}

void Cassandra::getRangeSlices(
        map<string, vector<Column> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const string& start,
        const string& finish,
        const int32_t row_count,
        const ConsistencyLevel::type level)
{
    KeyRange key_range;
    key_range.__set_start_key(start);
    key_range.__set_end_key(finish);
    key_range.__set_count(row_count);
    getRangeSlices(ret, col_parent, pred, key_range, level);
}

void Cassandra::getSuperRangeSlices(
        map<string, vector<SuperColumn> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const KeyRange& range,
        const ConsistencyLevel::type level)
{
    vector<KeySlice> key_slices;
    BORROW_CLIENT
    thrift_client->get_range_slices(key_slices,
                                    col_parent,
                                    pred,
                                    range,
                                    level);
    RELEASE_CLIENT

    if (! key_slices.empty())
    {
        for (vector<KeySlice>::const_iterator it= key_slices.begin();
                it != key_slices.end();
                ++it)
        {
            vector<SuperColumn>& thrift_cols = ret[it->key];
            getSuperColumnList(thrift_cols, it->columns);
        }
    }
}

void Cassandra::getSuperRangeSlices(
        map<string, vector<SuperColumn> >& ret,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const string& start,
        const string& finish,
        const int32_t row_count,
        const ConsistencyLevel::type level)
{
    KeyRange key_range;
    key_range.__set_start_key(start);
    key_range.__set_end_key(finish);
    key_range.__set_count(row_count);
    getSuperRangeSlices(ret, col_parent, pred, key_range, level);
}

void Cassandra::getIndexedSlices(
        map<string, map<string, string> >& ret,
        const IndexedSlicesQuery& query)
{
    vector<KeySlice> ret_vec;
    SlicePredicate thrift_slice_pred;
    createSlicePredicateObject(thrift_slice_pred, query);
    ColumnParent thrift_col_parent;
    thrift_col_parent.column_family.assign(query.getColumnFamily());
    BORROW_CLIENT
    thrift_client->get_indexed_slices(ret_vec,
                                      thrift_col_parent,
                                      query.getIndexClause(),
                                      thrift_slice_pred,
                                      query.getConsistencyLevel());

    RELEASE_CLIENT

    for (vector<KeySlice>::const_iterator it= ret_vec.begin();
            it != ret_vec.end();
            ++it)
    {
        vector<Column> thrift_cols;
        getColumnList(thrift_cols, it->columns);
        map<string, string> rows;
        for (vector<Column>::const_iterator inner_it= thrift_cols.begin();
                inner_it != thrift_cols.end();
                ++inner_it)
        {
            rows.insert(make_pair(inner_it->name, inner_it->value));
        }
        ret.insert(make_pair(it->key, rows));
    }
}

int32_t Cassandra::getCount(
        const string& key,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    int32_t ret = thrift_client->get_count(key, col_parent, pred, level);
    RELEASE_CLIENT
    return ret;
}

void Cassandra::getMultiCount(
        map<string, int32_t>& ret,
        const vector<string>& key_list,
        const ColumnParent& col_parent,
        const SlicePredicate& pred,
        const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    thrift_client->multiget_count(ret, key_list, col_parent, pred, level);
    RELEASE_CLIENT
}

void Cassandra::reloadKeyspaces()
{
    key_spaces_.clear();

    BORROW_CLIENT
    thrift_client->describe_keyspaces(key_spaces_);
    RELEASE_CLIENT
    if (!current_keyspace_.empty() && (current_ks_num_ = findKeyspace(current_keyspace_)) == (uint32_t) -1)
        current_keyspace_.clear();
}

const vector<KsDef>& Cassandra::getKeyspaces()
{
    return key_spaces_;
}

string Cassandra::createColumnFamily(const CfDef& cf_def)
{
    string schema_id;
    reloadKeyspaces();

    uint32_t ks_num = (cf_def.keyspace == current_keyspace_) ? current_ks_num_ : findKeyspace(cf_def.keyspace);
    uint32_t cf_num = findColumnFamily(ks_num, cf_def.name);
    BORROW_CLIENT
    if (cf_num == (uint32_t) -1)
        thrift_client->system_add_column_family(schema_id, cf_def);
    else
    {
        const_cast<CfDef &>(cf_def).__set_id(key_spaces_[ks_num].cf_defs[cf_num].id);
        thrift_client->system_update_column_family(schema_id, cf_def);
    }
    RELEASE_CLIENT
    return schema_id;
}

string Cassandra::dropColumnFamily(const string& cf_name)
{
    string schema_id;
    BORROW_CLIENT
    thrift_client->system_drop_column_family(schema_id, cf_name);
    RELEASE_CLIENT
    return schema_id;
}

string Cassandra::updateColumnFamily(const CfDef& cf_def)
{
    string schema_id;
    BORROW_CLIENT
    thrift_client->system_update_column_family(schema_id, cf_def);
    RELEASE_CLIENT
    return schema_id;
}

string Cassandra::createKeyspace(const KsDef& ks_def)
{
    string ret;
    reloadKeyspaces();

    uint32_t ks_num = findKeyspace(ks_def.name);
    BORROW_CLIENT
    if (ks_num == (uint32_t) -1)
        thrift_client->system_add_keyspace(ret, ks_def);
    else
        thrift_client->system_update_keyspace(ret, ks_def);
    RELEASE_CLIENT
    return ret;
}

string Cassandra::updateKeyspace(const KsDef& ks_def)
{
    string ret;
    BORROW_CLIENT
    thrift_client->system_update_keyspace(ret, ks_def);
    RELEASE_CLIENT
    return ret;
}

void Cassandra::truncateColumnFamily(const string& cf_name)
{
    BORROW_CLIENT
    thrift_client->truncate(cf_name);
    RELEASE_CLIENT
}

void Cassandra::batchMutate(const map<string, map<string, vector<Mutation> > >& mutation_map, const ConsistencyLevel::type level)
{
    BORROW_CLIENT
    thrift_client->batch_mutate(mutation_map, level);
    RELEASE_CLIENT
}

void Cassandra::executeCqlQuery(CqlResult& result, const string& query, const Compression::type compression)
{
    BORROW_CLIENT
    thrift_client->execute_cql_query(result, query, compression);
    RELEASE_CLIENT
}

string Cassandra::dropKeyspace(const string& ks_name)
{
    string ret;
    BORROW_CLIENT
    thrift_client->system_drop_keyspace(ret, ks_name);
    RELEASE_CLIENT
    if (current_keyspace_ == ks_name)
    {
        current_keyspace_.clear();
        current_ks_num_ = -1;
    }

    return ret;
}

const string& Cassandra::getClusterName()
{
    if (cluster_name_.empty())
    {
        BORROW_CLIENT
        thrift_client->describe_cluster_name(cluster_name_);
        RELEASE_CLIENT
    }
    return cluster_name_;
}

const string& Cassandra::getServerVersion()
{
    if (server_version_.empty())
    {
        BORROW_CLIENT
        thrift_client->describe_version(server_version_);
        RELEASE_CLIENT
    }
    return server_version_;
}

uint32_t Cassandra::findKeyspace(const string& name) const
{
    for (uint32_t i = 0; i < key_spaces_.size(); i++)
    {
        if (key_spaces_[i].name == name)
            return i;
    }
    return -1;
}

uint32_t Cassandra::findColumnFamily(uint32_t ks_num, const std::string& name) const
{
    if (ks_num == (uint32_t) -1 || ks_num >= key_spaces_.size())
        return -1;
    for (uint32_t i = 0; i < key_spaces_[ks_num].cf_defs.size(); i++)
    {
        if (key_spaces_[ks_num].cf_defs[i].name == name)
            return i;
    }
    return -1;
}

}
