/***************************************************************************
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

// ------ Dynamic Suffix Array --- Dynamic FM-Index -----
//  Storage of a Dynamic Permutation using two trees.
// paper: M. Salson and T. Lecroq and M. L\'{e}onard and L. Mouchard
//       Dynamic Extended Suffix Arrays, JDA, 2008

#include <am/succinct/dfmi/lpermutation.h>
#include <am/succinct/dfmi/linkedTree.h>


namespace dfmi
{

LPermutation::LPermutation()
{
    perm = new LinkedTree();
    invperm = new LinkedTree();
    length = 0;
}

LPermutation::LPermutation(uint64_t *pi, uint64_t size)
{
    perm = new LinkedTree();
    invperm = new LinkedTree();
    for (uint64_t i = 0; i < size; ++i)
    {
        insert(pi[i], i + 1);
    }
    length = size;
}

LPermutation::~LPermutation()
{
    delete perm;
    delete invperm;
}

/***   Requests ***/

uint64_t LPermutation::getIthElement(uint64_t i)
{
    LinkedNode *node = (LinkedNode *)perm->getIthNode(i);
    LinkedNode *invNode = (LinkedNode *)node->element;
    return invperm->getNumNode(invNode);
}

uint64_t LPermutation::getIthInvElement(uint64_t i)
{
    LinkedNode *invNode = (LinkedNode *)invperm->getIthNode(i);
    LinkedNode *node = (LinkedNode *)invNode->element;
    return perm->getNumNode(node);
}

uint64_t LPermutation::getSize()
{
    return length;
}

/*** Commands ***/

int LPermutation::deleteElem(uint64_t i)
{
    LinkedNode *invNode = (LinkedNode *)invperm->getIthNode(i);
    LinkedNode *node = (LinkedNode *)invNode->element;
    invperm->deleteNode(invNode);
    perm->deleteNode(node);
    --length;
    return 0;
}

int LPermutation::insert(uint64_t i, uint64_t pos)
{
    perm->insertElement(NULL, pos);
    LinkedNode *node = (LinkedNode *)perm->getInsertedNode();
    invperm->insertElement(node, i);
    node->element = invperm->getInsertedNode();
    ++length;
    return 0;
}

int LPermutation::update(uint64_t i, uint64_t j)
{
    LinkedNode *invNode = (LinkedNode *)invperm->getIthNode(i);
    LinkedNode *node = (LinkedNode *)invNode->element;
    invperm->deleteNode(invNode);
    invperm->insertElement(node, j);
    node->element = invperm->getInsertedNode();
    return 0;
}

}
