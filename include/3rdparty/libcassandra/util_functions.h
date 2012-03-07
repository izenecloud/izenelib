/*
 * LibCassandra
 * Copyright (C) 2010 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#ifndef __LIBCASSANDRA_UTIL_FUNCTIONS_H
#define __LIBCASSANDRA_UTIL_FUNCTIONS_H

#include "genthrift/cassandra_types.h"

#include "indexed_slices_query.h"


namespace libcassandra
{


/**
 * @param[in] url to parse
 * @return the port number from the given url
 */
int parsePortFromURL(const std::string &url);


/**
 * @param[in] url to parse
 * @return the host namefrom the given url
 */
std::string parseHostFromURL(const std::string &url);

/**
 * Convert a ColumnDefinition object to the thrift
 * equivalent - ColumnDef
 * @param[in] col_def a ColumnDefinition object
 */
void createColumnDefObject(
        org::apache::cassandra::ColumnDef& col_def,
        const std::string& in_name,
        const std::string& in_validation_class,
        const org::apache::cassandra::IndexType::type in_index_type,
        const std::string& in_index_name,
        const std::map<std::string, std::string>& in_index_options);

/**
 * Convert a KeyspaceDefinition object to the thrift
 * equivalent - KsDef
 * @param[in] ks_def a KeyspaceDefinition object
 */
void createKsDefObject(
        org::apache::cassandra::KsDef& ks_def,
        const std::string& in_name,
        const std::string& in_strategy_class,
        const std::map<std::string, std::string>& in_strategy_options,
        const int32_t in_replication_factor,
        const std::vector<org::apache::cassandra::CfDef>& in_cf_defs,
        bool in_durable_writes = true);

/**
 * Convert a ColumnFamilyDefinition object to the thrift
 * equivalent - CfDef
 * @param[in] cf_def a ColumnFamilyDefinition object
 */
void createCfDefObject(
        org::apache::cassandra::CfDef& cf_def,
        const std::string& in_keyspace,
        const std::string& in_name,
        const std::string& in_column_type,
        const std::string& in_comparator_type,
        const std::string& in_sub_comparator_type,
        const std::string& in_comment,
        const double in_row_cache_size,
        const double in_key_cache_size,
        const double in_read_repair_chance,
        const std::vector<org::apache::cassandra::ColumnDef>& in_column_metadata,
        const int32_t in_gc_grace_seconds,
        const std::string& in_default_validation_class,
        const int32_t in_id,
        const int32_t in_min_compaction_threshold,
        const int32_t in_max_compaction_threshold,
        const int32_t in_row_cache_save_period_in_seconds,
        const int32_t in_key_cache_save_period_in_seconds,
        const int8_t in_replicate_on_write,
        const double in_merge_shards_chance,
        const std::string& in_key_validation_class,
        const std::string& in_row_cache_provider,
        const std::string& in_key_alias,
        const std::string& in_compaction_strategy,
        const std::map<std::string, std::string>& in_compaction_strategy_options,
        const int32_t in_row_cache_keys_to_save,
        const std::map<std::string, std::string>& in_compression_options,
        const double in_bloom_filter_fp_chance);

/**
 * Convert a IndexedSlicesQuery object to the thrift
 * equivalent - SlicePredicate
 * @param[in] slice_pred result thrift SlicePredicate object equivalent to the query input
 * @param[in] query an IndexedSlicesQuery object
 */
void createSlicePredicateObject(org::apache::cassandra::SlicePredicate& slice_pred, const IndexedSlicesQuery& query);

/**
 * Extract the columns from the vector of columns or super columns
 * @param[in] cols result vector of Column objects
 * @param[in] raw_cols vector to process
 */
void getColumnList(std::vector<org::apache::cassandra::Column>& cols, const std::vector<org::apache::cassandra::ColumnOrSuperColumn>& raw_cols);

/**
 * Extract the super columns from the vector of columns or super columns
 * @param[in] super_cols result vector of SuperColumn objects
 * @param[in] raw_cols vector to process
 */
void getSuperColumnList(std::vector<org::apache::cassandra::SuperColumn>& super_cols, const std::vector<org::apache::cassandra::ColumnOrSuperColumn>& raw_cols);

/**
 * @return a timestamp in micro-seconds
 */
int64_t createTimestamp();

/**
 * Convert given 64 bit integer to big-endian
 * format and place these raw bytes in a std::string
 * This is the format thrift expects for a LongType
 * @param[in] t integer to work with
 * @return a std::string representing the input in big-endian format
 */
std::string serializeLong(int64_t t);

/**
 * Convert given string in big-endian format
 * to a 64 bit integer
 * @param[in] t string to work with
 * @return a int64_t
 */
int64_t deserializeLong(const std::string& t);

} /* end namespace libcassandra */

#endif /* __LIBCASSANDRA_UTIL_FUNCTIONS_H */
