/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
extern "C" {
#ident "$Id: db-insert.c 45905 2012-07-19 14:19:44Z prohaska $"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Define BDB if you want to compile this to use Berkeley DB
#include <stdint.h>
#include <inttypes.h>
#ifdef BDB
#include <sys/types.h>
#include <3rdparty/tokufractaltreeindex/db.h>
#define DIRSUF bdb
#else
#include <3rdparty/tokufractaltreeindex/tokudb.h>
#define DIRSUF tokudb
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#ident "$Id: db-scan.c 45905 2012-07-19 14:19:44Z prohaska $"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."

/* Scan the bench.tokudb/bench.db over and over. */
#define DONT_DEPRECATE_MALLOC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>


#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
namespace sf1r
{
static inline float toku_tdiff (struct timeval *a, struct timeval *b) {
    return (a->tv_sec - b->tv_sec) +1e-6*(a->tv_usec - b->tv_usec);
}

#if !defined(DB_PRELOCKED_WRITE)
#define NO_DB_PRELOCKED
#define DB_PRELOCKED_WRITE 0
#endif

int verbose=1;

enum { SERIAL_SPACING = 1<<6 };
enum { DEFAULT_ITEMS_TO_INSERT_PER_ITERATION = 1<<20 };
enum { DEFAULT_ITEMS_PER_TRANSACTION = 1<<14 };

static void insert (long long v);
#define CKERR(r) ({ int __r = r; if (__r!=0) fprintf(stderr, "%s:%d error %d %s\n", __FILE__, __LINE__, __r, db_strerror(r)); assert(__r==0); })
#define CKERR2(r,rexpect) if (r!=rexpect) fprintf(stderr, "%s:%d error %d %s\n", __FILE__, __LINE__, r, db_strerror(r)); assert(r==rexpect);

/* default test parameters */
int keysize = sizeof (long long);
int valsize = sizeof (long long);
int pagesize = 0;
long long cachesize = 128*1024*1024;
int do_1514_point_query = 0;
int dupflags = 0;
int noserial = 0; // Don't do the serial stuff
int norandom = 0; // Don't do the random stuff
int prelock  = 0;
int prelockflag = 0;
int items_per_transaction = DEFAULT_ITEMS_PER_TRANSACTION;
int items_per_iteration   = DEFAULT_ITEMS_TO_INSERT_PER_ITERATION;
int finish_child_first = 0;  // Commit or abort child first (before doing so to the parent).  No effect if child does not exist.
int singlex_child = 0;  // Do a single transaction, but do all work with a child
int singlex = 0;  // Do a single transaction
int singlex_create = 0;  // Create the db using the single transaction (only valid if singlex)
int insert1first = 0;  // insert 1 before doing the rest
int check_small_rollback = 0; // verify that the rollback logs are small (only valid if singlex)
int do_transactions = 0;
int if_transactions_do_logging = DB_INIT_LOG; // set this to zero if we want no logging when transactions are used
int do_abort = 0;
int n_insertions_since_txn_began=0;
int env_open_flags = DB_CREATE|DB_PRIVATE|DB_INIT_MPOOL;
u_int32_t put_flags = 0;
double compressibility = -1; // -1 means make it very compressible.  1 means use random bits everywhere.  2 means half the bits are random.
int do_append = 0;
int do_checkpoint_period = 0;
u_int32_t checkpoint_period = 0;

static void do_prelock(DB* db, DB_TXN* txn) {
    if (prelock) {
#if !defined(NO_DB_PRELOCKED)
        int r = db->pre_acquire_table_lock(db, txn);
        assert(r==0);
#else
	db = db; txn = txn;
#endif
    }
}

#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)
const char *dbdir = "./bench."  STRINGIFY(DIRSUF);
char *dbfilename = "bench.db";
char *dbname;

DB_ENV *dbenv;
DB *db;
DB_TXN *parenttid=0;
DB_TXN *tid=0;


 const char *pname;
 enum run_mode { RUN_HWC, RUN_LWC, RUN_VERIFY, RUN_RANGE} run_mode = RUN_HWC;
 int do_txns=1;//, prelock=0, prelockflag=0;
 u_int32_t lock_flag = 0;
 long limitcount=-1;
 //u_int32_t cachesize = 127*1024*1024;
 u_int64_t start_range = 0, end_range = 0;
 int n_experiments = 2;
 int bulk_fetch = 1;
 int env_open_flags_yesx = DB_CREATE|DB_PRIVATE|DB_INIT_MPOOL|DB_INIT_TXN|DB_INIT_LOG|DB_INIT_LOCK;
 int env_open_flags_nox = DB_CREATE|DB_PRIVATE|DB_INIT_MPOOL;

}
}
