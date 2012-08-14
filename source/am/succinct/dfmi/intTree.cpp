/***************************************************************************
 *   DynSA - Dynamic Suffix Array                                          *
 *   Copyright (C) 2006  Wolfgang Gerlach                                  *
 *                 2008  Mikael Salson                                     *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <am/succinct/dfmi/intTree.h>


namespace dfmi
{

void IntTree::increaseElement(size_t i, size_t inc)
{
    CustomNode<uint64_t> *n = getIthNode(i);
    if (n)
    {
        n->element += inc;
    }
}

void IntTree::decreaseElement(size_t i, size_t dec)
{
    CustomNode<uint64_t> *n = getIthNode(i);
    if (n)
    {
        n->element -= dec;
    }
}

}
