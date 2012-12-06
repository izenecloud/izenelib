/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id: toku_os_types.h 45903 2012-07-19 13:06:39Z leifwalsh $"
#ident "Copyright (c) 2007-2012 Tokutek Inc.  All rights reserved."

#if !defined(TOKU_OS_TYPES_H)
#define TOKU_OS_TYPES_H


#include <sys/types.h>
#include <sys/stat.h>

typedef int toku_os_handle_t;

struct fileid {
    dev_t st_dev; /* device and inode are enough to uniquely identify a file in unix. */
    ino_t st_ino;
};

typedef struct stat toku_struct_stat;

// windows compat
#if !defined(O_BINARY)
#define O_BINARY 0
#endif


#endif
