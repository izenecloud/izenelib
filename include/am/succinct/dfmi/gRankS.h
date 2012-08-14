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

// ------ Dynamic Sequence with Indels -----
// uses huffman shaped wavelet tree
// space: O(n H_0) time: O((log n) H_0)
// papers: V. Maekinen, G. Navarro. Dynamic Entropy-Compressed Sequences and Full-Text
//           Indexes. CPM 2006, Chapter 3.6


// Mikael: I put the code specific to this structure in a different class
//         so that both this class and dynSA class are easier to read.

#ifndef DFMI_GRANK_S_H
#define DFMI_GRANK_S_H

#include "sbvtree.h"

#include <queue>
#include <iostream>
#include <cstring>


namespace dfmi
{

template <class CharT> class DynRankS;

template <class CharT>
class WaveletNode
{
public:
    typedef CharT char_type;
    typedef WaveletNode<CharT> self_type;

    WaveletNode(char_type c, float relativeFrequency)
        : left(0), right(0), parent(0), relativeFrequency(relativeFrequency), c0(c), bittree(0)
    {
    }

    WaveletNode(WaveletNode *left, WaveletNode *right)
        : left(left), right(right), parent(0), bittree(0)
    {
        relativeFrequency = left->relativeFrequency + right->relativeFrequency;
        left->parent = this;
        right->parent = this;
    }

    size_t countNodes()
    {
        size_t nb_nodes = 0;
        if (left)
            nb_nodes += left->countNodes();
        if (right)
            nb_nodes += right->countNodes();
        if (bittree && bittree->root)
            nb_nodes += bittree->getNbNodes((SBVNode *)bittree->root);
        return nb_nodes;
    }

    ~WaveletNode()
    {
        delete bittree;
    }

    bool operator>(const WaveletNode &a) const
    {
        return relativeFrequency > a.relativeFrequency;
    }

private:
    WaveletNode *left;
    WaveletNode *right;
    WaveletNode *parent;
    float relativeFrequency; // used only while construction
    char_type c0;      // used also while construction
    char_type c1;

    SBVTree *bittree;

    friend class DynRankS<CharT>;
};

template <class CharT>
class DynRankS
{
public:
    typedef CharT char_type;
    typedef WaveletNode<CharT> node_type;
    typedef DynRankS<CharT> self_type;

    DynRankS(size_t alphabet_num);

    ~DynRankS();

    float *createCharDistribution(const char_type *sampleText, size_t sampleLength);
    void initEmptyDynRankS(const float *f); //argument: array containing relative frequencies of characters

    void empty();

    void insert(char_type c, size_t i);
    char_type deleteSymbol(size_t i);

    char_type operator[](size_t i);
    size_t rank(char_type c, size_t i);
    size_t select(char_type c, size_t i);

    size_t getSize();
    size_t nbNodes();

    //Iterator
    void iterateReset();
    bool iterateNext();
    char_type iterateGetSymbol();
    void recursiveIterateResetWaveletNode(node_type *w);

private:
    // functions
    void makeCodes(size_t code, int bits, node_type *node);
    void deleteLeaves(node_type *node);
    void appendBVTrees(node_type *node);
    void deleteDynRankSNodes(node_type *n);

private:
    node_type *root;    // root of the wavelet tree
    node_type **leaves; // needed for construction and select

