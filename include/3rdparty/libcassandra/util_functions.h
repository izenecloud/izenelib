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

#include <string>

#include "genthrift/cassandra_types.h"

#include "column_family_definition.h"
#include "indexed_slices_query.h"
#include "keyspace_definition.h"


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
 * @param[in] thrift_col_def result thrift ColumnDef object equivalent to the col_def input
 * @param[in] col_def a ColumnDefinition object
 */
void createColumnDefObject(org::apache::cassandra::ColumnDef& thrift_col_def, const ColumnDefinition& col_def);

/**
 * Convert a KeyspaceDefinition object to the thrift
 * equivalent - KsDef
 * @param[in] thrift_ks_def result thrift KsDef object equivalent to the ks_def input
 * @param[in] ks_def a KeyspaceDefinition object
 */
void createKsDefObject(org::apache::cassandra::KsDef& thrift_ks_def, const KeyspaceDefinition& ks_def);

/**
 * Convert a ColumnFamilyDefinition object to the thrift
 * equivalent - CfDef
 * @param[in] thrift_cf_def result thrift CfDef object equivalent to the cf_def input
 * @param[in] cf_def a ColumnFamilyDefinition object
 */
void createCfDefObject(org::apache::cassandra::CfDef& thrift_cf_def, const ColumnFamilyDefinition& cf_def);

/**
 * Convert a IndexedSlicesQuery object to the thrift
 * equivalent - SlicePredicate
 * @param[in] thrift_slice_pred result thrift SlicePredicate object equivalent to the query input
 * @param[in] query an IndexedSlicesQuery object
 */
void createSlicePredicateObject(org::apache::cassandra::SlicePredicate& thrift_slice_pred, const IndexedSlicesQuery& query);

/**
 * Extract the columns from the vector of columns or super columns
 * @param[in] thrift_cols result vector of Column objects
 * @param[in] cols vector to process
 */
void getColumnList(std::vector<org::apache::cassandra::Column>& thrift_cols, const std::vector<org::apache::cassandra::ColumnOrSuperColumn>& cols);

/**
 * Extract the super columns from the vector of columns or super columns
 * @param[in] thrift_cols result vector of SuperColumn objects
 * @param[in] cols vector to process
 */
void getSuperColumnList(std::vector<org::apache::cassandra::SuperColumn>& thrift_cols, const std::vector<org::apache::cassandra::ColumnOrSuperColumn>& cols);

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
