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

#ifndef LUX_IO_BTREE_H
#define LUX_IO_BTREE_H

#include "dbm.h"

namespace Lux
{
namespace IO
{

class Data;

static const uint32_t MAX_KSIZE = 255;
static const uint32_t CLUSTER_MAX_VSIZE = 255;
static const uint32_t NONCLUSTER_MAX_VSIZE
= std::numeric_limits<uint32_t>::max();
static const uint32_t BT_ALLOCATE_UNIT = 100;

// global header
typedef struct
{
    char magic[8];
    uint32_t num_keys;
    uint32_t num_nodes;
    uint32_t node_size;
    uint32_t root_id;
    uint32_t num_leaves;
    uint32_t num_nonleaves;
    uint32_t num_resized;
    uint32_t num_alloc_pages;
    store_mode_t smode;
    padding_mode_t pmode;
    uint32_t padding;
    uint16_t init_data_size;
    uint8_t index_type;
} btree_header_t;

typedef uint32_t node_id_t;
typedef struct
{
    bool is_root;
    bool is_leaf;
    uint32_t id;
    uint16_t num_keys;
    uint16_t data_off;
    uint16_t free_off;
    uint16_t free_size;
    uint32_t prev_id; // used only in leaf
    uint32_t next_id; // used only in leaf
} node_header_t;
typedef char * node_body_t;

typedef struct
{
    node_header_t *h;
    node_body_t *b;
} node_t;

typedef struct
{
    const void *key;
    uint16_t key_size;
    const void *val;
    uint32_t val_size;
    uint32_t size; // entry size stored in pages
    insert_mode_t mode;
} entry_t;
typedef entry_t up_entry_t;

typedef struct
{
    uint16_t off;
    uint16_t size;
} slot_t;

typedef enum
{
    KEY_FOUND,
    KEY_BIGGER,
    KEY_SMALLEST,
    KEY_BIGGEST
} find_key_t;

typedef struct
{
    char *data_p;
    char *slot_p;
    find_key_t type;
} find_res_t;

typedef enum
{
    OP_UNSPECIFIED,
    OP_SELCT,
    OP_INSERT,
    OP_DELETE,
    OP_CUR_FIRST,
    OP_CUR_LAST,
    OP_CUR_GET,
    OP_CUR_LBOUND
} op_mode_t;

typedef struct
{
    node_id_t node_id;
    uint16_t slot_index; // 0: biggest, num_keys-1: smallest
    bool is_set;
} cursor_t;

typedef int (*CMP)(data_t &d1, data_t &d2);

/*
 * Class Btree
 */
class Btree
{
public:
    Btree(db_index_t index_type = NONCLUSTER);
    ~Btree();
    bool open(std::string db_name, db_flags_t oflags);
    bool close(void);
    data_t *get(const void *key, uint32_t key_size);
    data_t *get(data_t *k);
    bool get(data_t *k, data_t **v, alloc_type_t atype = USER);
    bool put(const void *key, uint32_t key_size,
             const void *val, uint32_t val_size,
             insert_mode_t flags = OVERWRITE);
    bool put(data_t *k, data_t *v, insert_mode_t flags = OVERWRITE);
    bool del(const void *key, uint32_t key_size);
    bool del(data_t *k);
    void set_index_type(db_index_t index_type);
    void set_page_size(uint32_t page_size);
    void set_lock_type(lock_type_t lock_type);
    void set_noncluster_params(store_mode_t smode,
                               padding_mode_t pmode = PO2,
                               uint32_t padding = 0, // for FIXED
                               uint32_t extra_exponent = 0); // for PO2
    void set_cmp_func(CMP cmp);
    void set_bulk_loading(bool is_bulk_loading);
    static void clean_data(data_t *d);
    // cursor
    cursor_t *cursor_init(void);
    bool cursor_fin(cursor_t *c);
    bool first(cursor_t *c);
    bool last(cursor_t *c);
    bool get(cursor_t *c, data_t *key);
    bool lower_bound(cursor_t *c, data_t *key);
    bool next(cursor_t *c);
    bool prev(cursor_t *c);
    bool cursor_get(cursor_t *c, data_t **key, data_t **val, alloc_type_t atype);
    uint32_t size();
    // comparison functions
    static int str_cmp_func(data_t &d1, data_t &d2);
    static int int32_cmp_func(data_t &d1, data_t &d2);
    static int uint32_cmp_func(data_t &d1, data_t &d2);
    // debug method
    void show_root(void);
    void show_db_header(void);
    void show_node(uint32_t id);

private:
    int fd_;
    db_flags_t oflags_;
    char *map_;
    btree_header_t *dh_;
    CMP cmp_;
    db_index_t index_type_;
    uint32_t page_size_;
    Data *dt_;
#ifdef LUXIO_HAVE_LIBPTHREAD
    pthread_rwlock_t rwlock_;
#endif
    uint32_t num_allocated_;
    uint32_t node_size_;
    uint32_t num_resized_;
    lock_type_t lock_type_;
    store_mode_t smode_;
    padding_mode_t pmode_;
    uint32_t padding_;
    uint32_t extra_exponent_;
    bool is_bulk_loading_;

    bool open_(std::string db_name, db_flags_t oflags);
    node_t *_init_node(uint32_t id, bool is_root, bool is_leaf);
    node_t *_alloc_node(uint32_t id);
    bool find(node_id_t id, data_t *k, data_t **v, alloc_type_t atype);
    bool get_data(char *p, data_t **data, alloc_type_t atype);
    void prepend_size_and_copy(char *to, char *from, uint8_t size);
    bool insert(entry_t *entry);
    bool _insert(node_id_t id, entry_t *entry, up_entry_t **up_entry, bool &is_split);
    bool _del(node_id_t id, entry_t *entry);
    node_id_t _find_next(node_t *node, entry_t *entry,
                         op_mode_t op_mode = OP_UNSPECIFIED);
    bool put_entry_in_leaf(node_t *node, entry_t *entry);
    void put_entry_in_nonleaf(node_t *node, entry_t *entry);
    void put_entry(node_t *node, entry_t *entry, find_res_t *r);
    void find_key(node_t *node, const void *key, uint32_t key_size, find_res_t *r);
    bool append_page(void);
    bool alloc_pages(uint32_t num_pages, uint32_t page_size);
    bool realloc_pages(uint32_t num_pages, uint32_t page_size);
    bool split_node(node_t *node, node_t *new_node, up_entry_t **up_entry);
    void make_leftmost_ptr(node_t *node, char *ptr);
    void set_node_header(node_header_t *h, uint16_t off, uint16_t num_keys);
    void copy_entries(char *dp, char *sp, node_t *node, slot_t *slots,
                      uint16_t &data_off, int slot_from, int slot_to);
    char *get_prefix_key(char *big, char *small);
    up_entry_t *get_up_entry(node_t *node, slot_t *slots,
                             uint16_t boundary_off, node_id_t up_node_id);
    void clean_up_entry(up_entry_t **up_entry);
    bool remap(void);
    bool unlock_db(void);
    bool rlock_db(void);
    bool wlock_db(void);
    bool check_key(uint32_t key_size);
    bool check_val(uint32_t val_size);
    bool cursor_find(cursor_t *c, node_id_t id,
                     data_t *key, op_mode_t op_mode);
};

}
}

#endif