    size_t alphabet_num;
    std::vector<size_t> codes;
    size_t iterate;
};

template <class CharT>
float *DynRankS<CharT>::createCharDistribution(const char_type *sampleText, size_t sampleLength)
{
    if (!sampleLength)
        return new float[alphabet_num]();

    size_t *count = new size_t[alphabet_num]();

    for (size_t i = 0; i < sampleLength; ++i)
    {
        ++count[sampleText[i]];
    }

    float *f = new float[alphabet_num];

    for (size_t i = 0; i < alphabet_num; ++i)
    {
        f[i] = (float)count[i] / (float)sampleLength;
    }

    return f;
}

template <class CharT>
DynRankS<CharT>::DynRankS(size_t alphabet_num)
    : alphabet_num(alphabet_num)
    , codes(alphabet_num)
{
}

template <class CharT>
DynRankS<CharT>::~DynRankS()
{
    empty();
}

template <class CharT>
void DynRankS<CharT>::initEmptyDynRankS(const float *f)
{
    // pointers to the leaves for select
    leaves = new node_type*[alphabet_num];

    for (size_t j = 0; j < alphabet_num; ++j)
    {
        //all possible characters
        if (f[j] != 0.0f) //only those that exist
            leaves[j] = new node_type((char_type)j, f[j]);
        else
            leaves[j] = NULL;
    }

    // Huffman shape, Veli's approach:
    std::priority_queue<node_type *, std::vector<node_type *>, std::greater<node_type *> > q;
    for (size_t j = 0; j < alphabet_num; ++j)
    {
        if (leaves[j])
        {
            q.push((leaves[j]));
        }
    }

    // creates huffman shape:
    while (q.size() > 1)
    {
        node_type *left = q.top();
        q.pop();

        node_type *right = q.top();
        q.pop();

        q.push(new node_type(left, right));
    }

    root = q.top();
    q.pop();

    makeCodes(0, 0, root);	// writes codes

    // merge leaves	(one leaf represent two characters!)
    for (size_t j = 0; j < alphabet_num; ++j)
    {
        if (leaves[j])
        {
            if (leaves[j]->parent->left == leaves[j])
            {
                leaves[j]->parent->c0 = j;
            }
            else
            {
                leaves[j]->parent->c1 = j;
            }
            leaves[j] = leaves[j]->parent; // merge
        }
    }

    deleteLeaves(root);
    appendBVTrees(root);
}

template <class CharT>
void DynRankS<CharT>::empty()
{
    deleteDynRankSNodes(root);
    delete[] leaves;
}

template <class CharT>
void DynRankS<CharT>::insert(char_type c, size_t i)
{
    size_t level = 0;
    size_t code = codes[c];

    bool bit;
    node_type *walk = root;

    while (walk)
    {
        bit = code & 1U << level;

        walk->bittree->insertBit(bit, i); // TODO improve
        i = walk->bittree->rank(bit, i);

        if (bit)
        {
            //bit = 1
            walk = walk->right;
        }
        else
        {
            // bit = 0
            walk = walk->left;
        }

        ++level;
    } // end of while
}

template <class CharT>
CharT DynRankS<CharT>::deleteSymbol(size_t i)
{
    node_type *walk = root;
    bool bit;
    char_type c;
    while (true)
    {
        walk->bittree->deleteBit(i);
        bit = walk->bittree->getLastDeletedBit();
        i = walk->bittree->getLastDeletedRank();

        if (bit)
        {
            //bit = 1
            if (!walk->right)
            {
                c = walk->c1;
                break;
            }
            walk = walk->right;
        }
        else
        {
            // bit = 0
            if (!walk->left)
            {
                c = walk->c0;
                break;
            }
            walk = walk->left;
        }
    } // end of while
    return c;
}

template <class CharT>
CharT DynRankS<CharT>::operator[](size_t i)
{
    node_type *walk = root;
    bool bit;

    while (true)
    {
        bit = (*walk->bittree)[i]; //TODO improve by reducing
        i = walk->bittree->rank(bit, i);

        if (bit)
        {
            //bit = 1
            if (!walk->right) return walk->c1;
            walk = walk->right;
        }
        else
        {
            // bit = 0
            if (!walk->left) return walk->c0;
            walk = walk->left;
        }
    } // end of while
}

template <class CharT>
size_t DynRankS<CharT>::rank(char_type c, size_t i)
{
    if (!i) return 0;

    size_t level = 0;
    size_t code = codes[c];

    bool bit;
    node_type *walk = root;

    while (i > 0)
    {
        bit = code & 1U << level;

        i = walk->bittree->rank(bit, i);
        if (bit)
        {
            //bit = 1
            if (!walk->right) return i;
            walk = walk->right;
        }
        else
        {
            // bit = 0
            if (!walk->left) return i;
            walk = walk->left;
        }

        ++level;
    } // end of while

    return 0;
}

template <class CharT>
size_t DynRankS<CharT>::select(char_type c, size_t i)
{
    node_type *walk = leaves[c];

    bool bit = walk->c1 == c;

    while (walk->parent)
    {
        i = walk->bittree->select(bit, i);

        bit = walk == walk->parent->right;
        walk = walk->parent;
    } // end of while

    i = walk->bittree->select(bit, i);

    return i;
}

template <class CharT>
size_t DynRankS<CharT>::getSize()
{
    return root->bittree->getLength();
}

template <class CharT>
size_t DynRankS<CharT>::nbNodes()
{
    return root->countNodes();
}

template <class CharT>
void DynRankS<CharT>::iterateReset()
{
    iterate = 1;
    recursiveIterateResetWaveletNode(root);
}

template <class CharT>
bool DynRankS<CharT>::iterateNext()
{
    return !(++iterate > getSize());
}

template <class CharT>
CharT DynRankS<CharT>::iterateGetSymbol()
{
    node_type *walk = root;
    bool bit;

    while (true)
    {
        bit = walk->bittree->iterateGetBit(); // TODO improve

        walk->bittree->iterateNext();

        if (bit)
        {
            //bit = 1
            if (!walk->right) return walk->c1;
            walk = walk->right;
        }
        else
        {
            // bit = 0
            if (!walk->left) return walk->c0;
            walk = walk->left;
        }
    } // end of while
}

template <class CharT>
void DynRankS<CharT>::recursiveIterateResetWaveletNode(node_type *w)
{
    w->bittree->iterateReset();

    if (w->left) recursiveIterateResetWaveletNode(w->left);
    if (w->right) recursiveIterateResetWaveletNode(w->right);
}

template <class CharT>
void DynRankS<CharT>::makeCodes(size_t code, int bits, node_type *node)
{
    if (node->left)
    {
        makeCodes(code | (0 << bits), bits + 1, node->left);
        makeCodes(code | (1 << bits), bits + 1, node->right);
    }
    else
    {
        codes[node->c0] = code;
    }
}

template <class CharT>
void DynRankS<CharT>::deleteLeaves(node_type *node)
{
    bool leaf = true;

    if (node->left)
    {
        // internal node
        leaf = false;
        deleteLeaves(node->left);
    }

    if (node->right)
    {
        leaf = false;
        deleteLeaves(node->right);
    }

    if (leaf)
    {
        // is a leaf, delete it!
        if (node->parent)
        {
            if (node == node->parent->left) node->parent->left = 0;
            else node->parent->right = 0;
        }
        delete node;
    }
}

template <class CharT>
void DynRankS<CharT>::appendBVTrees(node_type *node)
{
    node->bittree = new SBVTree();

    if (node->left) appendBVTrees(node->left);
    if (node->right) appendBVTrees(node->right);
}

template <class CharT>
void DynRankS<CharT>::deleteDynRankSNodes(node_type *n)
{
    if (n->right) deleteDynRankSNodes(n->right);
    if (n->left) deleteDynRankSNodes(n->left);

    delete n;
}

}

namespace std
{

template <class CharT>
struct greater<dfmi::WaveletNode<CharT> *>
{
    bool operator()(dfmi::WaveletNode<CharT> const *p1, dfmi::WaveletNode<CharT> const *p2)
    {
        if (!p1)
            return false;
        if (!p2)
            return true;
        return *p1 > *p2;
    }
};

}

#endif
