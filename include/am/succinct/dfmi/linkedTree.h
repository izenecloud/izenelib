/***************************************************************************
 *   DynSA - Dynamic Suffix Array                                          *
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

/*
 * This class defined a tree whose nodes contain pointers
 */

#ifndef DFMI_LINKED_TREE_H
#define DFMI_LINKED_TREE_H

#include "customTree.h"


namespace dfmi
{

class LinkedNode: public CustomNode<void *> {};
class LinkedTree: public CustomTree<void *> {};

}

#endif
