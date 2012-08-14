/***************************************************************************
 *   DynFMI - Dynamic FM-Index for a Collection of Texts                   *
 *   Copyright (C) 2006  Wolfgang Gerlach                                  *
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


// This is a red-black tree implementation by Wolfgang Gerlach based on the algorithm provided by
// Cormen et al.: Introduction to Algorithms, Second Edition. MIT Press and McGraw-Hill, 2001

#ifndef DFMI_RB_TREE_H
#define DFMI_RB_TREE_H

#include <types.h>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dfmi
{

// generic Red-Black Tree Node:
class RBNode
{
public:
    enum RBNodecolor { BLACK, RED };

    RBNode() {}

    RBNode(RBNode *n)
        : parent(n), left(n), right(n)
    {
        color = RED;
    }

    virtual ~RBNode() {} //adds 4 bytes vtable

    RBNode *getParent()
    {
        return parent;
    }

    RBNode *getLeft()
    {
        return left;
    }

    RBNode *getRight()
    {
        return right;
    }

    void setParent(RBNode *n)
    {
        parent = n;
    }

    void setLeft(RBNode *n)
    {
        left = n;
    }

    void setRight(RBNode *n)
    {
        right = n;
    }

public:
    RBNode *parent;
    RBNode *left;
    RBNode *right;   // 3*4 bytes

    enum RBNodecolor color; // due to structure alignment: 4 bytes !!!
};

class RBTree
{
public:
    virtual ~RBTree();

    bool checkTree();

    uint64_t countBlack(RBNode *n)
    {
        if (n == nil)
            return 0;
        return (n->color == RBNode::BLACK ? 1: 0) + countBlack(n->left) + countBlack(n->right);
    }

    void rbInsertFixup(RBNode *z, void (*updateNode)(RBNode *n, RBTree *T));
    void rbDeleteFixup(RBNode *x, void (*updateNode)(RBNode *n, RBTree *T));
    void rbDelete(RBNode *z, void (*updateNode)(RBNode *n, RBTree *T));
    RBNode *findRightSiblingLeaf(RBNode *n);
    RBNode *findLeftSiblingLeaf(RBNode *n);
    RBNode *treeSuccessor(RBNode *x);
    RBNode *treePredecessor(RBNode *x);
    RBNode *treeMinimum(RBNode *x);
    RBNode *treeMaximum(RBNode *x);

    bool isLeftChild(RBNode *n);
    bool isRightChild(RBNode *n);

    int getNodeMaxDepth(RBNode *n);
    int getNodeMinDepth(RBNode *n);

    void printSubTree(RBNode *n);
    int checkSubTree(RBNode *n);
    void checkNode(RBNode *x);

    void deleteNode(RBNode *x)
    {
        if (x->left != nil) deleteNode(x->left);
        if (x->right != nil) deleteNode(x->right);
        delete x;
    }

private:
    void leftRotate(RBNode *x, void (*updateNode)(RBNode *n, RBTree *T));
    void rightRotate(RBNode *x, void (*updateNode)(RBNode *n, RBTree *T));

public:
    RBNode *root;
    RBNode *nil;
};

}
}

NS_IZENELIB_AM_END

#endif
