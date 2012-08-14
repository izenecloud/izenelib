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

// Implementation of the Dynamic Bit Vector with Indels problem
// space: O(n) time: O(log(n))
// papers: V. Maekinen, G. Navarro. Dynamic Entropy-Compressed Sequences and Full-Text
//           Indexes. CPM 2006, Chapter 3.4 Dynamic Structures for Bit Vectors
//   also: W.-L. Chan, W.-K. Hon, and T.-W. Lam. Compressed index for a dynamic collection
//           of texts. In Proc. CPM04, LNCS 3109, pages 445-456, 2004

#ifndef DFMI_SBV_TREE_H
#define DFMI_SBV_TREE_H

#include "rbtree.h"

// Size of a block in bits.
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16
#endif

// Type that fits best to store a block
#ifndef uint_block
#define uint_block uint16_t
#endif

// Maximum value on BLOCK_SIZE bits.
#ifndef BS_MAX
#define BS_MAX 0xFFFF
#endif

// logarithm (base 2) of the block size
// we use this for the division by the block size (with bit shiftings).
// if one does not want a power of two as a block size, then the code
// using LOG_BS and log_bs has to be slightly modified (probably using an
// integer division rather than a bit shifting).
#ifndef LOG_BS
#define LOG_BS 4
#endif

// Number of blocks stored in a node.
#ifndef NB_BLOCKS
#define NB_BLOCKS (4 * BLOCK_SIZE)
#endif

/* MIN_MERGE*superblock_size corresponds to the minimal number of bits
 * under which we merge two nodes */
#define MIN_MERGE 0.5

/*
 * MIN_EXCHANGE*superblock_size corresponds to the minimal number of
 * bits we exchange under merge and split (during deleteBit).
 * Should always be > 32
 */
#define MIN_EXCHANGE 0.1

/* When merging two nodes, we do not want to have a full node (ie.
 * a node that has superblock_size bits). MAX_AFTER_MERGE is the maximal
 * percentage of the superblock_size after merging.
 */
#define MAX_AFTER_MERGE 0.9

#define RANK 0
#define POSITIONS 1

namespace dfmi
{

class SBVTree;

void callUpdateCounters(RBNode *n, RBTree *T);
void callUpdateCountersOnPathToRoot(RBNode *n, RBTree *T);

const uint_block block_size = BLOCK_SIZE;
const uint32_t nb_blocks = NB_BLOCKS;
const uint32_t superblock_size = nb_blocks * block_size;
const uint32_t log_bs = LOG_BS;

class SBVNode : public RBNode
{
public:
    size_t myPositions; // 4*4 bytes = 16 bytes
    size_t myRank;
    size_t subtreePositions; //number of positions stored in the subtree rooted at this node
    size_t subtreeRank;      //number of bits set in the subtree rooted at this node

    uint_block blocks[nb_blocks] ; // we store 2*block_size^2 bits (in theory block_size = log(n) / 2 and thus we store log^2(n) bits).

    SBVNode()
        : RBNode(this), myPositions(0), myRank(0), subtreePositions(0), subtreeRank(0)
    {
    }

    SBVNode(SBVNode *n)
        : RBNode(n), myPositions(0), myRank(0), subtreePositions(0), subtreeRank(0)
    {
    }

    ~SBVNode()
    {
    }

    SBVNode *getParent()
    {
        return (SBVNode *)parent;
    }

    SBVNode *getLeft()
    {
        return (SBVNode *)left;
    }

    SBVNode *getRight()
    {
        return (SBVNode *)right;
    }

    SBVNode *copy(SBVTree *, SBVTree*, SBVNode *);

    void setParent(SBVNode *n)
    {
        parent = (RBNode *)n;
    }

    void setLeft(SBVNode *n)
    {
        left = (RBNode *)n;
    }

