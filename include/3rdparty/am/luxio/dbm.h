/*
 * Copyright (C) 2008-2009 Hiroyuki Yamada
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef LUX_IO_DBM_H
#define LUX_IO_DBM_H

#include "luxio-config.h"
#include "types.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <stdexcept>
#include <limits>

namespace Lux
{
namespace IO
{

static const uint32_t MIN_PAGESIZE = 1024;
static const uint32_t MAX_PAGESIZE = 65536;
static const uint32_t MIN_BLOCKSIZE = 1024;
static const uint32_t MAX_BLOCKSIZE = 65536;
typedef uint32_t block_id_t;

#pragma pack(2)
typedef struct
{
    block_id_t id;
    uint16_t off;
} data_ptr_t;
#pragma pack()

typedef struct
{
    const void *data;
    uint32_t size;
    uint32_t user_alloc_size;
} data_t;

typedef enum
{
    NONCLUSTER,
    CLUSTER
} db_index_t;

typedef enum
{
    OVERWRITE,
    NOOVERWRITE,
    APPEND // it's only supported in non-cluster index
} insert_mode_t;

typedef enum
{
    NO_LOCK,
    LOCK_THREAD,
    LOCK_PROCESS
} lock_type_t;

typedef enum
{
    USER,
    SYSTEM
} alloc_type_t;

typedef enum
{
    Padded,
    Linked
} store_mode_t;

typedef enum
{
    NOPADDING,
    FIXEDLEN,
    RATIO,
    BLOCKALIGNED,
    PO2 // power of 2
} padding_mode_t;

}
}

#endif
