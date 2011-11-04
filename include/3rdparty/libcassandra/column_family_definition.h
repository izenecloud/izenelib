/*
 * LibCassandra
 * Copyright (C) 2010-2011 Padraig O'Sullivan
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license. See
 * the COPYING file in the parent directory for full text.
 */

#ifndef __LIBCASSANDRA_COLUMN_FAMILY_DEFINITION_H
#define __LIBCASSANDRA_COLUMN_FAMILY_DEFINITION_H

#include <string>
#include <vector>

#include "genthrift/cassandra_types.h"

#include "column_definition.h"

namespace libcassandra
{

class Cassandra;

class ColumnFamilyDefinition
{

public:

    ColumnFamilyDefinition();
    ColumnFamilyDefinition(
        const std::string& in_keyspace_name,
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
        const std::map<std::string, std::string>& in_compression_options);
    ~ColumnFamilyDefinition() {}

    /**
     * @return column family name
     */
    std::string getName() const;

    void setName(const std::string& cf_name);

    /**
     * @return keyspace name
     */
    std::string getKeyspaceName() const;

    void setKeyspaceName(const std::string& ks_name);

    /**
     * @return column type
     */
    std::string getColumnType() const;

    void setColumnType(const std::string& col_type);

    /**
     * @return true if column type is set; false otherwise
     */
    bool isColumnTypeSet() const;

    /**
     * @return comparator type
     */
    std::string getComparatorType() const;

    void setComparatorType(const std::string& comp_type);

    /**
     * @return true if comparator type is set; false otherwise
     */
    bool isComparatorTypeSet() const;

    /**
     * @return sub-comparator type
     */
    std::string getSubComparatorType() const;

    void setSubComparatorType(const std::string& sub_comp_type);

    /**
     * @return true if sub comparator type is set; false otherwise
     */
    bool isSubComparatorTypeSet() const;

    /**
     * @return comment
     */
    std::string getComment() const;

    void setComment(const std::string& comm);

    /**
     * @return true if comment is set; false otherwise
     */
    bool isCommentSet() const;

    /**
     * @return row cache size
     */
    double getRowCacheSize() const;

    void setRowCacheSize(double size);

    /**
     * @return true if row cache size is > 0; false otherwise
     */
    bool isRowCacheSizeSet() const;

    /**
     * @return row cache save period in seconds
     */
    int32_t getRowCacheSavePeriod() const;

    void setRowCacheSavePeriod(int32_t save_period);

    /**
     * @return true if row cache save period is > 0; false otherwise
     */
    bool isRowCacheSavePeriodSet() const;

    /**
     * @return key cache size
     */
    double getKeyCacheSize() const;

    void setKeyCacheSize(double size);

    /**
     * @return true if key cache size is > 0; false otherwise
     */
    bool isKeyCacheSizeSet() const;

    /**
     * @return read repair chance
     */
    double getReadRepairChance() const;

    void setReadRepairChance(double chance);

    /**
     * @return true if read repair chance > 0; false otherwise
     */
    bool isReadRepairChanceSet() const;

    /**
     * @return garbage collection grace seconds
     */
    int32_t getGcGraceSeconds() const;

    void setGcGraceSeconds(int32_t gc_secs);

    /**
     * @return true if gc grace seconds > 0; false otherwise
     */
    bool isGcGraceSecondsSet() const;

    /**
     * @return default validation class
     */
    std::string getDefaultValidationClass() const;

    void setDefaultValidationClass(const std::string& class_name);

    /**
     * @return true if default validation class is set; false otherwise
     */
    bool isDefaultValidationClassSet() const;

    /**
     * @return column family ID
     */
    int32_t getId() const;

    void setId(int32_t new_id);

    /**
     * @return true if id > 0; false otherwise
     */
    bool isIdSet() const;

    /**
     * @return max compaction threshold
     */
    int32_t getMaxCompactionThreshold() const;

    void setMaxCompactionThreshold(int32_t threshold);

    /**
     * @return true if max compaction threshold > 0; false otherwise
     */
    bool isMaxCompactionThresholdSet() const;

    /**
     * @return min compaction threshold
     */
    int32_t getMinCompactionThreshold() const;

    void setMinCompactionThreshold(int32_t threshold);

    /**
     * @return true if min compaction threshold > 0; false otherwise
     */
    bool isMinCompactionThresholdSet() const;

    const std::vector<ColumnDefinition>& getColumnMetadata() const;

    void setColumnMetadata(std::vector<ColumnDefinition>& meta);

    void addColumnMetadata(const ColumnDefinition& col_meta);

    bool isColumnMetadataSet() const;

    void setCompressOptions(const std::map<std::string, std::string> & val);

    const std::map<std::string, std::string>& getCompressOptions() const;

    bool isCompressOptionsSet() const;

private:

    std::string keyspace_name;

    std::string name;

    std::string column_type;

    std::string comparator_type;

    std::string sub_comparator_type;

    std::string comment;

    double row_cache_size;

    double key_cache_size;

    double read_repair_chance;

    int32_t gc_grace_seconds;

    std::string default_validation_class;

    int32_t id;

    int32_t min_compaction_threshold;

    int32_t max_compaction_threshold;

    int32_t row_cache_save_period_in_seconds;

    int32_t key_cache_save_period_in_seconds;

    std::vector<ColumnDefinition> column_metadata;

    ////////////////////////////////////////////////////////////////////////////
    //
    // compression_options:
    //
    //   This is a container property for setting compression options on a
    //   column family. The compression_options property contains the
    //   following options:
    //
    //   * sstable_compression:
    //
    //     Which specifies the compression algorithm to use when compressing
    //     SSTable files. Cassandra supports two built-in compression classes:
    //
    //     - SnappyCompressor (Snappy compression library) and
    //     - DeflateCompressor (Java Zip implementation).
    //
    //     Snappy offers faster compression/decompression while Zip offers
    //     better compression ratio. Which one to choose depends on whether you
    //     favour space saving over speed or the contrary. For example, if you
    //     intend for I/O intensive tasks, then SnappyCompressor should be the
    //     choice. Developers can also implement custom compression classes
    //     around org.apache.cassandra.io.compress.ICompressor interface.
    //
    //   * chunk_length_kb:
    //
    //     Which sets the compression chunk size in kilobytes. The default
    //     (as 64) is a good middle ground for compressing column families
    //     with either wide or skinny rows. For wide rows, it allows reading a
    //     64kb slice of column data without decompressing the entire row.
    //     For skinny rows, it is a good trade-off between maximizing the
    //     compression ratio and minimizing the overhead of decompressing more
    //     than needed data to access a requested row. The compression chunk
    //     size can be adjusted to account for data access patterns (how much
    //     data is typically requested at a time) and the average size of rows
    //     across the column family.
    //
    ////////////////////////////////////////////////////////////////////////////

    std::map<std::string, std::string> compression_options;

};

} /* end namespace libcassandra */

#endif /* __LIBCASSANDRA_COLUMN_FAMILY_DEFINITION_H */
