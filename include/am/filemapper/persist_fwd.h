#ifndef PERSIST_FWD_H
#define PERSIST_FWD_H

#include <stdexcept>
#include <types.h>

NS_IZENELIB_AM_BEGIN

enum
{
    shared_heap=1,
    private_map=2,
    auto_grow=4,
    temp_heap=8,
    create_new=16,
    read_only=32,
    lock_cs=64,
    lock_ipc=128,
    ipc_heap=auto_grow|lock_ipc,
    local_heap=auto_grow|lock_cs
};

class failed_to_create : public std::bad_alloc
{
};

class failed_to_map : public std::bad_alloc
{
};

NS_IZENELIB_AM_END

#endif

