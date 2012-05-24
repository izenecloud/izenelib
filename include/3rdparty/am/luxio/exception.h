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

#ifndef LUX_EXCEPTION_H
#define LUX_EXCEPTION_H

#include <string>
#include <exception>

namespace Lux
{

class luxio_error : public std::exception
{
public:
    explicit luxio_error(const std::string &msg)
        : std::exception(), msg_(msg)
    {}
    virtual ~luxio_error() throw ()
    {}
    virtual const char * what() const throw()
    {
        return msg_.c_str();
    }
private:
    std::string msg_;
};

class mmap_alloc_error : public luxio_error
{
public:
    explicit mmap_alloc_error(const std::string &msg)
        : luxio_error("[mmap_alloc_error] " + msg)
    {}
    virtual ~mmap_alloc_error() throw ()
    {}
};

class disk_alloc_error : public luxio_error
{
public:
    explicit disk_alloc_error(const std::string &msg)
        : luxio_error("[disk_alloc_error] " + msg)
    {}
    virtual ~disk_alloc_error() throw ()
    {}
};

class fatal_error : public luxio_error
{
public:
    explicit fatal_error(const std::string &msg)
        : luxio_error("[fatal_error] " + msg)
    {}
    virtual ~fatal_error() throw ()
    {}
};

}

#endif
