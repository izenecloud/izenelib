// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_CUCKOO_HASH_TABLE_H
#define YIELD_PLATFORM_CUCKOO_HASH_TABLE_H

#include "yield/platform/platform_types.h"

#include <cstring>

#define CHT_MAX_LG_TABLE_SIZE_IN_BINS 20


namespace YIELD
{
	template <typename KeyType, class ValueType>
	class CuckooHashTable
	{
	public:
		CuckooHashTable( uint8_t lg_table_size_in_bins = 6, uint8_t records_per_bin = 8, uint8_t table_count = 2 )
			: lg_table_size_in_bins( lg_table_size_in_bins ), records_per_bin( records_per_bin ), table_count( table_count )
		{
			per_table_records_filled = new KeyType[table_count];
			table_size_in_bins = table_size_in_records = 0;
			resizeTables( lg_table_size_in_bins );
			clear();
		}

		~CuckooHashTable()
		{
			delete [] per_table_records_filled;
			delete [] tables;
		}

		ValueType find( KeyType external_key )
		{
			KeyType internal_key = external_key;
			Record* record;

			for ( uint8_t table_i = 0; table_i < table_count; table_i++ )
			{
				record = getRecord( table_i, internal_key, external_key );
				if ( record )
					return record->value;
				else
					internal_key = rehashKey( internal_key );
			}

			return 0;
		}

		void insert( KeyType external_key, ValueType value )
		{
			while ( lg_table_size_in_bins < CHT_MAX_LG_TABLE_SIZE_IN_BINS )
			{
				if ( insertWithoutResize( external_key, value ) )
					return;
				else
					resizeTables( lg_table_size_in_bins + 1 ); // Will set lg_table_size_in_bins
			}

//			DebugBreak();
		}

		ValueType erase( KeyType external_key )
		{
			KeyType internal_key = external_key;
			Record* record;

			for ( uint8_t table_i = 0; table_i < table_count; table_i++ )
			{
				record = getRecord( table_i, internal_key, external_key );
				if ( record )
				{
					ValueType old_value = record->value;
					std::memset( record, 0, sizeof( Record ) );
					total_records_filled--;
					per_table_records_filled[table_i]--;
					return old_value;
				}
				else
					internal_key = rehashKey( internal_key );
			}

			return 0;
		}

		void clear()
		{
			std::memset( tables, 0, sizeof( Record ) * table_size_in_records * table_count );
			for ( uint8_t table_i = 0; table_i < table_count; table_i++ ) per_table_records_filled[table_i] = 0;
			total_records_filled = 0;
		}

		size_t size()
		{
			return total_records_filled;
		}

		bool empty()
		{
			return size() == 0;
		}

		class iterator
		{
		public:
			iterator() : cht( NULL ), record_i( 0 ) { }
			iterator( CuckooHashTable<KeyType, ValueType>* cht, size_t record_i ) : cht( cht ), record_i( record_i ) { }
			iterator( const iterator& other ) : cht( other.cht ), record_i( other.record_i ) { }

			iterator& operator++()
			{
				return ++( *this );
			}

			iterator& operator++( int )
			{
				record_i++;
				while ( record_i < ( cht->table_count * cht->table_size_in_records ) &&
					    cht->tables[record_i].external_key == 0 )
					record_i++;
				return *this;
		    }

			ValueType& operator*()
			{
				return cht->tables[record_i].value;
			}

			bool operator!=( const iterator& other )
			{
				return record_i != other.record_i;
			}

		private:
			CuckooHashTable<KeyType, ValueType>* cht;
			size_t record_i;
		};

		iterator begin()
		{
			return iterator( this, 0 );
		}

		iterator end()
		{
			return iterator( this, table_count * table_size_in_records );
		}

	private:
		uint8_t lg_table_size_in_bins, records_per_bin, table_count;
		unsigned table_size_in_records, table_size_in_bins;

		struct Record
		{
			KeyType external_key;
			ValueType value;
		};

		Record* tables;
		KeyType total_records_filled, *per_table_records_filled;


		Record* getRecord( uint8_t table_i, KeyType internal_key, KeyType external_key )
		{
			Record* table = tables + ( table_i * table_size_in_records );
			KeyType bin_i = internal_key & ( table_size_in_bins -1 ), bin_i_end = bin_i + records_per_bin;
			for ( ; bin_i < bin_i_end; bin_i++ )
			{
				if ( table[bin_i].external_key == external_key )
				{
					//if ( bin_i_end - bin_i < records_per_bin ) DebugBreak();
					return &table[bin_i];
				}
			}
			return 0;
		}

		KeyType rehashKey( KeyType key )
		{
			return key ^ ( key >> lg_table_size_in_bins );
		}

		bool insertWithoutResize( KeyType external_key, ValueType value )
		{
			if ( find( external_key ) == value )
				return true;

			KeyType internal_key = external_key;
			Record* record;

			for ( uint8_t table_i = 0; table_i < table_count; table_i++ )
			{
				record = getRecord( table_i, internal_key, 0 ); // Get an empty record

				if ( record )
				{
					record->external_key = external_key;
					record->value = value;
					total_records_filled++;
					per_table_records_filled[table_i]++;
					return true;
				}
				else
					internal_key = rehashKey( internal_key );
			}

			return false;
		}

		void resizeTables( uint8_t new_lg_table_size_in_bins )
		{
			Record* old_tables = tables;
			uint8_t old_lg_table_size_in_bins = lg_table_size_in_bins;
	//		KeyType old_table_size_in_bins = table_size_in_bins;
			KeyType old_table_size_in_records = table_size_in_records;

			while ( new_lg_table_size_in_bins < CHT_MAX_LG_TABLE_SIZE_IN_BINS )
			{
				lg_table_size_in_bins = new_lg_table_size_in_bins;
				table_size_in_bins = 1 << lg_table_size_in_bins;
				table_size_in_records = table_size_in_bins * records_per_bin;
				tables = new Record[table_size_in_records * table_count];
				this->clear();
				total_records_filled = 0;

				if ( new_lg_table_size_in_bins == old_lg_table_size_in_bins ) // We're being called from the constructor
					return;
				else // There are old records
				{
					for ( uint8_t old_table_i = 0; old_table_i < table_count; old_table_i++ )
					{
						Record* old_table = old_tables + ( old_table_i * old_table_size_in_records );
						for ( KeyType old_record_i = 0; old_record_i < old_table_size_in_records; old_record_i++ )
						{
							Record* old_record = &old_table[old_record_i];
							if ( old_record->external_key != 0 )
							{
								if ( insertWithoutResize( old_record->external_key, old_record->value ) )
									continue;
								else
								{
									new_lg_table_size_in_bins++;
									break; // Out of the old_record_i for loop
								}
							}
						}

						if ( new_lg_table_size_in_bins != lg_table_size_in_bins ) // We were unable to insert an old record without a resize
							break; // Out of the old_table_i for loop
					}

					if ( new_lg_table_size_in_bins == lg_table_size_in_bins ) // We successfully resized the table
					{
						delete [] old_tables;
						return;
					}
					else // We could not insert all of the old records in the resized table, try again
						delete [] tables;
				}
			}

//			DebugBreak(); // We could not insert all of the old records without going past the max lg_table_size.
						  // Something is definitely wrong.
		}
	};
};

#endif
