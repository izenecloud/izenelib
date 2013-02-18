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
#include <array.h>
#include <util.h>
#include <data.h>
#include <boost/filesystem.hpp>
#ifdef LUXIO_HAVE_LIBPTHREAD
#include <pthread.h>
#endif

namespace Lux
{
namespace IO
{

static const char *ARYMAGIC = "LUXAR001";

Array::Array(db_index_t index_type, uint8_t data_size)
    : map_(NULL),
      dt_(NULL),
      smode_(Padded),
      pmode_(RATIO),
      padding_(20),
      lock_type_(NO_LOCK),
      index_type_(index_type),
      data_size_(index_type == NONCLUSTER ? sizeof(data_ptr_t) : data_size)
{
#ifdef LUXIO_HAVE_LIBPTHREAD
    if (pthread_rwlock_init(&rwlock_, NULL) != 0)
    {
        error_log("pthread_rwlock_init failed.");
        exit(-1);
    }
#endif
}
Array::Array(std::string db_name, db_index_t index_type, uint8_t data_size)
    : map_(NULL),
      dt_(NULL),
      smode_(Padded),
      pmode_(RATIO),
      padding_(20),
      lock_type_(NO_LOCK),
      index_type_(index_type),
      data_size_(index_type == NONCLUSTER ? sizeof(data_ptr_t) : data_size)
{
#ifdef LUXIO_HAVE_LIBPTHREAD
    if (pthread_rwlock_init(&rwlock_, NULL) != 0)
    {
        error_log("pthread_rwlock_init failed.");
        exit(-1);
    }
#endif
    set_noncluster_params(Lux::IO::Linked);
    set_lock_type(Lux::IO::LOCK_THREAD);
    //open(db_name.c_str(), Lux::IO::DB_CREAT);
    if ( !boost::filesystem::exists(db_name+"_Autofill_") )
    {
        open((db_name+"_Autofill_").c_str(), Lux::IO::DB_CREAT);
    }
    else
    {
        open((db_name+"_Autofill_").c_str(), Lux::IO::DB_RDWR);
    }
}

Array::~Array(void)
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
        error_log("pthread_rwlock_destroy failed.");
        exit(-1);
    }
#endif
}

bool Array::open(std::string db_name, db_flags_t oflags)
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

