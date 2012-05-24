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

#ifndef LUX_IO_TYPES_H
#define LUX_IO_TYPES_H

namespace Lux
{
namespace IO
{

typedef int db_flags_t;
const static db_flags_t DB_RDONLY = 0x0000;
const static db_flags_t DB_RDWR = 0x0002;
const static db_flags_t DB_CREAT = 0x0200;
const static db_flags_t DB_TRUNC = 0x0400;

}
}

#endif
