/*
 * LibCassandra
 * Copyright (C) 2010-2011 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include "libcassandra/keyspace_definition.h"

using namespace libcassandra;
using namespace std;
using namespace org::apache::cassandra;


KeyspaceDefinition::KeyspaceDefinition()
    :name()
    ,strategy_class("org.apache.cassandra.locator.SimpleStrategy")
    ,strategy_options()
    ,replication_factor(1)
    ,col_family_defs()
    ,durable_writes(true)
{}


KeyspaceDefinition::KeyspaceDefinition(
    const string& in_name,
    const string& in_strategy_class,
    const map<string, string>& in_strategy_options,
    const int32_t in_replication_factor,
    const vector<CfDef>& in_cf_defs,
    bool in_durable_writes)
    :name(in_name)
    ,strategy_class(in_strategy_class)
    ,strategy_options(in_strategy_options)
    ,replication_factor(in_replication_factor)
    ,durable_writes(in_durable_writes)
{
    for (vector<CfDef>::const_iterator it= in_cf_defs.begin();
            it != in_cf_defs.end();
            ++it)
    {
        const CfDef& thrift_entry= *it;
        ColumnFamilyDefinition entry(thrift_entry.keyspace,
                                     thrift_entry.name,
                                     thrift_entry.column_type,
                                     thrift_entry.comparator_type,
                                     thrift_entry.subcomparator_type,
                                     thrift_entry.comment,
                                     thrift_entry.row_cache_size,
                                     thrift_entry.key_cache_size,
                                     thrift_entry.read_repair_chance,
                                     thrift_entry.column_metadata,
                                     thrift_entry.gc_grace_seconds,
                                     thrift_entry.default_validation_class,
                                     thrift_entry.id,
                                     thrift_entry.min_compaction_threshold,
                                     thrift_entry.max_compaction_threshold,
                                     thrift_entry.row_cache_save_period_in_seconds,
                                     thrift_entry.key_cache_save_period_in_seconds,
                                     thrift_entry.compression_options);
        col_family_defs[thrift_entry.name] = entry;
    }
}


const string& KeyspaceDefinition::getName() const
{
    return name;
}


void KeyspaceDefinition::setName(const string& ks_name)
{
    name.assign(ks_name);
}


const string& KeyspaceDefinition::getStrategyClass() const
{
    return strategy_class;
}


void KeyspaceDefinition::setStrategyClass(const string& strat_class)
{
    strategy_class.assign(strat_class);
}


const map<string, string>& KeyspaceDefinition::getStrategyOptions() const
{
    return strategy_options;
}


void KeyspaceDefinition::setStrategyOptions(const map<string, string>& opts)
{
    strategy_options = opts;
}


int32_t KeyspaceDefinition::getReplicationFactor() const
{
    return replication_factor;
}


void KeyspaceDefinition::setReplicationFactor(int32_t rep_factor)
{
    replication_factor = rep_factor;
}


const map<string, ColumnFamilyDefinition>& KeyspaceDefinition::getColumnFamilies() const
{
    return col_family_defs;
}


void KeyspaceDefinition::setColumnFamilies(const map<string, ColumnFamilyDefinition>& cfs)
{
    col_family_defs = cfs;
}

bool KeyspaceDefinition::getDurableWrites() const
{
    return durable_writes;
}

void KeyspaceDefinition::setDurableWrites(bool val)
{
    durable_writes = val;
}
