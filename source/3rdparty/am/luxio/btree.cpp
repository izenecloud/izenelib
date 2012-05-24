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

#include <luxio-config.h>
#include <btree.h>
#include <util.h>
#include <data.h>
#include <exception.h>
#ifdef LUXIO_HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#define ALLOC_AND_COPY(s1, s2, size) \
  char s1[size+1]; \
  memcpy(s1, s2, size); \
  s1[size] = '\0';

namespace Lux
{
namespace IO
{

static const char *BTMAGIC = "LUXBT001";

Btree::Btree(db_index_t index_type)
    : map_(NULL),
      cmp_(str_cmp_func),
      index_type_(index_type),
      page_size_(getpagesize()),
      dt_(NULL),
      lock_type_(NO_LOCK),
      smode_(Linked),
      pmode_(PO2),
      padding_(0),
      extra_exponent_(0),
      is_bulk_loading_(false)
{
#ifdef LUXIO_HAVE_LIBPTHREAD
    if (pthread_rwlock_init(&rwlock_, NULL) != 0)
    {
        error_log("pthread_rwlock_init failed");
        exit(-1);
    }
#endif
}

Btree::~Btree()
{
    if (dt_ != NULL)
    {
        delete dt_;
        dt_ = NULL;
    }
    if (map_ != NULL)
    {
        close();
    }
#ifdef LUXIO_HAVE_LIBPTHREAD
    if (pthread_rwlock_destroy(&rwlock_) != 0)
    {
        error_log("pthread_rwlock_destroy failed");
        exit(-1);
    }
#endif
}

bool Btree::open(std::string db_name, db_flags_t oflags)
{
    if (lock_type_ == LOCK_THREAD)
    {
#ifdef LUXIO_HAVE_LIBPTHREAD
        pthread_rwlock_wrlock(&rwlock_);
#endif
    }
    bool res = open_(db_name, oflags);
    if (lock_type_ == LOCK_THREAD)
    {
#ifdef LUXIO_HAVE_LIBPTHREAD
        pthread_rwlock_unlock(&rwlock_);
#endif
    }

    return res;
}

bool Btree::close(void)
{
    if (!wlock_db())
    {
        return false;
    }
    if (map_ != NULL)
    {
        if (oflags_ != O_RDONLY)
        {
            if (msync(map_, dh_->node_size * dh_->num_alloc_pages, MS_SYNC) < 0)
            {
                error_log("msync failed.");
                unlock_db();
                return false;
            }
        }
        if (munmap(map_, dh_->node_size * dh_->num_alloc_pages) < 0)
        {
            error_log("munmap failed.");
            unlock_db();
            return false;
        }
    }
    map_ = NULL;
    if (!unlock_db())
    {
        return false;
    }
    if (::close(fd_) < 0)
    {
        error_log("close failed.");
        return false;
    }
    return true;
}

data_t *Btree::get(const void *key, uint32_t key_size)
{
    data_t k = {key, key_size};
    return get(&k);
}

data_t *Btree::get(data_t *k)
{
    data_t *v = NULL;
    if (!check_key(k->size))
    {
        return NULL;
    }
    if (!rlock_db())
    {
        return NULL;
    }
    if (!find(dh_->root_id, k, &v, SYSTEM))
    {
        clean_data(v);
    }
    if (!unlock_db())
    {
        return NULL;
    }
    return v;
}

bool Btree::get(data_t *k, data_t **v, alloc_type_t atype)
{
    bool res = true;
    if (!check_key(k->size))
    {
        return false;
    }
    if (!rlock_db())
    {
        return false;
    }
    res = find(dh_->root_id, k, v, atype);
    if (!res && atype == SYSTEM)
    {
        clean_data(*v);
    }
    if (!unlock_db())
    {
        return false;
    }
    return res;
}

bool Btree::put(const void *key, uint32_t key_size,
                const void *val, uint32_t val_size,
                insert_mode_t flags)
{
    data_t k = {key, key_size};
    data_t v = {val, val_size};
    return put(&k, &v, flags);
}

bool Btree::put(data_t *k, data_t *v, insert_mode_t flags)
{
    static char data[CLUSTER_MAX_VSIZE + sizeof(uint8_t)];
    bool res = true;
    if (!check_key(k->size) || !check_val(v->size))
    {
        return false;
    }
    if (!wlock_db())
    {
        return false;
    }

    uint32_t val_size = v->size + sizeof(uint8_t);
    data_t val = {data, val_size}; // for prepended size in cluster mode
    uint32_t entry_size = k->size;
    if (dh_->index_type == CLUSTER)
    {
        memset(data, 0, CLUSTER_MAX_VSIZE + sizeof(uint8_t));
        prepend_size_and_copy(data, (char *) v->data, v->size);
        v = &val;
        entry_size += val.size;
    }
    else
    {
        entry_size += sizeof(data_ptr_t);
    }
    entry_t entry = {(char *) k->data, k->size,
                     (char *) v->data, v->size,
                     entry_size, flags
                    };

    // [TODO] should return error status
    try
    {
        res = insert(&entry);
    }
    catch (const std::bad_alloc &e)
    {
        res = false;
    }
    catch (const disk_alloc_error &e)
    {
        error_log(e.what());
        error_log("possibly out of disk space");
        res = false;
    }
    catch (const mmap_alloc_error &e)
    {
        error_log(e.what());
        error_log("possibly out of memory");
        res = false;
    }
    catch (const fatal_error &e)
    {
        throw;
    }

    if (!unlock_db())
    {
        return false;
    }
    return res;
}

bool Btree::del(const void *key, uint32_t key_size)
{
    data_t k = {key ,key_size};
    return del(&k);
}

bool Btree::del(data_t *k)
{
    if (!check_key(k->size))
    {
        return false;
    }
    entry_t entry = {k->data, k->size, NULL, 0, 0};

    if (!wlock_db())
    {
        return false;
    }
    bool res = _del(dh_->root_id, &entry);
    if (!unlock_db())
    {
        return false;
    }

    return res;
}

void Btree::set_index_type(db_index_t index_type)
{
    if (index_type == CLUSTER ||
            index_type == NONCLUSTER)
    {
        index_type_ = index_type;
    }
}

void Btree::set_page_size(uint32_t page_size)
{
    if (page_size > MAX_PAGESIZE ||
            page_size < MIN_PAGESIZE)
    {
        return;
    }
    page_size_ = page_size;
}

void Btree::set_lock_type(lock_type_t lock_type)
{
    lock_type_ = lock_type;
}

// only for noncluster database
void Btree::set_noncluster_params(store_mode_t smode, padding_mode_t pmode,
                                  uint32_t padding, uint32_t extra_exponent)
{
    smode_ = smode;
    pmode_ = pmode;
    padding_ = padding;
    extra_exponent_ = extra_exponent;
}

void Btree::set_cmp_func(CMP cmp)
{
    cmp_ = cmp;
}

void Btree::set_bulk_loading(bool is_bulk_loading)
{
    is_bulk_loading_ = is_bulk_loading;
}

void Btree::clean_data(data_t *d)
{
    if (d != NULL)
    {
        if ((char *) (d->data) != NULL)
        {
            delete [] (char *) (d->data);
        }
        delete d;
    }
    d = NULL;
}

/* cursors */
/* [TODO] cursors methods are not thread-safe for now */
cursor_t *Btree::cursor_init(void)
{
    cursor_t *c = new cursor_t;
    c->is_set = false;
    return c;
}

bool Btree::cursor_fin(cursor_t *c)
{
    if (c != NULL)
    {
        delete c;
        c = NULL;
    }
    return true;
}

bool Btree::first(cursor_t *c)
{
    if (!cursor_find(c, dh_->root_id, NULL, OP_CUR_FIRST))
    {
        return false;
    }
    if (c->node_id == 0)
    {
        return false;
    }
    c->is_set = true;
    return true;
}

bool Btree::last(cursor_t *c)
{
    if (!cursor_find(c, dh_->root_id, NULL, OP_CUR_LAST))
    {
        return false;
    }
    if (c->node_id == 0)
    {
        return false;
    }
    c->is_set = true;
    return true;
}

bool Btree::get(cursor_t *c, data_t *key)
{
    if (!cursor_find(c, dh_->root_id, key, OP_CUR_GET))
    {
        return false;
    }
    if (c->node_id == 0)
    {
        return false;
    }
    c->is_set = true;
    return true;
}

bool Btree::lower_bound(cursor_t *c, data_t *key)
{
    if (!cursor_find(c, dh_->root_id, key, OP_CUR_LBOUND))
    {
        return false;
    }
    if (c->node_id == 0)
    {
        return false;
    }
    c->is_set = true;
    return true;
}

// next bigger key
bool Btree::next(cursor_t *c)
{
    if (!c->is_set)
    {
        if (!first(c))
        {
            return false;
        }
        return true;
    }
    if (c->slot_index == 0)
    {
        node_id_t curr_node_id = c->node_id;
        bool res = true;
        while (true)
        {
            node_t *node = _alloc_node(curr_node_id);
            curr_node_id = node->h->next_id;
            delete node;
            if (curr_node_id == 0)
            {
                res = false;
                break;
            }
            node_t *next_node = _alloc_node(curr_node_id);
            if (next_node->h->num_keys >= 1)
            {
                c->slot_index = next_node->h->num_keys - 1;
                c->node_id = curr_node_id;
                delete next_node;
                break;
            }
            delete next_node;
        }
        if (!res)
        {
            return false;
        }
    }
    else
    {
        --c->slot_index;
    }
    return true;
}

// next smaller key
bool Btree::prev(cursor_t *c)
{
    if (!c->is_set)
    {
        if (!last(c))
        {
            return false;
        }
        return true;
    }
    if (c->node_id == 0)
    {
        return false;
    }
    node_t *node = _alloc_node(c->node_id);
    bool res = true;
    if (c->slot_index == node->h->num_keys - 1)
    {
        node_id_t curr_node_id = node->h->prev_id;
        while (true)
        {
            if (curr_node_id == 0)
            {
                res = false;
                break;
            }
            node_t *node_prev = _alloc_node(curr_node_id);
            if (node_prev->h->num_keys != 0)
            {
                c->slot_index = 0;
                c->node_id = curr_node_id;
                delete node_prev;
                break;
            }
            else
            {
                curr_node_id = node_prev->h->prev_id;
            }
            delete node_prev;
        }
    }
    else
    {
        ++c->slot_index;
    }
    delete node;
    if (c->node_id == 0 || !res)
    {
        return false;
    }
    return true;
}

bool Btree::cursor_get(cursor_t *c, data_t **key, data_t **val, alloc_type_t atype)
{
    node_t *node = _alloc_node(c->node_id);
    slot_t *slots = (slot_t *) ((char *) node->b + node->h->free_off);
    slot_t *slot = slots + c->slot_index;

    bool res;
    char *p = (char *) node->b + slot->off;
    if (atype == SYSTEM)
    {
        *key = new data_t;
        (*key)->data = new char[slot->size+1];
        ((char *) (*key)->data)[slot->size] = '\0';
    }
    memcpy((char *) (*key)->data, p, slot->size);
    (*key)->size = slot->size;
    res = get_data(p + slot->size, val, atype);
    delete node;

    return res;
}

// comparison functions
int Btree::str_cmp_func(data_t &d1, data_t &d2)
{
    return strcmp((char *) d1.data, (char *) d2.data);
}

int Btree::int32_cmp_func(data_t &d1, data_t &d2)
{
    return (*(int32_t *) d1.data - *(int32_t *) d2.data);
};

int Btree::uint32_cmp_func(data_t &d1, data_t &d2)
{
    if (*(uint32_t *) d1.data < *(uint32_t *) d2.data) return -1;
    else if (*(uint32_t *) d1.data == *(uint32_t *) d2.data) return 0;
    else return 1;
};

void Btree::show_root(void)
{
    show_node(dh_->root_id);
}

void Btree::show_db_header()
{
    std::cout << "----- ROOT -----" << std::endl
              << "num_keys: " << dh_->num_keys << std::endl
              << "num_nodes: " << dh_->num_nodes << std::endl
              << "node_size: " << dh_->node_size << std::endl
              << "init_data_size: " << dh_->init_data_size << std::endl
              << "root_id: " << dh_->root_id << std::endl
              << "num_leaves: " << dh_->num_leaves << std::endl
              << "num_nonleaves: " << dh_->num_nonleaves << std::endl
              << "index_type: " << (int) dh_->index_type << std::endl;
}

uint32_t Btree::size()
{
    return  dh_->num_keys;
}

// debug method
void Btree::show_node(uint32_t id)
{
    std::cout << std::endl;
    std::cout << "----- NODE " << id << " -----" << std::endl;
    node_t *node = _alloc_node(id);
    if (node == NULL)
    {
        std::cout << "node[ " << id << "] is not allocated, yet" << std::endl;
        return;
    }
    std::cout << "is_root: " << node->h->is_root << std::endl
              << "is_leaf: " << node->h->is_leaf << std::endl
              << "id: " << node->h->id << std::endl
              << "num_keys: " << node->h->num_keys << std::endl
              << "data_off: "<< node->h->data_off << std::endl
              << "free_off: " << node->h->free_off << std::endl
              << "free_size: " << node->h->free_size << std::endl;

    if (!node->h->is_leaf)
    {
        node_id_t leftmost;
        memcpy(&leftmost, (char *) node->b, sizeof(node_id_t));
        std::cout << "leftmost[" << leftmost << "]" << std::endl;
    }

    char *slot_p = (char *) node->h + dh_->node_size; // point to the tail of the node
    char *body_p = (char *) node->b;
    for (int i = 1; i <= node->h->num_keys; ++i)
    {
        slot_p -= sizeof(slot_t);
        slot_t *slot = (slot_t *) slot_p;
        std::cout << "off[" << slot->off << "], size[" << slot->size << "]" << std::endl;

        char key_buf[256];
        uint32_t val;
        memset(key_buf, 0, 256);
        memcpy(key_buf, body_p + slot->off, slot->size);
        memcpy(&val, body_p + slot->off + slot->size, sizeof(node_id_t));
        std::cout << "key[" << key_buf << "]" << std::endl;
        std::cout << "val[" << val << "]" << std::endl;
    }
    delete node;
}

// private methods

bool Btree::open_(std::string db_name, db_flags_t oflags)
{
    std::string idx_db_name = db_name + ".bidx";
    fd_ = _open(idx_db_name.c_str(), oflags, 00644);
    if (fd_ < 0)
    {
        error_log("open failed.");
        return false;
    }
    oflags_ = oflags;
    if (lock_type_ == LOCK_PROCESS)
    {
        if (flock(fd_, LOCK_EX) != 0)
        {
            error_log("flock failed.");
            return false;
        }
    }

    struct stat stat_buf;
    if (fstat(fd_, &stat_buf) == -1 || !S_ISREG(stat_buf.st_mode))
    {
        error_log("fstat failed.");
        return false;
    }

    btree_header_t dh;
    memset(&dh, 0, sizeof(btree_header_t));
    if (stat_buf.st_size == 0 && oflags & DB_CREAT)
    {
        // initialize the header for the newly created file
        memcpy(dh.magic, BTMAGIC, strlen(BTMAGIC));
        dh.num_keys = 0;
        // one for db_header, one for root node and one for leaf node
        dh.num_nodes = 3;
        dh.node_size = page_size_;
        dh.init_data_size = dh.node_size - sizeof(node_header_t);
        dh.root_id = 1;
        dh.num_leaves = 0;
        dh.num_nonleaves = 0; // root node
        dh.num_resized = 0;
        dh.num_alloc_pages = 0;
        dh.smode = smode_;
        dh.pmode = pmode_;
        dh.padding = padding_;
        dh.index_type = index_type_;

        if (_write(fd_, &dh, sizeof(btree_header_t)) < 0)
        {
            error_log("write failed.");
            return false;
        }
        if (!alloc_pages(BT_ALLOCATE_UNIT, dh.node_size))
        {
            error_log("alloc_page failed.");
            return false;
        }

        // root node and the first leaf node
        node_t *root = _init_node(1, true, false);
        node_t *leaf = _init_node(2, false, true);

        // make a link from root to the first leaf node
        memcpy(root->b, &(leaf->h->id), sizeof(node_id_t));
        root->h->data_off += sizeof(node_id_t);
        root->h->free_size -= sizeof(node_id_t);

        delete root;
        delete leaf;

    }
    else
    {
        if (_read(fd_, &dh, sizeof(btree_header_t)) < 0)
        {
            error_log("read failed.");
            return false;
        }

        if (stat_buf.st_size != dh.num_alloc_pages * dh.node_size)
        {
            error_log("database corruption occured");
            return false;
        }

        map_ = (char *) _mmap(fd_, dh.node_size * dh.num_alloc_pages, oflags);
        if (map_ == NULL)
        {
            error_log("mmap failed.");
            return false;
        }
    }

    dh_ = (btree_header_t *) map_;
    num_allocated_ = dh_->num_alloc_pages;
    node_size_ = dh_->node_size;
    num_resized_ = dh_->num_resized;

    if (index_type_ != dh_->index_type)
    {
        error_log("wrong index type.");
        return false;
    }

    if (dh_->index_type == NONCLUSTER)
    {
        if(dh_->smode == Padded)
        {
            dt_ = new PaddedData(dh_->pmode, dh_->padding);
        }
        else if (dh_->smode == Linked)
        {
            dt_ = new LinkedData(dh_->pmode, dh_->padding);
        }
        else
        {
            error_log("specified store mode doesn't exitst.");
        }
        dt_->set_po2_extra_exponent(extra_exponent_);
        std::string data_db_name = db_name + ".data";
        if (!dt_->open(data_db_name.c_str(), oflags))
        {
            error_log("opening data database failed.");
            return false;
        }
    }

    if (lock_type_ == LOCK_PROCESS)
    {
        if (flock(fd_, LOCK_UN) != 0)
        {
            error_log("flock failed.");
            return false;
        }
    }
    return true;
}

node_t *Btree::_init_node(uint32_t id, bool is_root, bool is_leaf)
{
    assert(id >= 1 && id <= dh_->num_nodes - 1);
    char *node_p = (char *) &(map_[dh_->node_size * id]);

    node_header_t *node_hdr_p = (node_header_t *) node_p;
    node_hdr_p->is_root = is_root;
    node_hdr_p->is_leaf = is_leaf;
    node_hdr_p->id = id;
    node_hdr_p->num_keys = 0;
    node_hdr_p->data_off = 0;
    node_hdr_p->free_off = dh_->node_size - sizeof(node_header_t);;
    node_hdr_p->free_size = node_hdr_p->free_off;
    node_hdr_p->prev_id = 0; // 0 means no link
    node_hdr_p->next_id = 0; // 0 means no link
    node_body_t *node_body_p = (node_body_t *) (node_p + sizeof(node_header_t));

    node_t *node = new node_t;
    node->h = node_hdr_p;
    node->b = node_body_p;

    if (is_leaf)
    {
        ++(dh_->num_leaves);
    }
    else
    {
        ++(dh_->num_nonleaves);
    }

    return node;
}

node_t *Btree::_alloc_node(uint32_t id)
{
    assert(id >= 1 && id <= dh_->num_nodes - 1);
    char *node_p = (char *) &(map_[dh_->node_size * id]);
    node_t *node = new node_t;
    node->h = (node_header_t *) node_p;
    node->b = (node_body_t *) (node_p + sizeof(node_header_t));
    return node;
}

bool Btree::find(node_id_t id, data_t *k, data_t **v, alloc_type_t atype)
{
    assert(id >= 1 && id <= dh_->num_nodes - 1);
    bool res = true;;
    entry_t entry = {k->data, k->size, NULL, 0, 0};

    node_t *node = _alloc_node(id);
    if (node->h->is_leaf)
    {
        find_res_t r;
        find_key(node, entry.key, entry.key_size, &r);
        if (r.type == KEY_FOUND)
        {
            slot_t *slot = (slot_t *) r.slot_p;
            res = get_data(r.data_p + slot->size, v, atype);
        }
    }
    else
    {
        node_id_t next_id = _find_next(node, &entry);
        res = find(next_id, k, v, atype);
    }
    delete node;
    return res;
}

bool Btree::get_data(char *p, data_t **data, alloc_type_t atype)
{
    if (dh_->index_type == CLUSTER)
    {
        uint8_t *size = (uint8_t *) p;
        if (atype == SYSTEM)
        {
            *data = new data_t;
            (*data)->data = (char *) new char[*size + 1];
            ((char *) (*data)->data)[*size] = '\0';
        }
        else
        {
            if ((*data)->user_alloc_size < *size)
            {
                error_log("allocated size is too small");
                return false;
            }
            if ((*data)->user_alloc_size >= (size_t)(*size + 1))
            {
                ((char *) (*data)->data)[*size] = '\0';
            }
        }
        (*data)->size = *size;
        memcpy((void *) (*data)->data, p + sizeof(uint8_t), *size);
    }
    else
    {
        data_ptr_t *data_ptr = (data_ptr_t *) p;
        if (!dt_->get(data_ptr, data, atype))
        {
            return false;
        }
    }
    return true;
}

void Btree::prepend_size_and_copy(char *to, char *from, uint8_t size)
{
    memcpy(to, &size, sizeof(uint8_t));
    memcpy(to + sizeof(uint8_t), from, size);
}

bool Btree::insert(entry_t *entry)
{
    bool is_split = false;
    up_entry_t *up_entry = NULL;

    if (!_insert(dh_->root_id, entry, &up_entry, is_split))
    {
        error_log("_insert failed.");
        return false;
    }

    // when split happens, the entry is not inserted.
    if (is_split)
    {
        up_entry = NULL;
        bool is_split = false;
        if (!_insert(dh_->root_id, entry, &up_entry, is_split))
        {
            error_log("_insert failed.");
            return false;
        }
        if (is_split)
        {
            // try couple of times (not forever)
            return false;
        }
    }
    return true;
}

bool Btree::_insert(node_id_t id, entry_t *entry, up_entry_t **up_entry, bool &is_split)
{
    vinfo_log("_insert");
    assert(id >= 1 && id <= dh_->num_nodes - 1);
    node_t *node = _alloc_node(id);
    if (node->h->is_leaf)
    {
        if (node->h->free_size >= entry->size + sizeof(slot_t))
        {
            // there is enough space, then just put the entry
            if (!put_entry_in_leaf(node, entry))
            {
                error_log("put_entry_in_leaf failed");
                return false;
            }
        }
        else
        {
            // no updating if the entry key exists

            if (!append_page())
            {
                error_log("append_page failed.");
                return false;
            }
            // must reallocate after remapped
            delete node;
            node = _alloc_node(id);

            // create new leaf node
            node_t *new_node = _init_node(dh_->num_nodes-1, false, true);
            if (!split_node(node, new_node, up_entry))
            {
                error_log("split_node failed.");
                return false;
            }

            delete new_node;
            is_split = true;
        }
    }
    else
    {
        node_id_t next_id = _find_next(node, entry, OP_INSERT);
        if (!_insert(next_id, entry, up_entry, is_split))
        {
            return false;
        }

        delete node;
        if (*up_entry == NULL)
        {
            return true;
        }

        // must reallocate after remapped
        node = _alloc_node(id);

        if (node->h->free_size >= (*up_entry)->size + sizeof(slot_t))
        {
            put_entry_in_nonleaf(node, *up_entry);
            clean_up_entry(up_entry);
        }
        else
        {
            if (!append_page())
            {
                error_log("append_page failed.");
                return false;
            }
            // must reallocate after remapped
            delete node;
            node = _alloc_node(id);

            // pointing the pushed up entry
            up_entry_t *e = *up_entry;
            *up_entry = NULL;

            node_t *new_node = _init_node(dh_->num_nodes-1, false, false);
            if (!split_node(node, new_node, up_entry))
            {
                error_log("split_node failed.");
                return false;
            }

            // compare e with up_e to decide which node putting e into
            data_t e_data = {e->key, e->key_size};
            data_t up_e_data = {(*up_entry)->key, (*up_entry)->key_size};

            if (cmp_(e_data, up_e_data) < 0)
            {
                put_entry_in_nonleaf(node, e); // goes to old node
            }
            else
            {
                put_entry_in_nonleaf(new_node, e); // goes to new node
            }
            delete new_node;

            if (node->h->is_root)
            {
                if (!append_page())
                {
                    error_log("append_page failed.");
                    return false;
                }
                delete node;
                node = _alloc_node(id);

                node_t *new_root = _init_node(dh_->num_nodes-1, true, false);
                make_leftmost_ptr(new_root, (char *) &(node->h->id));
                put_entry_in_nonleaf(new_root, *up_entry);
                // change root
                dh_->root_id = new_root->h->id;
                node->h->is_root = false;
                delete new_root;
                clean_up_entry(up_entry);
            }
            clean_up_entry(&e);
        }
    }
    delete node;
    return true;
}

bool Btree::_del(node_id_t id, entry_t *entry)
{
    assert(id >= 1 && id <= dh_->num_nodes - 1);
    node_t *node = _alloc_node(id);
    if (node->h->is_leaf)
    {
        find_res_t r;
        find_key(node, entry->key, entry->key_size, &r);
        if (r.type == KEY_FOUND)
        {
            // remove the data in non-clustered
            if (dh_->index_type == NONCLUSTER)
            {
                slot_t *slot = (slot_t *) r.slot_p;
                data_ptr_t *data_ptr = (data_ptr_t *) (r.data_p + slot->size);
                dt_->del(data_ptr);
            }
            // remove the slot
            char *p = (char *) node->b + node->h->free_off;
            if (p != r.slot_p)
            {
                memmove(p + sizeof(slot_t), p, r.slot_p - p);
            }
            node->h->free_off += sizeof(slot_t);
            node->h->free_size += sizeof(slot_t);
            --(node->h->num_keys);
            --(dh_->num_keys);
        }
    }
    else
    {
        node_id_t next_id = _find_next(node, entry);
        if (!_del(next_id, entry))
        {
            return false;
        }
    }
    delete node;
    return true;
}

node_id_t Btree::_find_next(node_t *node, entry_t *entry,
                            op_mode_t op_mode)
{
    node_id_t id;
    find_res_t r;

    if (op_mode == OP_CUR_FIRST)
    {
        r.type = KEY_SMALLEST;
    }
    else if (op_mode == OP_CUR_LAST ||
             (is_bulk_loading_ && op_mode == OP_INSERT))
    {
        // always expect biggest keys in bulk loading
        r.type = KEY_BIGGEST;
    }
    else
    {
        find_key(node, entry->key, entry->key_size, &r);
    }

    if (node->h->num_keys == 0 || r.type == KEY_SMALLEST)
    {
        memcpy(&id, (char *) node->b, sizeof(node_id_t));
    }
    else
    {
        slot_t *slot;
        if (r.type == KEY_BIGGEST)
        {
            char *free_p = (char *) node->b + node->h->free_off;
            slot = (slot_t *) free_p;
        }
        else
        {
            slot = (slot_t *) r.slot_p;
        }
        memcpy(&id, (char *) node->b + slot->off + slot->size, sizeof(node_id_t));
    }
    return id;
}

bool Btree::put_entry_in_leaf(node_t *node, entry_t *entry)
{
    vinfo_log("put_entry_in_leaf");
    find_res_t r;
    if (is_bulk_loading_)
    {
        // always expect biggest keys in bulk loading
        r.type = KEY_BIGGEST;
    }
    else
    {
        find_key(node, entry->key, entry->key_size, &r);
    }
    if (r.type == KEY_FOUND && entry->mode == NOOVERWRITE)
    {
        return true;
    }

    if (dh_->index_type == CLUSTER)
    {
        if (r.type == KEY_FOUND)
        {
            // append is not supported in b+-tree cluster index. only updating
            if (entry->mode == APPEND)
            {
                error_log("append is not supported in cluster index");
                return false;
            }
            char *val_p = (char *) r.data_p + entry->key_size;
            // check if updating entry is bigger than existing
            if (entry->val_size > *(uint8_t *) val_p + sizeof(uint8_t))
            {
                return false;
            }
            memcpy(val_p, entry->val, entry->val_size);
        }
        else
        {
            put_entry(node, entry, &r);
        }
    }
    else
    {
        entry_t _entry = {entry->key, entry->key_size,
                          NULL, sizeof(data_ptr_t), entry->size, entry->mode
                         };
        data_t data = {entry->val, entry->val_size};
        data_ptr_t *res_data_ptr;

        if (r.type == KEY_FOUND)
        {
            char *val_ptr = (char *) r.data_p + entry->key_size;
            data_ptr_t *data_ptr = (data_ptr_t *) val_ptr;

            // append or update the data, get the ptr to the data and update the index
            if (entry->mode == APPEND)
            {
                res_data_ptr = dt_->append(data_ptr, &data);
            }
            else
            {
                res_data_ptr = dt_->update(data_ptr, &data);
            }
            if (res_data_ptr == NULL)
            {
                return false;
            }
            memcpy(val_ptr, res_data_ptr, sizeof(data_ptr_t));
        }
        else
        {
            // put the data, get the ptr to the data and update the index
            res_data_ptr = dt_->put(&data);
            if (res_data_ptr == NULL)
            {
                return false;
            }
            _entry.val = res_data_ptr;
            put_entry(node, &_entry, &r);
        }
        dt_->clean_data_ptr(res_data_ptr);
    }
    return true;
}

void Btree::put_entry_in_nonleaf(node_t *node, entry_t *entry)
{
    vinfo_log("put_entry_in_nonleaf");

    find_res_t r;
    find_key(node, entry->key, entry->key_size, &r);
    put_entry(node, entry, &r);
}

void Btree::put_entry(node_t *node, entry_t *entry, find_res_t *r)
{
    vinfo_log("put_entry");
    // append entry
    char *data_p = (char *) node->b + node->h->data_off;
    char *free_p = (char *) node->b + node->h->free_off;
    memcpy(data_p, entry->key, entry->key_size);
    memcpy(data_p + entry->key_size, entry->val, entry->val_size);

    // organize ordered slots
    slot_t slot = { node->h->data_off, entry->key_size };

    if (r->type == KEY_BIGGEST ||
            (r->type == KEY_SMALLEST && node->h->num_keys == 0))
    {
        // prepend
        memcpy(free_p - sizeof(slot_t), &slot, sizeof(slot_t));
    }
    else if (r->type == KEY_SMALLEST)
    {
        // insert:KEY_SMALLEST (shifting all the slots)
        char *tail_p = (char *) node->b + dh_->init_data_size;
        int shift_size = tail_p - free_p;
        memmove(free_p - sizeof(slot_t), free_p, shift_size);
        memcpy(tail_p - sizeof(slot_t), &slot, sizeof(slot_t));
    }
    else
    {
        // insert:KEY_BIGGER (shifting some of the slots)
        int shift_size = r->slot_p - free_p;
        memmove(free_p - sizeof(slot_t), free_p, shift_size);
        memcpy(r->slot_p - sizeof(slot_t), &slot, sizeof(slot_t));
    }

    // update metadata
    node->h->data_off += entry->size;
    node->h->free_off -= sizeof(slot_t);
    node->h->free_size -= entry->size + sizeof(slot_t);
    if (node->h->is_leaf)
    {
        ++(dh_->num_keys);
    }
    ++(node->h->num_keys);
}

void Btree::find_key(node_t *node, const void *key, uint32_t key_size, find_res_t *r)
{
    //find_res_t *r = new find_res_t;
    char *slot_p = (char *) node->b + node->h->free_off;
    slot_t *slots = (slot_t *) slot_p;

    if (node->h->num_keys == 0)
    {
        r->type = KEY_SMALLEST;
        return;
    }

    // [TODO] API should be changed ? : take data_t instead of key and key_size
    data_t k = {key, key_size};

    char checked[node->h->num_keys];
    memset(checked, 0, node->h->num_keys);

    // binary search
    int low_bound = 0;
    int up_bound = node->h->num_keys - 1;
    int middle = node->h->num_keys / 2;
    bool is_found = false;
    bool is_going_upper = false;
    int last_middle = -1;
    while (1)
    {
        // the key is not found if it's already checked
        if (checked[middle])
        {
            break;
        }
        checked[middle] = 1;

        slot_t *slot = slots + middle;
        ALLOC_AND_COPY(stored_key, (char *) node->b + slot->off, slot->size);
        data_t stored_data = {stored_key, slot->size};

        int res = cmp_(k, stored_data);
        if (res == 0)
        {
            // found
            is_found = true;
            r->data_p = (char *) node->b + slot->off;
            break;
        }
        else if (res < 0)
        {
            // entry key is smaller (going to upper offset)
            low_bound = middle;
            last_middle = middle;
            div_t d = div(up_bound - middle, 2);
            middle = d.rem > 0 ? middle + d.quot + 1 : middle + d.quot;
            is_going_upper = true;
        }
        else
        {
            // entry key is bigger (going to lower offset)
            up_bound = middle;
            last_middle = middle;
            middle = low_bound + (middle - low_bound) / 2;
            is_going_upper = false;
        }
    }

    slot_t *slot = slots + middle;
    r->slot_p = (char *) slot;

    if (is_found)
    {
        r->type = KEY_FOUND;
    }
    else
    {
        if (is_going_upper)
        {
            if (middle == last_middle)
            {
                r->type = KEY_SMALLEST;
            }
            else
            {
                r->type = KEY_BIGGER;
            }
        }
        else
        {
            if (middle == last_middle)
            {
                r->type = KEY_BIGGEST;
            }
            else
            {
                slot = slots + (++middle);
                r->slot_p = (char *) slot;
                r->type = KEY_BIGGER;
            }
        }
    }
    return;
}

// [TODO] return value should be void
bool Btree::append_page(void)
{
    vinfo_log("append_page");
    // one page appending
    return realloc_pages(dh_->num_nodes + 1, dh_->node_size);
}

// [TODO] return value should be void
bool Btree::alloc_pages(uint32_t num_pages, uint32_t page_size)
{
    if (ftruncate(fd_, page_size * num_pages) < 0)
    {
        throw disk_alloc_error("ftruncate failed.");
    }
    map_ = (char *) _mmap(fd_, page_size * num_pages, oflags_);
    if (map_ == NULL)
    {
        throw mmap_alloc_error("mmap failed.");
    }
    dh_ = (btree_header_t *) map_;
    ++(dh_->num_resized);
    dh_->num_alloc_pages = num_pages;
    return true;
}

// [TODO] return value should be void
bool Btree::realloc_pages(uint32_t num_pages, uint32_t page_size)
{
    if (dh_->num_alloc_pages < num_pages)
    {
        // saved in non-mapped area
        uint32_t num_alloc_pages = dh_->num_alloc_pages;
        uint32_t num_pages_extended = num_alloc_pages + BT_ALLOCATE_UNIT;

        if (map_ != NULL)
        {
            if (munmap(map_, page_size * num_alloc_pages) < 0)
            {
                error_log("munmap failed.");
                return false;
            }
            map_ = NULL;
        }

        try
        {
            alloc_pages(num_pages_extended, page_size);
        }
        catch (const disk_alloc_error &e)
        {
            throw;
        }
        catch (const mmap_alloc_error &e)
        {
            if (ftruncate(fd_, page_size * num_alloc_pages) < 0)
            {
                throw fatal_error("ftruncate the previous size failed.");
            }
            map_ = (char *) _mmap(fd_, page_size * num_alloc_pages, oflags_);
            if (map_ == NULL)
            {
                throw fatal_error("mmap the previous size failed.");
            }
            throw;
        }
    }
    dh_->num_nodes = num_pages;
    return true;
}

bool Btree::split_node(node_t *node, node_t *new_node, up_entry_t **up_entry)
{
    vinfo_log("split_node");
    // current node slots
    slot_t *slots = (slot_t *) ((char *) node->b + node->h->free_off);

    // stay_num entries stay in the node, others move to the new node
    uint16_t num_stays = node->h->num_keys / 2;
    uint16_t num_moves = node->h->num_keys - num_stays;

    // [SPEC] a node must contain at least 4 entries
    if (num_stays <= 2 || num_moves <= 2)
    {
        error_log("the number of entries in one node is too small.");
        return false;
    }

    // get a entry being set in the parent node
    *up_entry = get_up_entry(node, slots, num_moves, new_node->h->id);
    if (!node->h->is_leaf)
    {
        if (num_moves == 1)
        {
            error_log("something bad happened. usually never reaches here.");
            return false;
        }
        else
        {
            --num_moves; // the entry is pushed up in non-leaf node
        }
    }

    uint16_t off = 0;
    // needs left most pointer in non-leaf node
    if (!node->h->is_leaf)
    {
        uint16_t leftmost_off = (slots + num_moves)->off + (slots + num_moves)->size;
        make_leftmost_ptr(new_node, (char *) node->b + leftmost_off);
        off += sizeof(node_id_t);
    }

    // copy the bigger entries to the new node
    char *slot_p = (char *) new_node->b + dh_->init_data_size;
    copy_entries((char *) new_node->b, slot_p, node, slots, off, num_moves-1, 0);
    set_node_header(new_node->h, off, num_moves);
    // make a link
    new_node->h->prev_id = node->h->id;

    // copy staying entries into the buffers
    char tmp_node[dh_->node_size];
    node_t n;
    node_t *np = &n;
    np->b = (node_body_t *) (tmp_node + sizeof(node_header_t));
    off = 0;

    // needs left most pointer in non-leaf node
    if (!node->h->is_leaf)
    {
        memcpy((char *) np->b, (char *) node->b, sizeof(node_id_t));
        off += sizeof(node_id_t);
    }

    // copy entry to the data buffer
    slot_p = (char *) np->b + dh_->init_data_size;
    copy_entries((char *) np->b, slot_p, node, slots, off,
                 node->h->num_keys-1, node->h->num_keys-num_stays);

    // copy the buffers to the node and update the header
    memcpy((char *) node->b, (char *) np->b, dh_->init_data_size);
    set_node_header(node->h, off, num_stays);
    // make a link
    node->h->next_id = new_node->h->id;

    return true;
}

// make a left most pointer in non-leaf node
void Btree::make_leftmost_ptr(node_t *node, char *ptr)
{
    memcpy((char *) node->b, ptr, sizeof(node_id_t));
    node->h->data_off += sizeof(node_id_t);
    node->h->free_size -= sizeof(node_id_t);
}

void Btree::set_node_header(node_header_t *h, uint16_t off, uint16_t num_keys)
{
    h->data_off = off;
    h->free_off = dh_->init_data_size - num_keys * sizeof(slot_t);
    h->free_size = h->free_off - h->data_off;
    h->num_keys = num_keys;
}

/*
 * copy entries and slots from a specified node to a buffer specified by dp and sp.
 * offsets in slots are updated in each entry move.
 */
void Btree::copy_entries(char *dp, char *sp, node_t *node, slot_t *slots,
                         uint16_t &data_off, int slot_from, int slot_to)
{
    vinfo_log("copy_entries");
    for (int i = slot_from; i >= slot_to; --i)
    {
        char *entry_p = (char *) node->b + (slots+i)->off;
        uint32_t entry_size = (slots+i)->size; // key_size
        if (dh_->index_type == CLUSTER)
        {
            if (node->h->is_leaf)
            {
                char *val_p = entry_p + (slots+i)->size;
                entry_size += *(uint8_t *) val_p + sizeof(uint8_t); // +val_size
            }
            else
            {
                entry_size += sizeof(node_id_t);
            }
        }
        else
        {
            entry_size += sizeof(data_ptr_t); // +val_size
        }
        memcpy(dp + data_off, entry_p, entry_size);
        // new slot for the entry above
        slot_t slot = { data_off, (slots+i)->size };
        sp -= sizeof(slot_t);
        memcpy(sp, &slot, sizeof(slot_t));
        data_off += entry_size;
    }
}

// get prefix between a big key and a small key
char *Btree::get_prefix_key(char *big, char *small)
{
    size_t len = strlen(big) > strlen(small) ? strlen(small) : strlen(big);
    char *prefix = new char[len+2];
    memset(prefix, 0, len+2);

    size_t prefix_off = 0;
    for (size_t i = 0; i < len; ++i, ++prefix_off)
    {
        if (big[i] != small[i])
        {
            break;
        }
    }
    memcpy(prefix, big, prefix_off+1);
    return prefix;
}

up_entry_t *Btree::get_up_entry(node_t *node, slot_t *slots,
                                uint16_t boundary_off, node_id_t up_node_id)
{
    up_entry_t *up_entry = new up_entry_t;

    if (cmp_ == str_cmp_func && node->h->is_leaf)
    {
        slot_t *slot_r = slots + boundary_off; // right slot (smaller)
        slot_t *slot_l = slots + boundary_off - 1; // left slot (bigger)
        ALLOC_AND_COPY(key_small, (char *) node->b + slot_r->off, slot_r->size);
        ALLOC_AND_COPY(key_big, (char *) node->b + slot_l->off, slot_l->size);
        // get prefix key for prefix key compression
        up_entry->key = get_prefix_key(key_big, key_small);
        up_entry->key_size = strlen((char *) up_entry->key);
    }
    else
    {
        slot_t *slot = slots + boundary_off - 1;
        up_entry->key = new char[slot->size+1];
        memset((char *) up_entry->key, 0, slot->size+1);
        memcpy((char *) up_entry->key, (char *) node->b + slot->off, slot->size);
        up_entry->key_size = slot->size;
    }
    up_entry->val = new char[sizeof(node_id_t)];
    memcpy((char *) up_entry->val, &up_node_id, sizeof(node_id_t));
    up_entry->val_size = sizeof(node_id_t);
    up_entry->size = up_entry->key_size + up_entry->val_size;

    return up_entry;
}

void Btree::clean_up_entry(up_entry_t **up_entry)
{
    delete [] (char *) (*up_entry)->key;
    delete [] (char *) (*up_entry)->val;
    delete *up_entry;
    *up_entry = NULL;
}

bool Btree::remap(void)
{
    // saved because map_ is being unmmaped
    uint32_t num_alloc_pages = dh_->num_alloc_pages;
    if (munmap(map_, node_size_ * num_allocated_) < 0)
    {
        error_log("munmap failed.");
        return false;
    }
    map_ = (char *) _mmap(fd_, node_size_ * num_alloc_pages, oflags_);
    if (map_ == NULL)
    {
        error_log("mmap failed");
        return false;
    }
    dh_ = (btree_header_t *) map_;
    num_allocated_ = num_alloc_pages;
    num_resized_ = dh_->num_resized;

    return true;
}

bool Btree::unlock_db(void)
{
    if (lock_type_ == NO_LOCK)
    {
        return true;
    }
    else if (lock_type_ == LOCK_THREAD)
    {
        // thread level locking
#ifdef LUXIO_HAVE_LIBPTHREAD
        pthread_rwlock_unlock(&rwlock_);
#endif
    }
    else
    {
        // process level locking
        if (flock(fd_, LOCK_UN) != 0)
        {
            return false;
        }
    }
    return true;
}

bool Btree::rlock_db(void)
{
    if (lock_type_ == NO_LOCK)
    {
        return true;
    }
    else if (lock_type_ == LOCK_THREAD)
    {
        // thread level locking
#ifdef LUXIO_HAVE_LIBPTHREAD
        pthread_rwlock_rdlock(&rwlock_);
#endif
    }
    else
    {
        // process level locking
        if (flock(fd_, LOCK_SH) != 0)
        {
            error_log("flock failed.");
            return false;
        }
        if (num_resized_ != dh_->num_resized)
        {
            if (!remap())
            {
                return false;
            }
        }
    }
    return true;
}

bool Btree::wlock_db(void)
{
    if (lock_type_ == NO_LOCK)
    {
        return true;
    }
    else if (lock_type_ == LOCK_THREAD)
    {
        // thread level locking
#ifdef LUXIO_HAVE_LIBPTHREAD
        pthread_rwlock_wrlock(&rwlock_);
#endif
    }
    else
    {
        // process level locking
        if (flock(fd_, LOCK_EX) != 0)
        {
            error_log("flock failed.");
            return false;
        }
        if (num_resized_ != dh_->num_resized)
        {
            if (!remap())
            {
                return false;
            }
        }
    }
    return true;
}

bool Btree::check_key(uint32_t key_size)
{
    if (key_size > MAX_KSIZE || key_size <= 0)
    {
        error_log("key size is too big or too small");
        return false;
    }
    return true;
}

bool Btree::check_val(uint32_t val_size)
{
    if (val_size <= 0)
    {
        error_log("value size is too small");
        return false;
    }
    if (dh_->index_type == CLUSTER)
    {
        if (val_size > CLUSTER_MAX_VSIZE)
        {
            error_log("value size is too big");
            return false;
        }
    }
    else
    {
        if (val_size > NONCLUSTER_MAX_VSIZE)
        {
            error_log("value size is too big");
            return false;
        }
    }
    return true;
}

bool Btree::cursor_find(cursor_t *c, node_id_t id,
                        data_t *key, op_mode_t op_mode)
{
    bool res = true;
    entry_t entry;
    if (key != NULL)
    {
        entry.key = key->data;
        entry.key_size = key->size;
        entry.val = NULL;
        entry.val_size = 0;
        entry.size = 0;
    }

    node_t *node = _alloc_node(id);
    if (node->h->is_leaf)
    {
        if (node->h->num_keys == 0)
        {
            res = false;
        }
        else
        {
            c->node_id = id;
            if (op_mode == OP_CUR_FIRST)
            {
                c->slot_index = node->h->num_keys - 1;
            }
            else if (op_mode == OP_CUR_LAST)
            {
                c->slot_index = 0;
            }
            else if (op_mode == OP_CUR_GET)
            {
                find_res_t r;
                find_key(node, key->data, key->size, &r);
                if (r.type == KEY_FOUND)
                {
                    char *p = (char *) node->b + node->h->free_off;
                    c->slot_index = (r.slot_p - p) / sizeof(slot_t);
                }
            }
            else if (op_mode == OP_CUR_LBOUND)
            {
                find_res_t r;
                find_key(node, key->data, key->size, &r);
                if (r.type == KEY_FOUND || r.type == KEY_SMALLEST)
                {
                    char *p = (char *) node->b + node->h->free_off;
                    c->slot_index = (r.slot_p - p) / sizeof(slot_t);
                }
                else if (r.type == KEY_BIGGER)
                {
                    char *p = (char *) node->b + node->h->free_off;
                    if (r.slot_p - p - 1 >= 0)
                    {
                        c->slot_index = (r.slot_p - p - 1) / sizeof(slot_t);
                    }
                    else
                    {
                        res = false;
                    }
                }
                else
                {
                    res = false;
                }
            }
            else
            {
                error_log("operation not supported");
                res = false;
            }
        }
    }
    else
    {
        node_id_t next_id = _find_next(node, &entry, op_mode);
        res = cursor_find(c, next_id, key, op_mode);
    }
    delete node;
    return res;
}

}
}
