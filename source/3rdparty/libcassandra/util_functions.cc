/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#include <sys/time.h>

#include "libcassandra/util_functions.h"

using namespace std;
using namespace org::apache::cassandra;

namespace libcassandra
{

void createColumnDefObject(ColumnDef& thrift_col_def, const ColumnDefinition& col_def)
{
    thrift_col_def.__set_name(col_def.getName());
    thrift_col_def.__set_validation_class(col_def.getValidationClass());
    if (col_def.isIndexTypeSet())
    {
        thrift_col_def.__set_index_type(col_def.getIndexType());
    }
    if (col_def.isIndexNameSet())
    {
        thrift_col_def.__set_index_name(col_def.getIndexName());
    }
    if (col_def.isIndexOptionsSet())
    {
        thrift_col_def.__set_index_options(col_def.getIndexOptions());
    }
}


void createKsDefObject(KsDef& thrift_ks_def, const KeyspaceDefinition& ks_def)
{
    thrift_ks_def.__set_name(ks_def.getName());
    thrift_ks_def.__set_strategy_class(ks_def.getStrategyClass());
    vector<ColumnFamilyDefinition> cf_defs= ks_def.getColumnFamilies();
    for (vector<ColumnFamilyDefinition>::iterator it= cf_defs.begin();
            it != cf_defs.end();
            ++it)
    {
        thrift_ks_def.cf_defs.push_back(CfDef());
        createCfDefObject(thrift_ks_def.cf_defs.back(), *it);
    }
    thrift_ks_def.__set_replication_factor(ks_def.getReplicationFactor());
    thrift_ks_def.__set_durable_writes(ks_def.getDurableWrites());
}


void createCfDefObject(CfDef& thrift_cf_def, const ColumnFamilyDefinition& cf_def)
{
    /*
     * keyspace name and cf name are required
     * TODO - throw an exception if these are not present
     */
    thrift_cf_def.__set_keyspace(cf_def.getKeyspaceName());
    thrift_cf_def.__set_name(cf_def.getName());
    /* everything else associated with a column family is optional */
    if (cf_def.isColumnTypeSet())
    {
        thrift_cf_def.__set_column_type(cf_def.getColumnType());
    }
    if (cf_def.isComparatorTypeSet())
    {
        thrift_cf_def.__set_comparator_type(cf_def.getComparatorType());
    }
    if (cf_def.isSubComparatorTypeSet())
    {
        thrift_cf_def.__set_subcomparator_type(cf_def.getSubComparatorType());
    }
    if (cf_def.isCommentSet())
    {
        thrift_cf_def.__set_comment(cf_def.getComment());
    }
    if (cf_def.isRowCacheSizeSet())
    {
        thrift_cf_def.__set_row_cache_size(cf_def.getRowCacheSize());
    }
    if (cf_def.isKeyCacheSizeSet())
    {
        thrift_cf_def.__set_key_cache_size(cf_def.getKeyCacheSize());
    }
    if (cf_def.isReadRepairChanceSet())
    {
        thrift_cf_def.__set_read_repair_chance(cf_def.getReadRepairChance());
    }
    if (cf_def.isGcGraceSecondsSet())
    {
        thrift_cf_def.__set_gc_grace_seconds(cf_def.getGcGraceSeconds());
    }
    if (cf_def.isDefaultValidationClassSet())
    {
        thrift_cf_def.__set_default_validation_class(cf_def.getDefaultValidationClass());
    }
    if (cf_def.isIdSet())
    {
        thrift_cf_def.__set_id(cf_def.getId());
    }
    if (cf_def.isMaxCompactionThresholdSet())
    {
        thrift_cf_def.__set_max_compaction_threshold(cf_def.getMaxCompactionThreshold());
    }
    if (cf_def.isMinCompactionThresholdSet())
    {
        thrift_cf_def.__set_min_compaction_threshold(cf_def.getMinCompactionThreshold());
    }
    if (cf_def.isColumnMetadataSet())
    {
        vector<ColumnDefinition> cols= cf_def.getColumnMetadata();
        std::vector<ColumnDef>	 column_metadata;
        for (vector<ColumnDefinition>::iterator it= cols.begin(); it != cols.end(); ++it)
        {
            column_metadata.push_back(ColumnDef());
            createColumnDefObject(column_metadata.back(), *it);
        }

        thrift_cf_def.__set_column_metadata(column_metadata);
    }
    if (cf_def.isCompressOptionsSet())
    {
        thrift_cf_def.__set_compression_options(cf_def.getCompressOptions());
    }
}


void createSlicePredicateObject(SlicePredicate& thrift_slice_pred, const IndexedSlicesQuery& query)
{
    if (query.isColumnsSet())
    {
        thrift_slice_pred.__isset.column_names= true;
        vector<string> cols= query.getColumns();
        for (vector<string>::iterator it= cols.begin();
                it != cols.end();
                ++it)
        {
            thrift_slice_pred.column_names.push_back(*it);
        }
    }
    if (query.isRangeSet())
    {
        SliceRange thrift_slice_range;
        thrift_slice_range.start.assign(query.getStartColumn());
        thrift_slice_range.finish.assign(query.getEndColumn());
        thrift_slice_range.reversed= query.getReverseColumns();
        thrift_slice_range.count= query.getRowCount();
        thrift_slice_pred.__isset.slice_range= true;
        thrift_slice_pred.slice_range= thrift_slice_range;
    }
}


void getColumnList(vector<Column>& thrift_cols, const vector<ColumnOrSuperColumn>& cols)
{
    vector<Column> ret(cols.size());
    for (vector<ColumnOrSuperColumn>::const_iterator it= cols.begin();
            it != cols.end();
            ++it)
    {
        ret.push_back(it->column);
    }
    thrift_cols.swap(ret);
}


void getSuperColumnList(vector<SuperColumn>& thrift_cols, const vector<ColumnOrSuperColumn>& cols)
{
    vector<SuperColumn> ret(cols.size());
    for (vector<ColumnOrSuperColumn>::const_iterator it= cols.begin();
            it != cols.end();
            ++it)
    {
        ret.push_back(it->super_column);
    }
    thrift_cols.swap(ret);
}


int64_t createTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
}


string serializeLong(int64_t t)
{
    unsigned char raw_array[8];
    raw_array[0]= (t >> 56) & 0xff;
    raw_array[1]= (t >> 48) & 0xff;
    raw_array[2]= (t >> 40) & 0xff;
    raw_array[3]= (t >> 32) & 0xff;
    raw_array[4]= (t >> 24) & 0xff;
    raw_array[5]= (t >> 16) & 0xff;
    raw_array[6]= (t >> 8) & 0xff;
    raw_array[7]= t & 0xff;
    return string(reinterpret_cast<const char *>(raw_array), 8);
}


int64_t deserializeLong(string& t)
{
    int64_t ret= 0;
    int64_t tmp= 0;
    unsigned char *raw_array= reinterpret_cast<unsigned char *>(const_cast<char *>(t.c_str()));
    ret|= raw_array[7];
    tmp= raw_array[6];
    ret|= (tmp << 8);
    tmp= raw_array[5];
    ret|= (tmp << 16);
    tmp= raw_array[4];
    ret|= (tmp << 24);
    tmp= raw_array[3];
    ret|= (tmp << 32);
    tmp= raw_array[2];
    ret|= (tmp << 40);
    tmp= raw_array[1];
    ret|= (tmp << 48);
    tmp= raw_array[0];
    ret|= (tmp << 56);
    return ret;
}

} /* end namespace libcassandra */