    void setRight(SBVNode *n)
    {
        right = (RBNode *)n;
    }
};

class SBVTree : public RBTree
{
public:
    /* Precomputed table giving the number of ones in a block */
    static unsigned char *number_ones; // size: 1<<block_size
    static uint_block *mask_positions; // size: block_size
    static bool init_done;

    static void init();

    //Constructors
    SBVTree()
    {
        init();

        setNil(new SBVNode());
        setRoot(getNil());

        tempnil = getNil();
    }

    //Destructor:
    ~SBVTree();

    /**
     * Copy the current SBVtree in the variable given in parameter.
     */
    SBVTree *copy();
    bool operator[](size_t);

    // inserts bit at position i, countings begins with 1:
    void appendBit(bool bit);
    void insertBit(bool bit, size_t i);
    void deleteBit(size_t i);
    void setBit(size_t i);
    void unsetBit(size_t i);
    void changeBit(size_t i, bool bit);

    bool getLastDeletedBit();
    size_t getLastDeletedRank();

    size_t rank0(size_t i);
    size_t rank1(size_t i);
    size_t rank(bool b, size_t i)
    {
        return b ? rank1(i) : rank0(i);
    }

    size_t select0(size_t i);
    size_t select1(size_t i);
    size_t select(bool b, size_t i)
    {
        return b ? select1(i) : select0(i);
    }

    /**
     * Copy nb_bits_copied bits from *ptab_src at position start_src to the position
     * 0 of *ptab_dest.
     * Note that ptab_dest and ptab_src are pointers on arrays and can designate
     * the same array.
     */
    uint64_t array_copyleft(uint_block *ptab_dest, uint_block *ptab_src,  uint64_t start_src, uint64_t nb_bits_copied);

    /**
     * Copy nb_bits_copied bits from *ptab_src at the first position to the position
     * start_dest in *ptab_dest.
     */
    uint64_t array_copyright(uint_block *ptab_dest, uint_block *ptab_src, uint64_t start_dest, uint64_t nb_bits_copied);

    void setRoot(SBVNode *n)
    {
        root = (RBNode *)n;
    }

    SBVNode *getRoot()
    {
        return (SBVNode *)root;
    }

    void setNil(SBVNode *n)
    {
        tempnil = n;
        nil = (RBNode *)n;
        n->color = RBNode::BLACK;
    }

    SBVNode *getNil()
    {
        return tempnil;
    }

    uint64_t getNbNodes(SBVNode *current_node)
    {
        uint64_t nb = 0;
        if (current_node->left != getNil())
            nb += getNbNodes((SBVNode *)current_node->left);
        if (current_node->right != getNil())
            nb += getNbNodes((SBVNode *)current_node->right);
        return nb + 1;
    }

    // write bits back into a stream:
    void displayBits();
    uint_block *getBits();

    int getTreeMaxDepth();
    int getTreeMinDepth();
    size_t getLength();

    void iterateReset();
    bool iterateGetBit();
    bool iterateNext();
    size_t iterateGetRank(bool bit);

    void checkSubTree(SBVNode *n);

    void updateCounters(SBVNode *n);
    void updateCountersOnPathToRoot(SBVNode *walk);

    //debug:
    void printNode(size_t i);
    void printSubtree(SBVNode *node);

protected:
    size_t iterate;             /* total position in the bit vector */
    size_t iterateLocal;        /* position inside the block */
    size_t iterateBlock;         /* block number */
    size_t iterateRank;         /* total rank */
    SBVNode *iterateNode;

    bool lastDeletedBit;
    size_t lastDeletedRank;

    SBVNode *tempnil;

    // content of BVNode, for debugging:
    void printNode(SBVNode *n);

    // other operations:
    size_t getLocalRank(SBVNode *n, size_t position);
    size_t getLocalSelect0(SBVNode *n, size_t query);
    size_t getLocalSelect1(SBVNode *n, size_t query);

    SBVNode *getNodeWithIthBit(size_t i, uint64_t *infos);

    void deleteNode(SBVNode *n);
    void deleteLeaf(SBVNode *leaf);
};

} // namespace

#endif
