/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include <sys/time.h>
#include <endian.h>

#include "libcassandra/util_functions.h"

using namespace std;
using namespace org::apache::cassandra;

namespace libcassandra
{

void createColumnDefObject(
        ColumnDef& col_def,
        const string& in_name,
        const string& in_validation_class,
        const IndexType::type in_index_type,
        const string& in_index_name,
        const map<string, string>& in_index_options)
{
    col_def.__set_name(in_name);
    col_def.__set_validation_class(in_validation_class);
    if (! in_index_name.empty())
    {
        col_def.__set_index_type(in_index_type);
        col_def.__set_index_name(in_index_name);
        col_def.__set_index_options(in_index_options);
    }
}

void createKsDefObject(
        KsDef& ks_def,
        const string& in_name,
        const string& in_strategy_class,
        const map<string, string>& in_strategy_options,
        const int32_t in_replication_factor,
        const vector<CfDef>& in_cf_defs,
        bool in_durable_writes)
{
    ks_def.__set_name(in_name);
    ks_def.__set_strategy_class(in_strategy_class);
    if (! in_strategy_options.empty())
    {
        ks_def.__set_strategy_options(in_strategy_options);
    }
    if (in_replication_factor > 0)
    {
        ks_def.__set_replication_factor(in_replication_factor);
    }
    if (! in_cf_defs.empty())
    {
        ks_def.__set_cf_defs(in_cf_defs);
    }
    ks_def.__set_durable_writes(in_durable_writes);
}

void createCfDefObject(
        CfDef& cf_def,
        const string& in_keyspace,
        const string& in_name,
        const string& in_column_type,
        const string& in_comparator_type,
        const string& in_sub_comparator_type,
        const string& in_comment,
        const double in_row_cache_size,
        const double in_key_cache_size,
        const double in_read_repair_chance,
        const vector<ColumnDef>& in_column_metadata,
        const int32_t in_gc_grace_seconds,
        const string& in_default_validation_class,
        const int32_t in_id,
        const int32_t in_min_compaction_threshold,
        const int32_t in_max_compaction_threshold,
        const int32_t in_row_cache_save_period_in_seconds,
        const int32_t in_key_cache_save_period_in_seconds,
        const int8_t in_replicate_on_write,
        const double in_merge_shards_chance,
        const string& in_key_validation_class,
        const string& in_row_cache_provider,
        const string& in_key_alias,
        const string& in_compaction_strategy,
        const map<string, string>& in_compaction_strategy_options,
        const int32_t in_row_cache_keys_to_save,
        const map<string, string>& in_compression_options,
        const double in_bloom_filter_fp_chance)
{
    /*
     * keyspace name and cf name are required
     * TODO - throw an exception if these are not present
     */
    cf_def.__set_keyspace(in_keyspace);
    cf_def.__set_name(in_name);
    /* everything else associated with a column family is optional */
    if (! in_column_type.empty())
    {
        cf_def.__set_column_type(in_column_type);
    }
    if (! in_comparator_type.empty())
    {
        cf_def.__set_comparator_type(in_comparator_type);
    }
    if (! in_sub_comparator_type.empty())
    {
        cf_def.__set_subcomparator_type(in_sub_comparator_type);
    }
    if (! in_comment.empty())
    {
        cf_def.__set_comment(in_comment);
    }
    if (in_row_cache_size > 0)
    {
        cf_def.__set_row_cache_size(in_row_cache_size);
    }
    if (in_key_cache_size > 0)
    {
        cf_def.__set_key_cache_size(in_key_cache_size);
    }
    if (in_read_repair_chance > 0)
    {
        cf_def.__set_read_repair_chance(in_read_repair_chance);
    }
    if (! in_column_metadata.empty())
    {
        cf_def.__set_column_metadata(in_column_metadata);
    }
    if (in_gc_grace_seconds > 0)
    {
        cf_def.__set_gc_grace_seconds(in_gc_grace_seconds);
    }
    if (! in_default_validation_class.empty())
    {
        cf_def.__set_default_validation_class(in_default_validation_class);
    }
    if (in_id > 0)
    {
        cf_def.__set_id(in_id);
    }
    if (in_min_compaction_threshold > 0)
    {
        cf_def.__set_min_compaction_threshold(in_min_compaction_threshold);
    }
    if (in_max_compaction_threshold > 0)
    {
        cf_def.__set_max_compaction_threshold(in_max_compaction_threshold);
    }
    if (in_row_cache_save_period_in_seconds > 0)
    {
        cf_def.__set_row_cache_save_period_in_seconds(in_row_cache_save_period_in_seconds);
    }
    if (in_key_cache_save_period_in_seconds > 0)
    {
        cf_def.__set_key_cache_save_period_in_seconds(in_key_cache_save_period_in_seconds);
    }
    if (in_replicate_on_write >= 0)
    {
        cf_def.__set_replicate_on_write(in_replicate_on_write);
    }
    if (in_merge_shards_chance > 0)
    {
        cf_def.__set_merge_shards_chance(in_merge_shards_chance);
    }
    if (! in_key_validation_class.empty())
    {
        cf_def.__set_key_validation_class(in_key_validation_class);
    }
    if (! in_row_cache_provider.empty())
    {
        cf_def.__set_row_cache_provider(in_row_cache_provider);
    }
    if (! in_key_alias.empty())
    {
        cf_def.__set_key_alias(in_key_alias);
    }
    if (! in_compaction_strategy.empty())
    {
        cf_def.__set_compaction_strategy(in_compaction_strategy);
    }
    if (! in_compaction_strategy_options.empty())
    {
        cf_def.__set_compaction_strategy_options(in_compaction_strategy_options);
    }
    if (in_row_cache_keys_to_save > 0)
    {
        cf_def.__set_row_cache_keys_to_save(in_row_cache_keys_to_save);
    }
    if (! in_compression_options.empty())
    {
        cf_def.__set_compression_options(in_compression_options);
    }
    if (in_bloom_filter_fp_chance > 0)
    {
        cf_def.__set_bloom_filter_fp_chance(in_bloom_filter_fp_chance);
    }
}

void createSlicePredicateObject(SlicePredicate& slice_pred, const IndexedSlicesQuery& query)
{
    if (query.isColumnsSet())
    {
        slice_pred.__isset.column_names = true;
        slice_pred.__set_column_names(query.getColumns());
    }
    if (query.isRangeSet())
    {
        slice_pred.__isset.slice_range = true;
        slice_pred.slice_range.__set_start(query.getStartColumn());
        slice_pred.slice_range.__set_finish(query.getEndColumn());
        slice_pred.slice_range.__set_reversed(query.getReverseColumns());
        slice_pred.slice_range.__set_count(query.getRowCount());
    }
}

void getColumnList(vector<Column>& cols, const vector<ColumnOrSuperColumn>& raw_cols)
{
    vector<Column> ret(raw_cols.size());
    for (vector<ColumnOrSuperColumn>::const_iterator it = raw_cols.begin();
            it != raw_cols.end();
            ++it)
    {
        ret.push_back(it->column);
    }
    cols.swap(ret);
}

void getSuperColumnList(vector<SuperColumn>& super_cols, const vector<ColumnOrSuperColumn>& raw_cols)
{
    vector<SuperColumn> ret(raw_cols.size());
    for (vector<ColumnOrSuperColumn>::const_iterator it = raw_cols.begin();
            it != raw_cols.end();
            ++it)
    {
        ret.push_back(it->super_column);
    }
    super_cols.swap(ret);
}

int64_t createTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
}

string serializeLong(int64_t t)
{
    int64_t tmp = htobe64(t);
    return string(reinterpret_cast<const char *>(&tmp), 8);
}

int64_t deserializeLong(const string& t)
{
    return be64toh(*reinterpret_cast<const int64_t*>(t.c_str()));
}

} /* end namespace libcassandra */
