#ifndef AM_LUXIO_RAW_BTREE_H
#define AM_LUXIO_RAW_BTREE_H
/**
 * @file am/luxio/raw/BTree.h
 * @author Yingfeng Zhang
 * @date Created <2011-11-08 10:18:25>
 */

#include <am/concept/DataType.h>
#include <am/raw/Buffer.h>
#include <am/tc/String.h>

#include <3rdparty/am/luxio/btree.h>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace izenelib {
namespace am {
namespace luxio {
namespace raw {

using izenelib::am::raw::Buffer;

class BTree
    : boost::noncopyable
{
public:
    typedef Buffer key_type;
    typedef Buffer value_type;
    typedef DataType<Buffer, Buffer> data_type;
    typedef int size_type;
    typedef boost::shared_ptr<Lux::IO::cursor_t> cursor_type;

    enum IndexType{
        NON_CLUSTER_LINKING,
        NON_CLUSTER_PADDING,
        CLUSTER,
    };

    explicit BTree(const std::string& file = "")
    : bdb_(new Lux::IO::Btree(Lux::IO::NONCLUSTER))
    , isOpened_(false)
    , file_(file)
    {
        set_index_type(NON_CLUSTER_LINKING);
    }

    ~BTree()
    {
        close();
        if (bdb_)
        {
            delete bdb_;
        }
    }

    void set_index_type(IndexType type)
    {
        switch(type)
        {
        case NON_CLUSTER_LINKING:
            {
            bdb_->set_index_type(Lux::IO::NONCLUSTER);
            bdb_->set_noncluster_params(Lux::IO::Linked);
            }
            break;
        case NON_CLUSTER_PADDING:
            {
            bdb_->set_index_type(Lux::IO::NONCLUSTER);
            bdb_->set_noncluster_params(Lux::IO::Padded);
            }
            break;
        case CLUSTER:
            bdb_->set_index_type(Lux::IO::CLUSTER);
            break;
        default:
            break;
        }
    }

    void setcmpfunc(Lux::IO::CMP cmp)
    {
        bdb_->set_cmp_func(cmp);
    }
    //@}

    bool open(const std::string& file)
    {
        file_ = file;
        return open();
    }

    bool open()
    {
        close(); // close first if opened

        if (!file_.empty())
        {
            if ( !boost::filesystem::exists(file_) )
            {
                bdb_->open(file_.c_str(), Lux::IO::DB_CREAT);
            }
            else
            {
                bdb_->open(file_.c_str(), Lux::IO::DB_RDWR);
            }
            isOpened_ = true;
        }

        return isOpened_;
    }

    std::string getFileName() const
    {
        return file_;
    }
    void close()
    {
        if (bdb_ && isOpened_)
        {
            flush();
            bdb_->close();
            isOpened_ = false;
        }
    }

    bool isOpened() const
    {
        return isOpened_;
    }
    /**
     * @deprecated
     */
    bool is_open() const
    {
        return isOpened_;
    }

    bool flush()
    {
        return isOpened();
    }

    size_type size() const
    {
        return bdb_ ? bdb_->size() : 0;
    }
    /**
     * @deprecated
     */
    size_type num_items() const
    {
        return size();
    }
    bool empty() const
    {
        return size() == 0;
    }
    bool clear()
    {
        if(isOpened())
        {
            close();
            boost::filesystem::remove_all(file_);
            open();
        }
        return false;
    }

    /**
     * @brief Insert new data into database.
     *
     * If a record with the same key exists in the database, this function has no effect.
     *
     * @param key Key Buffer
     * @param value Value Buffer
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool insert(const Buffer& key, const Buffer& value)
    {
        return isOpened()&& bdb_->put(
                   key.data(),key.size(), value.data(),value.size(),Lux::IO::APPEND);
    }
    /**
     * @brief Insert new data into database.
     *
     * If a record with the same key exists in the database, this function has no effect.
     *
     * @param data data record
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool insert(const data_type& data)
    {
        return insert(data.get_key(), data.get_value());
    }

    /**
     * @brief Insert new data into database or update the existing record.
     *
     * @param key Key Buffer
     * @param value Value Buffer
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool update(const Buffer& key, const Buffer& value)
    {
        return isOpened() && bdb_->put(
            key.data(), key.size(),
            value.data(), value.size(),
            Lux::IO::OVERWRITE
        );
    }
    /**
     * @brief Insert new data into database or update the existing record.
     *
     * @param data data record
     *
     * @return If successful, the return value is @c true, else, it is @c false.
     */
    bool update(const data_type& data)
    {
        return update(data.get_key(), data.get_value());
    }

    bool get(const Buffer& key, Buffer& value) const
    {
        Lux::IO::data_t *val_p = NULL;
        if (isOpened())
        {
            val_p = bdb_->get(key.data(), key.size());
            if(val_p)
            {
                value.attach(static_cast<char*>(const_cast<void*>(val_p->data)),
                                    static_cast<std::size_t>(val_p->size),
                                    &::free);
                delete val_p;
                return true;
            }
        }

        return false;
    }

    bool del(const Buffer& key)
    {
        return isOpened() &&
            bdb_->del(key.data(), key.size());
    }

    bool append(const Buffer& key, const Buffer& value)
    {
        return isOpened() && bdb_->put(
                   key.data(),key.size(), value.data(),value.size(),Lux::IO::APPEND);
    }

    //@{
    //@brief iteration
    cursor_type begin() const
    {
        if(isOpened())
        {
            cursor_type ans(bdb_->cursor_init());
            bool result = bdb_->first(ans.get());
            if(result) return ans;
        }
        return cursor_type();
    }

    cursor_type begin(Buffer& key) const
    {
        if(isOpened())
        {
            cursor_type ans(bdb_->cursor_init());
            Lux::IO::data_t k = {key.data(), (uint32_t)key.size()};

            //The cursor is set to the first record corresponding the key
            //or the next substitute if completely matching record does not exist.
            bool result = bdb_->lower_bound(ans.get(), &k);
            if(result) return ans;
        }
        return cursor_type();
    }

    cursor_type rbegin() const
    {
        if(isOpened())
        {
            cursor_type ans(bdb_->cursor_init());
            bool result = bdb_->last(ans.get());
            if(result) return ans;
        }
        return cursor_type();
    }

    bool fetch(cursor_type& cursor, Buffer& key, Buffer& value)
    {
        if(isOpened() && cursor.get())
        {
            Lux::IO::data_t* k;
            Lux::IO::data_t* v;
            if(bdb_->cursor_get(cursor.get(), &k, &v, Lux::IO::SYSTEM))
            {
                key.attach(static_cast<char*>(const_cast<void*>(k->data)),
                                    static_cast<std::size_t>(k->size),
                                    &::free);
                value.attach(static_cast<char*>(const_cast<void*>(v->data)),
                                    static_cast<std::size_t>(v->size),
                                    &::free);
                delete k;
                delete v;
                return true;
            }
        }
        return false;
    }

    bool iterNext(cursor_type& cursor)
    {
        bool ret = bdb_->next(cursor.get());
        if(!ret) cursor.reset();
        return ret;
    }

    bool iterPrev(cursor_type& cursor)
    {
        bool ret = bdb_->prev(cursor.get());
        if(!ret) cursor.reset();
        return ret;
    }

private:
    Lux::IO::Btree* bdb_;
    bool isOpened_;
    std::string file_;
};

}}}} // namespace izenelib::am::luxio::raw

#endif // AM_LUXIO_RAW_BTREE_H
