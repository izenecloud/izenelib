// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include "log/log_stats.h"

using namespace babudb;

#define SAMPLE_EVERY_MS 5000

Stats::Stats( const string& name, GlobalRegistries* global_registries ) : name( name ), global_registries( global_registries )
{
//	next_sample_timestamp = last_sample_timestamp = first_timestamp = TimeMeasurement::getCurrentTimestamp();
}

Stats::~Stats()
{
}


/*
void Stats::sampleTimedStatistics()
{
	uint64 current_timestamp = TimeMeasurement::getCurrentTimestamp();
	if ( current_timestamp > next_sample_timestamp )
	{
		double ms_elapsed = TimeMeasurement::ticksToMilliseconds( current_timestamp - last_sample_timestamp );

		calculateDerivatives( ms_elapsed );

		last_sample_timestamp = current_timestamp;
		next_sample_timestamp = current_timestamp + TimeMeasurement::millisecondsToTicks( SAMPLE_EVERY_MS );

		Stage* stats_target = global_registries->getEventTarget( getQualifiedEventName() );
		// Have to look up stats_target each time because it may be a Python request handler stage, which can change
		if( !stats_target )
			stats_target = global_registries->getEventTarget( "StatsEvent" );

		if ( stats_target )
			stats_target->sendAsync( (Stats*)clone() );
	}
}
*/

void Stats::triggerStats()
{
//	next_sample_timestamp = 0;
}

void LogStats::calculateDerivatives( double ms_elapsed )
{
	bytes_appended_rate = ( double )( bytes_appended_in_period * 1000.0 / ms_elapsed );
	append_rate = ( double )( appends_in_period * 1000.0 / ms_elapsed );

	bytes_appended_in_period = 0;
	appends_in_period = 0;
}

/*
void LogStats::serialize( OutputBinding& binding )
{
	Stats::serialize( binding );
	binding.writeDouble( "LogFileSize", log_file_length );
	binding.writeDouble( "LogSize", log_length );
	binding.writeDouble( "GapsSize", gaps_length );
	binding.writeDouble( "NoOfRecords", no_of_records );
	binding.writeDouble( "NoOfDeletors", no_of_deletors );
	binding.writeDouble( "NoOfGaps", no_of_gaps );
	binding.writeDouble( "BytesAppendRate", bytes_appended_rate );
	binding.writeDouble( "AppendRate", append_rate );
}
*/

void LogDatabaseStats::calculateDerivatives( double ms_elapsed )
{
	LogStats::calculateDerivatives( ms_elapsed );

	update_rate = ( double )( no_of_updates_in_period * 1000.0 / ms_elapsed );

	no_of_updates_in_period = 0;
}

/*
void LogDatabaseStats::serialize( OutputBinding& binding )
{
	LogStats::serialize( binding );
	binding.writeDouble( "NoOfEvents", no_of_events );
	binding.writeDouble( "EventsSize", cum_event_size );
	binding.writeDouble( "NoOfOrphanedParameters", no_of_orphaned_params );
	binding.writeDouble( "OrphanedParametersSize", cum_orphaned_params_size );
	binding.writeDouble( "NoOfOperations", no_of_ops );
	binding.writeDouble( "OperationsSize", cum_ops_size );
	binding.writeDouble( "NoOfBlocks", no_of_blocks );
	binding.writeDouble( "NoOfUpdates", no_of_updates );
	binding.writeDouble( "NoOfUpdatesInPeriod", no_of_updates_in_period );
	binding.writeDouble( "UpdateRate", update_rate );
	binding.writeDouble( "CleanupOffset", cleanup_offset );
	binding.writeDouble( "CleanupRecordsMoved", cleanup_records_moved );
}
*/
