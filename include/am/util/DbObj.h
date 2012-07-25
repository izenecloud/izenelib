/**
 * @file DBObj.h
 * @brief The header file of DbObj.
 *
 * This file defines class DbObj, which provide uniform format for each item in the file.
 */

#ifndef DBOBJ_H_
#define DBOBJ_H_

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <boost/intrusive_ptr.hpp>

#include <types.h>
#include <util/RefCount.h>
#include <string.h>

NS_IZENELIB_AM_BEGIN

// Convenience definition
#ifndef byte
typedef unsigned char byte;
#endif

namespace util{
/**
 * \brief It represents the sort of objects we deal with in the BtreeFile. This is analogous to
 * a DBT in the Berkeley DB package.
 *
 */
class DbObj : public izenelib::util::RefCount<izenelib::util::ReadWriteLock>
//boost::enable_shared_from_this<DbObj>
{
public:
    /**
     * \brief Default constructor
     */
    DbObj(void)
    {
        _data = 0;
        _size = 0;
    }

    /**
     *  \brief Constructor taking a pointer and a size. Make a copy of the _data.
     */
    DbObj(void* pd, size_t sz)
    {
        _size = sz;
        if (_size)
        {
            _data = new byte[_size];
            memcpy(_data, pd, _size);
        }
    }

    /**
     *  \brief Constructor taking a std::string reference.
     */
    DbObj(const std::string& s)
    {
        _size = s.length();
        _data = new byte[_size];
        memcpy(_data, s.c_str(), _size);
    }

    /**
     *  \brief Constructor taking a (possibly) null terminated string.
     */
    DbObj(const char* ps, size_t sz = 0)
    {
        _size = (0 == sz) ? strlen(ps) : sz;
        _data = new byte[_size];
        memcpy(_data, ps, _size);
    }

    /**
     *  \brief Constructor taking a 32-bit unsigned int
     */
    DbObj(unsigned long ul)
    {
        _size = sizeof(unsigned long);
        _data = new byte[_size];
        *((unsigned long*)_data) = ul;
    }

    /**
     *  \brief Constructor taking a 32-bit int
     */
    DbObj(long l)
    {
        _size = sizeof(long);
        _data = new byte[_size];
        *((long*)_data) = l;
    }

    /**
     * \brief Constructor taking a 16-bit unsigned int
     */
    DbObj(unsigned short us)
    {
        _size = sizeof(unsigned short);
        _data = new byte[_size];
        *((unsigned short*)_data) = us;
    }

    /**
     *  \brief Constructor taking a 16-bit int
     */
    DbObj(short s)
    {
        _size = sizeof(short);
        _data = new byte[_size];
        *((short*)_data) = s;
    }

    /**
     *  \brief Copy constructor. Call the assignment operator.
     */
    DbObj(DbObj& obj)
    {
        _data = 0;
        _size = 0;
        operator=(obj);
    }

    /**
     * \brief Destructor. Delete the pointer if the size is non-zero.
     */
    ~DbObj()
    {
        if (_size != 0)
        {
            delete[] _data;
            _data = 0;
            _size = 0;
        }
    }

    /**
     *  \brief Assignment operator. Make a copy of the other object.
     */
    DbObj& operator=(DbObj& obj)
    {
        if (_size != 0)
        {
            delete[] _data;
        }
        if (obj._size > 0)
        {
            _size = obj._size;
            _data = new byte[_size];
            memcpy(_data, obj._data, _size);
        }
        return *this;
    }

    bool operator <=(DbObj& obj)
    {
        return memcmp(_data, obj.getData(), std::min(_size, obj.getSize()));

    }

public:

    const void* getData() const {
        return _data;
    }
    size_t getSize() const {
        return _size;
    }
    /*
     *     we can set any type of data. By this, we can deal with different KeyType and DataType.
     *
     */

    void setData(const void* pd, size_t sz)
    {
        if (_size)
        {
            delete[] _data;
        }
        _size = sz;
        _data = new byte[_size];
        memcpy(_data, pd, _size);
    }

    /*
     *     for debug.
     */

    void display()
    {
        std::cout << "\nDbObj display..\n";
        std::cout << "_size " << _size << std::endl;
        std::cout << "_data " << (char*)_data << std::endl;
    }


private:
    byte* _data;
    size_t _size;
};

typedef boost::intrusive_ptr<DbObj> DbObjPtr;
typedef std::vector<DbObjPtr> DBOBJVECTOR;
typedef std::list<DbObjPtr> DBOBJLIST;

}//end of namespace util

NS_IZENELIB_AM_END

#endif /*DBOBJ_H_*/
