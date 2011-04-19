// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

// LogStats keeps statistics about the Log and its usage

#ifndef LOG__LOG_STATS_H
#define LOG__LOG_STATS_H

#include "yield/platform/debug.h"
#include "yield/platform/platform_types.h"
using namespace YIELD;

#include <string>
using std::string;

namespace babudb {

class GlobalRegistries;

class Stats
{
public:
  Stats() {};
  Stats(const string&, GlobalRegistries*);
  Stats(const Stats& other) 
    : name( other.name ), global_registries( NULL ),
      first_timestamp( other.first_timestamp ),
      last_sample_timestamp( other.last_sample_timestamp )
    {}

  virtual ~Stats();


  void sampleTimedStatistics();
  void triggerStats();

protected:
  virtual void calculateDerivatives( double ) = 0;

  string name;
  GlobalRegistries* global_registries;

  uint64_t first_timestamp, last_sample_timestamp, next_sample_timestamp;

};

class LogStats : public Stats
{
public:
  LogStats() : Stats() {}

  LogStats( const string& name, GlobalRegistries* r ) 
    : Stats( name, r ),
      log_file_length( 0 ), log_length( 0 ), gaps_length( 0 ), no_of_gaps( 0 ),
      no_of_records( 0 ), no_of_deletors( 0 ), bytes_appended_in_period( 0 ), appends_in_period( 0 )
    {}


  LogStats( const LogStats& o ) 
    : Stats( o ),
      log_file_length( o.log_file_length ), log_length( o.log_length ),
      gaps_length( o.gaps_length ), no_of_gaps( o.no_of_gaps ),
      no_of_records( o.no_of_records ), no_of_deletors( o.no_of_deletors ),
      bytes_appended_in_period( o.bytes_appended_in_period ), appends_in_period( o.appends_in_period ),
      bytes_appended_rate( o.bytes_appended_rate ), append_rate( o.append_rate )
    {}

  // Event
  const char* getQualifiedEventName()	{ return "LogStats"; }


public:
  void calculateDerivatives( double );

  unsigned int log_file_length; // the raw file length , log_length, gaps_length, no_of_gaps;	// for low-level fragmentation
  unsigned int log_length;	  // last write position

  unsigned int gaps_length;	  // unoccupied space (raw length - data incl header and footer)
  unsigned int no_of_gaps;	  // number of gaps between records

  unsigned int no_of_records, no_of_deletors;

  unsigned long bytes_appended_in_period, appends_in_period;			// for assessing disk I/O
  double bytes_appended_rate, append_rate;
};

class LogDatabaseStats : public LogStats
{
public:
  LogDatabaseStats( const string& name, GlobalRegistries* r ) : LogStats( name, r ),
    no_of_events( 0 ), cum_event_size( 0 ), no_of_ops( 0 ), cum_ops_size( 0 ),
    no_of_orphaned_params( 0 ), cum_orphaned_params_size( 0 ),
    no_of_blocks( 0 ), no_of_updates( 0 ), cleanup_records_moved( 0 ), cleanup_offset( 0 ),
    no_of_updates_in_period( 0 ), update_rate( 0 ) {}

  LogDatabaseStats( const LogDatabaseStats& o ) : LogStats( o ),
    no_of_events( o.no_of_events ), cum_event_size( o.cum_event_size ),
    no_of_ops( o.no_of_ops ), cum_ops_size( o.cum_ops_size ),
    no_of_orphaned_params( o.no_of_orphaned_params ), cum_orphaned_params_size( o.cum_orphaned_params_size ),
    no_of_blocks( o.no_of_blocks ), no_of_updates( o.no_of_updates ),
    cleanup_records_moved( o.cleanup_records_moved ), cleanup_offset( o.cleanup_offset ),
    no_of_updates_in_period( o.no_of_updates_in_period ), update_rate( o.update_rate ) {}

  // Event
  const char* getQualifiedEventName()	{ return "LogDatabaseStats"; }

public:
  void calculateDerivatives( double );

  // Statistics
  // records are: events, operation headers, operation parameters
  unsigned int no_of_events;		// Rec. Source/Target, region headers
  unsigned int cum_event_size;

  unsigned int no_of_ops;
  unsigned int cum_ops_size;

  unsigned int no_of_orphaned_params;				// operations or op. paramters that are not referenced by an index
  unsigned int cum_orphaned_params_size;		// and their cum. size

  unsigned int no_of_blocks;
  unsigned int no_of_updates;

  unsigned int cleanup_records_moved, cleanup_offset;

  unsigned long no_of_updates_in_period;
  double update_rate;
};

}

#endif
