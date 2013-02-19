/*
 * Copyright (C) 2008 Hiroyuki Yamada
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

#ifndef LUX_IO_ARRAY_H
#define LUX_IO_ARRAY_H

#include "dbm.h"

namespace Lux
{
namespace IO
{

class Data;
const int DEFAULT_PAGESIZE = getpagesize();
static const uint32_t ARY_ALLOCATE_UNIT = 100;

// global header
typedef struct
{
    char magic[8];
    uint32_t num_keys;
    uint32_t num_pages;
    uint16_t page_size;
    uint8_t index_type;
    uint8_t data_size; // for fixed length value in cluster index
    uint32_t num_resized;
} array_header_t;
/*
 * Class Array
 */
class Array
{
public:
    Array(db_index_t index_type = CLUSTER,
          uint8_t data_size = sizeof(uint32_t));
    Array(std::string db_name, db_index_t index_type = Lux::IO::NONCLUSTER,
          uint8_t data_size = sizeof(uint32_t));

    ~Array(void);
    bool open(std::string db_name, db_flags_t oflags);
    bool close(void);
    data_t *get(uint32_t index);
    bool get(uint32_t index, data_t **data, alloc_type_t atype = USER);
    bool put(uint32_t index,
             const void *val, uint32_t val_size,
             insert_mode_t flags = OVERWRITE);
    bool put(uint32_t index, data_t *data, insert_mode_t flags = OVERWRITE);
    bool del(uint32_t index);
    void set_index_type(db_index_t index_type, uint8_t data_size);
    void set_lock_type(lock_type_t lock_type);
    void set_noncluster_params(store_mode_t smode,
                               padding_mode_t pmode = RATIO,
                               uint32_t padding = 20);
    static void clean_data(data_t *d);
    void show_db_header(void);
    size_t num_items(void);

    /// add interface for AutofillIDManager;
    void flush()
    {}
    void display()
    {}

    bool put(const uint32_t& NameID, const std::string& NameString)
    {
        uint32_t len = NameString.length();
        char* str = new char[len];
        for (unsigned int i = 0; i < len ; i++)
            str[i] = NameString[i];
        return put(NameID, (void*)str, len, Lux::IO::OVERWRITE);
    }

    bool get(const unsigned int& NameID, std::string& NameString)
    {
        data_t* Namedata = get(NameID);
        if (Namedata == NULL)
            return false;
        std::string namestr((char*)Namedata->data, Namedata->size);
        NameString = namestr;
        return true;
    }

private:
    int fd_;
    db_flags_t oflags_;
    char *map_;
    Data *dt_;
    store_mode_t smode_;
    padding_mode_t pmode_;
    uint32_t padding_;
    lock_type_t lock_type_;
    db_index_t index_type_;
    uint8_t data_size_;
    uint64_t allocated_size_;
    array_header_t *dh_;
#ifdef LUXIO_HAVE_LIBPTHREAD
    pthread_rwlock_t rwlock_;
#endif
    uint32_t num_pages_;
    uint16_t page_size_;
    uint32_t num_resized_;

    bool open_(std::string db_name, db_flags_t oflags);
    bool alloc_pages(uint32_t num_pages, uint16_t page_size);
    bool realloc_pages(uint32_t num_pages, uint16_t page_size);
    bool remap(void);
    bool unlock_db(void);
    bool rlock_db(void);
    bool wlock_db(void);
};

}
}

#endif