bool Array::close(void)
{
    if (!wlock_db())
    {
        return false;
    }
    if (map_ != NULL)
    {
        if (msync(map_, dh_->page_size * dh_->num_pages, MS_SYNC) < 0)
        {
            error_log("msync failed.");
            unlock_db();
            return false;
        }
        if (munmap(map_, dh_->page_size * dh_->num_pages) < 0)
        {
            error_log("munmap failed.");
            unlock_db();
            return false;
        }
        map_ = NULL;
    }
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

data_t *Array::get(uint32_t index)
{
    data_t *data;

    if (!rlock_db())
    {
        return NULL;
    }
    size_t off = index * dh_->data_size + dh_->page_size;
    assert(off + dh_->data_size <= allocated_size_);
    if (off + dh_->data_size > allocated_size_)
    {
        unlock_db();
        return NULL;
    }

    if (dh_->index_type == CLUSTER)
    {
        data = new data_t;
        data->size = dh_->data_size;
        data->data = new char[dh_->data_size + 1];
        ((char *) data->data)[dh_->data_size] = '\0';
        memcpy((char *) data->data, map_ + off, dh_->data_size);
    }
    else
    {
        data_ptr_t *data_ptr = (data_ptr_t *) (map_ + off);
        data = dt_->get(data_ptr);
    }
    if (!unlock_db())
    {
        return NULL;
    }
    return data;
}

bool Array::get(uint32_t index, data_t **data, alloc_type_t atype)
{
    if (!rlock_db())
    {
        return false;
    }
    size_t off = index * dh_->data_size + dh_->page_size;
    assert(off + dh_->data_size <= allocated_size_);
    if (off + dh_->data_size > allocated_size_)
    {
        unlock_db();
        return false;
    }

    if (dh_->index_type == CLUSTER)
    {
        if (atype == SYSTEM)
        {
            *data = new data_t;
            (*data)->data = (char *) new char[dh_->data_size + 1];
            ((char *) (*data)->data)[dh_->data_size] = '\0';
        }
        else
        {
            if ((*data)->user_alloc_size < dh_->data_size)
            {
                error_log("allocated size is too small for the data.");
                unlock_db();
                return false;
            }
            if ((*data)->user_alloc_size >= (size_t)(dh_->data_size + 1))
            {
                ((char *) (*data)->data)[dh_->data_size] = '\0';
            }
        }
        memcpy((char *) (*data)->data, map_ + off, dh_->data_size);
        (*data)->size = dh_->data_size;
    }
    else
    {
        data_ptr_t *data_ptr = (data_ptr_t *) (map_ + off);
        if (!dt_->get(data_ptr, data, atype))
        {
            unlock_db();
            return false;
        }
    }
    if (!unlock_db())
    {
        return false;
    }
    return true;
}

bool Array::put(uint32_t index,
                const void *val, uint32_t val_size, insert_mode_t flags)
{
    data_t data = {val, val_size};
    return put(index, &data, flags);
}

bool Array::put(uint32_t index, data_t *data, insert_mode_t flags)
{
    if (!wlock_db())
    {
        return false;
    }
    size_t off = index * dh_->data_size + dh_->page_size;
    if (off + dh_->data_size > allocated_size_)
    {
        div_t d = div(off + dh_->data_size, dh_->page_size);
        uint32_t page_num = d.rem > 0 ? d.quot + 1 : d.quot;
        if (!realloc_pages(page_num, dh_->page_size))
        {
            unlock_db();
            return false;
        }
    }

    if (dh_->index_type == CLUSTER)
    {
        // only update is supported in cluster index
        memcpy(map_ + off, data->data, dh_->data_size);
    }
    else
    {
        data_ptr_t *res_data_ptr;
        data_ptr_t *data_ptr = (data_ptr_t *) (map_ + off);

        if (data_ptr->id != 0 || data_ptr->off != 0)   // already stored
        {
            if (flags == APPEND)
            {
                res_data_ptr = dt_->append(data_ptr, data);
            }
            else     // OVERWRITE
            {
                res_data_ptr = dt_->update(data_ptr, data);
            }
        }
        else
        {
            res_data_ptr = dt_->put(data);
        }
        if (res_data_ptr == NULL)
        {
            unlock_db();
            return false;
        }
        memcpy(map_ + off, res_data_ptr, sizeof(data_ptr_t));
        dt_->clean_data_ptr(res_data_ptr);
    }
    if (!unlock_db())
    {
        return false;
    }
    return true;
}

bool Array::del(uint32_t index)
{
    if (!wlock_db())
    {
        return false;
    }
    size_t off = index * dh_->data_size + dh_->page_size;
    assert(off + dh_->data_size <= allocated_size_);
    if (off + dh_->data_size > allocated_size_)
    {
        unlock_db();
        return false;
    }

    if (dh_->index_type == NONCLUSTER)
    {
        data_ptr_t *data_ptr = (data_ptr_t *) (map_ + off);
        if (!dt_->del(data_ptr))
        {
            unlock_db();
            return false;
        }
    }
    // [NOTICE] deleting only fills zero
    memset(map_ + off, 0, dh_->data_size);

    if (!unlock_db())
    {
        return false;
    }
    return true;
}

void Array::set_index_type(db_index_t index_type, uint8_t data_size)
{
    if (index_type != CLUSTER && index_type != NONCLUSTER)
    {
        return;
    }
    index_type_ = index_type;
    if (index_type == CLUSTER)
    {
        data_size_ = data_size;
    }
    else
    {
        data_size_ = sizeof(data_ptr_t);
    }
}

void Array::set_lock_type(lock_type_t lock_type)
{
    lock_type_ = lock_type;
}

// only for noncluster database
void Array::set_noncluster_params(store_mode_t smode,
                                  padding_mode_t pmode,
                                  uint32_t padding)
{
    smode_ = smode;
    pmode_ = pmode;
    padding_ = padding;
}

void Array::clean_data(data_t *d)
{
    if (d != NULL)
    {
        delete [] (char *) (d->data);
        delete d;
        d = NULL;
    }
}

void Array::show_db_header()
{
    std::cout << "========= SHOW DATABASE HEADER ==========" << std::endl;
    std::cout << "num_keys: " << dh_->num_keys << std::endl;
    std::cout << "num_nodes: " << dh_->num_pages << std::endl;
    std::cout << "node_size: " << dh_->page_size << std::endl;
    std::cout << "index_type: " << (int) dh_->index_type << std::endl;
    std::cout << "data_size: " << (int) dh_->data_size << std::endl;
}

size_t Array::num_items()
{
    return dh_->num_keys;
}

bool Array::open_(std::string db_name, db_flags_t oflags)
{
    std::string idx_db_name = db_name + ".aidx";
    fd_ = _open(idx_db_name.c_str(), oflags, 00644);
    if (fd_ < 0)
    {
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
        return false;
    }

    array_header_t dh;
    memset(&dh, 0, sizeof(array_header_t));
    if (stat_buf.st_size == 0 && oflags & DB_CREAT)
    {
        memcpy(dh.magic, ARYMAGIC, strlen(ARYMAGIC));
        dh.num_keys = 0;
        // one for db_header
        dh.num_pages = ARY_ALLOCATE_UNIT;
        dh.page_size = getpagesize();
        dh.num_resized = 0;
        dh.index_type = index_type_;
        dh.data_size = data_size_;

        if (_write(fd_, &dh, sizeof(array_header_t)) < 0)
        {
            return false;
        }
        if (!alloc_pages(dh.num_pages, dh.page_size))
        {
            error_log("alloc_page failed.");
        }

    }
    else
    {
        if (_read(fd_, &dh, sizeof(array_header_t)) < 0)
        {
            error_log("read failed");
            return false;
        }

        // [TODO] read filesize and compare with num_nodes * node_size
        // if they differ, gives alert and trust the filesize ?
        map_ = (char *) _mmap(fd_, dh.page_size * dh.num_pages, oflags);
        if (map_ == NULL)
        {
            return false;
        }
    }

    dh_ = (array_header_t *) map_;
    allocated_size_ = dh_->page_size * dh_->num_pages;
    num_pages_ = dh_->num_pages;
    page_size_ = dh_->page_size;
    num_resized_ = dh_->num_resized;

    if (index_type_ != dh_->index_type)
    {
        error_log("wrong index type");
        return false;
    }

    if (dh_->index_type == NONCLUSTER)
    {
        if(smode_ == Padded)
        {
            dt_ = new PaddedData(pmode_, padding_);
        }
        else
        {
            dt_ = new LinkedData(pmode_, padding_);
        }
        std::string data_db_name = db_name + ".data";
        dt_->open(data_db_name.c_str(), oflags);
    }

    if (lock_type_ == LOCK_PROCESS)
    {
        if (flock(fd_, LOCK_UN) != 0)
        {
            return false;
        }
    }

    return true;
}

bool Array::alloc_pages(uint32_t num_pages, uint16_t page_size)
{
    allocated_size_ = page_size * num_pages;
    if (ftruncate(fd_, allocated_size_) < 0)
    {
        error_log("ftruncate failed.");
        return false;
    }
    map_ = (char *) _mmap(fd_, allocated_size_, oflags_);
    if (map_ == NULL)
    {
        error_log("mmap failed.");
        return false;
    }
    dh_ = (array_header_t *) map_;
    dh_->num_pages = num_pages;
    num_pages_ = num_pages;

    return true;
}

bool Array::realloc_pages(uint32_t num_pages, uint16_t page_size)
{
    uint32_t prev_num_pages = dh_->num_pages;

    if (map_ != NULL)
    {
        if (munmap(map_, dh_->page_size * dh_->num_pages) < 0)
        {
            error_log("munmap failed in realloc_pages");
            return false;
        }
    }

    // large size allocation for fragmentation in both disk and memory.
    if (prev_num_pages + ARY_ALLOCATE_UNIT > num_pages)
    {
        num_pages = prev_num_pages + ARY_ALLOCATE_UNIT;
    }

    if (!alloc_pages(num_pages, page_size))
    {
        return false;
    }
    ++(dh_->num_resized);

    // fill zero in the newly allocated pages
    memset(map_ + dh_->page_size * prev_num_pages, 0,
           dh_->page_size * (num_pages - prev_num_pages));

    return true;
}

bool Array::remap(void)
{
    uint32_t num_pages = dh_->num_pages;
    if (munmap(map_, page_size_ * num_pages_) < 0)
    {
        error_log("munmap failed.");
        return false;
    }
    map_ = (char *) _mmap(fd_, page_size_ * num_pages, oflags_);
    if (map_ == NULL)
    {
        error_log("mmap failed.");
        return false;
    }
    dh_ = (array_header_t *) map_;
    num_pages_ = dh_->num_pages;
    num_resized_ = dh_->num_resized;

    return true;
}

bool Array::unlock_db(void)
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

bool Array::rlock_db(void)
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

bool Array::wlock_db(void)
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

}
}
