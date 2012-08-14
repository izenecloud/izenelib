/***************************************************************************
 *   Dynamic bit vectors having n/log^2(n) nodes only                      *
 *                 2009  Mikael Salson                                     *
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

#include <am/succinct/dfmi/sbvtree.h>

#include <iostream>
#include <cstdlib>


namespace dfmi
{

// Init the static variables
unsigned char *SBVTree::number_ones = new unsigned char[1 << block_size];
uint_block *SBVTree::mask_positions = new uint_block[block_size];
bool SBVTree::init_done = false;

SBVNode *SBVNode::copy(SBVTree *treeSrc, SBVTree *treeDest, SBVNode *parent)
{
    SBVNode *n = new SBVNode(treeDest->getNil());
    n->parent = parent;
    if (left != treeSrc->getNil())
    {
        n->left = ((SBVNode *)left)->copy(treeSrc, treeDest, n);
    }
    else
    {
        n->left = treeDest->getNil();
    }

    if (right != treeSrc->getNil())
    {
        n->right = ((SBVNode *)right)->copy(treeSrc, treeDest, n);
    }
    else
    {
        n->right = treeDest->getNil();
    }
    n->color = color;
    n->myPositions = myPositions;
    n->myRank = myRank;
    n->subtreePositions = subtreePositions;
    n->subtreeRank = subtreeRank;
    for (uint32_t i = 0; i < nb_blocks; ++i)
    {
        n->blocks[i] = blocks[i];
    }
    return n;
}

SBVTree::~SBVTree()
{
}

uint64_t SBVTree::array_copyleft(uint_block *tab_dest, uint_block *tab_src,  uint64_t start_src, uint64_t nb_bits_copied)
{
    if (!nb_bits_copied)
        return 0;
    uint64_t current_cell_dest = 0;
    uint64_t current_cell_src = start_src >> log_bs;
    uint_block pos_cell_src = start_src & (block_size - 1);
    uint_block remaining_bits = nb_bits_copied & (block_size - 1);
    uint64_t nb_memb = (nb_bits_copied >> log_bs);
    uint64_t nb_bits_set = 0;

    uint_block lsb_mask, msb_mask, lsb_shift, msb_shift;
    uint_block dest_lsb_mask, dest_msb_mask;
    lsb_mask = (1 << pos_cell_src) - 1;
    msb_mask = BS_MAX ^ lsb_mask;
    lsb_shift = block_size - pos_cell_src;
    msb_shift = pos_cell_src;

    if (lsb_shift == block_size)
        dest_msb_mask = BS_MAX;
    else
        dest_msb_mask = (1 << lsb_shift) - 1;
    dest_lsb_mask = BS_MAX ^ dest_msb_mask;

    uint64_t nb_slices = 2 * nb_memb;
    if (!lsb_mask)
        --nb_slices;
    uint_block value;

    for (uint64_t i = 0; i < nb_slices; ++i)
    {
        if ((i & 1))
        {
            // Gets the least significant bits of the next cell and shift them
            // so that they are the most significant of the current cell.
            value = tab_src[current_cell_src] & lsb_mask;
            tab_dest[current_cell_dest] = (tab_dest[current_cell_dest] & dest_msb_mask) | value << lsb_shift;
            ++current_cell_dest;
        }
        else
        {
            // Gets the most significant bits and shift them so that they are
            // the least significant
            value = tab_src[current_cell_src] & msb_mask;
            tab_dest[current_cell_dest] = (tab_dest[current_cell_dest] & dest_lsb_mask) | value >> msb_shift;
            ++current_cell_src;
        }
        nb_bits_set += number_ones[value];
    }

    if (remaining_bits > 0)
    {
        if (!lsb_mask)
            ++current_cell_dest;
        // We do not want to overwrite the most significant bits of the cell (ie. the (block_size - remaining_bits) bits).
        // if remaining_bits == 2 (and block_size == 16) we have 14 zeros (msb) followed by 2 ones in keepbits_mask. The ones correspond to the values we want to replace in the original cell.
        uint_block replacebits_mask = (1 << remaining_bits) - 1;
        tab_dest[current_cell_dest] = (tab_dest[current_cell_dest] & (BS_MAX ^ replacebits_mask))
            | (((tab_src[current_cell_src] & msb_mask) >> msb_shift) & replacebits_mask);
        nb_bits_set += number_ones[tab_dest[current_cell_dest] & replacebits_mask];
        if (remaining_bits > lsb_shift)
        {
            ++current_cell_src;
            remaining_bits -= lsb_shift;
            lsb_mask = (1 << remaining_bits) - 1;
            tab_dest[current_cell_dest] = (tab_dest[current_cell_dest]
                    & (BS_MAX ^ (lsb_mask << lsb_shift)))
                | ((tab_src[current_cell_src] & lsb_mask) << lsb_shift);
            nb_bits_set += number_ones[tab_src[current_cell_src] & lsb_mask];
        }
    }
    return nb_bits_set;
}

uint64_t SBVTree::array_copyright(uint_block *tab_dest, uint_block *tab_src, uint64_t start_dest, uint64_t nb_bits_copied)
{
    if (!nb_bits_copied)
        return 0;
    uint64_t final_dest = start_dest + nb_bits_copied - 1;
    uint64_t current_cell_dest = final_dest / block_size;
    uint_block pos_cell_dest = start_dest & (block_size - 1);
    uint_block pos_right_cell_dest = final_dest & (block_size - 1);
    uint64_t current_cell_src = 0;
    uint64_t nb_memb = (nb_bits_copied >> log_bs) + ((nb_bits_copied & (block_size - 1)) != 0);

    uint_block msb_shift = block_size - pos_cell_dest;
    uint_block lsb_mask;
    uint64_t nb_bits_set = 0;

    if (msb_shift == block_size)
        lsb_mask = BS_MAX;
    else
        lsb_mask = (1 << msb_shift) - 1;
    uint_block msb_mask = BS_MAX ^ lsb_mask;
    current_cell_src += nb_memb - 1;

    if (current_cell_src > 0)
    {
        // Treats the first cell separately
        if (pos_right_cell_dest < block_size - 1)
            tab_dest[current_cell_dest] &= BS_MAX ^ ((1 << (pos_right_cell_dest + 1)) - 1);
        else
            tab_dest[current_cell_dest] = 0;
        uint64_t tmp_rank = number_ones[tab_dest[current_cell_dest]];

        if (pos_right_cell_dest < pos_cell_dest)
        {
            // the copy ends in the msb
            uint64_t tmp_msb_mask = ((1 << (pos_right_cell_dest + 1)) - 1) << msb_shift;
            tab_dest[current_cell_dest] |= (tab_src[current_cell_src] & tmp_msb_mask) >> msb_shift;
        }
        else
        {
            uint64_t tmp_lsb_mask;
            if (pos_right_cell_dest - pos_cell_dest + 1 == block_size)
                tmp_lsb_mask = BS_MAX;
            else
                tmp_lsb_mask = ((1 << (pos_right_cell_dest - pos_cell_dest + 1)) - 1);
            tab_dest[current_cell_dest] |= (tab_src[current_cell_src] & tmp_lsb_mask) << pos_cell_dest;
            tab_dest[current_cell_dest] |= (tab_src[current_cell_src - 1] & msb_mask) >> msb_shift;
            --current_cell_src;
        }

        nb_bits_set += number_ones[tab_dest[current_cell_dest]] - tmp_rank;
        --current_cell_dest;
    }

    while (current_cell_src > 0)
    {
        tab_dest[current_cell_dest] = (tab_src[current_cell_src] & lsb_mask) << pos_cell_dest;
        tab_dest[current_cell_dest] |= (tab_src[current_cell_src - 1] & msb_mask) >> msb_shift;
        nb_bits_set += number_ones[tab_dest[current_cell_dest]];
        --current_cell_dest;
        --current_cell_src;
    }
    // Treating last cell. We have to take care not to overwrite everything in the
    // destination cell.
    uint64_t keep_bits = (1 << pos_cell_dest) - 1;
    nb_bits_set += number_ones[tab_src[current_cell_src] & lsb_mask];
    tab_dest[current_cell_dest] = (tab_dest[current_cell_dest] & keep_bits) | (tab_src[current_cell_src] & lsb_mask) << pos_cell_dest;

    return nb_bits_set;
}

void SBVTree::init()
{
    if (!init_done)
    {
        // Computing the table
        uint64_t power = 0;
        uint64_t i = 2;
        number_ones[0] = 0;
        number_ones[1] = 1;
        while (i < (1 << block_size))
        {
            if (i == (uint64_t)1 << (power + 1))
                ++power;
            number_ones[i] = number_ones[i - (1 << power)] + 1;
            ++i;
        }

        // "Increasing" masks : ..00001 ...00011 ...00111 ...01111
        // that can cover the whole block
        uint_block current_mask = 1;
        i = 0;
        while (i < block_size)
        {
            mask_positions[i] = current_mask;
            current_mask = current_mask << 1 | 1;
            ++i;
        }

        init_done = true;
    }
}

void SBVTree::iterateReset()
{
    iterate = 1;
    iterateLocal = 0;
    iterateBlock = 0;
    if (root != nil)
    {
        iterateNode = (SBVNode *)treeMinimum(root);
        iterateRank = iterateNode->blocks[0] & 1;
    }
    else
    {
        iterateNode = getNil();
        iterateRank = 0;
    }
}

bool SBVTree::iterateGetBit()
{
    return iterateNode->blocks[iterateBlock] & 1 << iterateLocal;
}

size_t SBVTree::iterateGetRank(bool bit)
{
    if (bit) return iterateRank;
    else return (iterate - iterateRank);
}

bool SBVTree::iterateNext()
{
    ++iterate;

    if (iterate > getLength()) return false;

    ++iterateLocal;
    if (iterateLocal == block_size)
    {
        iterateLocal = 0;
        ++iterateBlock;
    }
    if (iterateLocal + iterateBlock * block_size + 1 > iterateNode->myPositions
            || iterateBlock == nb_blocks)
    {
        // jump to next node;
        iterateNode = (SBVNode *)treeSuccessor(iterateNode);
        iterateLocal = 0;
        iterateBlock = 0;
    }

    if (iterateGetBit()) ++iterateRank;

    return true;
}

SBVNode *SBVTree::getNodeWithIthBit(size_t i, uint64_t *infos)
{
    SBVNode *current_node = getRoot();
    size_t nb_left;
    size_t min_right;
    SBVNode *left_node;
    size_t left_subtreeRank;
    bool not_found = true;

    do
    {
        left_node = current_node->getLeft();
        if (left_node == getNil())
        {
            left_subtreeRank = nb_left = 0;
        }
        else
        {
            nb_left = left_node->subtreePositions;
            left_subtreeRank = left_node->subtreeRank;
        }
        min_right = nb_left + current_node->myPositions;

        if (i <= nb_left)
        {
            current_node = left_node;
        }
        else if (i > min_right && current_node->getRight() != getNil())
        {
            i -= min_right;
            if (infos)
            {
                infos[RANK] += left_subtreeRank + current_node->myRank;
                infos[POSITIONS] += min_right;
            }
            current_node = current_node->getRight();
        }
        else
        {
            if (infos)
            {
                infos[RANK] += left_subtreeRank;
                infos[POSITIONS] += nb_left;
            }
            not_found = false;
        }
    } while (not_found && current_node != getNil());

    if (current_node == getNil())
    {
        std::cerr << "prob getNodeWithIthBit : " << i << std::endl;
    }

    return current_node;
}

size_t SBVTree::getLength()
{
    return ((SBVNode *)root)->subtreePositions;
}

void SBVTree::printNode(SBVNode *n)
{
    std::cout << "address: " << n << std::endl;

    if (n != getNil())
    {
        std::cout << "taille : " << n->myPositions << std::endl;
        for (uint32_t i = 0; i < n->myPositions; ++i)
        {
            if (i % block_size == 0)
                std::cout << " ";
            std::cout << ((n->blocks[i >> log_bs] & 1 << (i & (block_size - 1))) != 0);
        }
        std::cout << std::endl;
        std::cout << "n->myPositions=" << n->myPositions << std::endl;
        std::cout << "myRank: " << n->myRank << std::endl;
        std::cout << "subtreePositions: " << n->subtreePositions << std::endl;
        std::cout << "subtreeRank: " << n->subtreeRank << std::endl;
        std::cout << "color: " << ((n->color == dfmi::RBNode::RED) ? "RED" : "BLACK") << std::endl;
        std::cout << "parent: " << n->getParent() << std::endl;
        std::cout << "left: " << n->getLeft() << std::endl;
        std::cout << "right:" << n->getRight() << std::endl << std::endl;
    }
    else
    {
        std::cout << "NULL" << std::endl << std::endl;
    }
}

int SBVTree::getTreeMaxDepth()
{
    return getNodeMaxDepth(root);
}

int SBVTree::getTreeMinDepth()
{
    return getNodeMinDepth(root);
}

void SBVTree::updateCounters(SBVNode *n)
{
    if (n == getNil()) return;

    size_t leftRank = 0;
    size_t leftPositions = 0;
    size_t rightRank = 0;
    size_t rightPositions = 0;

    if (n->getLeft() != getNil())
    {
        leftRank = n->getLeft()->subtreeRank;
        leftPositions = n->getLeft()->subtreePositions;
    }

    if (n->getRight() != getNil())
    {
        rightPositions = n->getRight()->subtreePositions;
        rightRank = n->getRight()->subtreeRank;
    }

    n->subtreeRank = leftRank + rightRank + n->myRank;
    n->subtreePositions = leftPositions + rightPositions + n->myPositions;
}

size_t SBVTree::getLocalRank(SBVNode *n, size_t position)
{
#ifndef NDEBUG
    if (position > n->myPositions)
    {
        std::cerr << "error: getLocalRank: invalid position in block.\n";
        exit(EXIT_FAILURE);
    }
#endif

    size_t i = 0;
    size_t nb_ones = 0;
    while (position >= block_size)
    {
#ifndef NDEBUG
        uint_block tmp_dbg = n->blocks[i];
        std::cerr << "i = " << i << ", n->blocks[i] = " << tmp_dbg << std::endl;
        std::cerr << "number_ones: " << (int) number_ones[tmp_dbg] << std::endl;
#endif
        nb_ones += number_ones[n->blocks[i]];
        ++i;
        position -= block_size;
    }
    nb_ones += number_ones[n->blocks[i] & mask_positions[position]];
    return nb_ones;
}

size_t SBVTree::getLocalSelect1(SBVNode *n, size_t query)
{
#ifndef NDEBUG
    if (query > n->myPositions)
    {
        std::cerr << "error: getLocalSelect1: invalid position in block.\n";
        exit(EXIT_FAILURE);
    }
#endif
    size_t select =0;
    size_t i = 0;
    while (select < query)
    {
        select += number_ones[n->blocks[i]];
        ++i;
    }
    --i;
    // We know which block contains the position, now, we kneed the
    // exact position
    size_t position = block_size;
    // We have too much ones. How many to discard?
    size_t ones_remaining = select - query + 1;
    uint_block mask = 1 << (position - 1);
    while (ones_remaining > 0)
    {
        --position;
        ones_remaining -= (n->blocks[i] & mask) != 0;
        mask >>= 1;
    }
    return i * block_size + position + 1;
}

size_t SBVTree::getLocalSelect0(SBVNode *n, size_t query)
{
#ifndef NDEBUG
    if (query > n->myPositions)
    {
        std::cerr << "error: getLocalSelect0: invalid position in block.\n";
        exit(EXIT_FAILURE);
    }
#endif
    size_t select = 0;
    size_t i = 0;
    while (select < query)
    {
        select += block_size - number_ones[n->blocks[i]];
        ++i;
    }
    --i;
    // We know which block contains the position, now, we kneed the
    // exact position
    size_t position = block_size;
    // We have too much zeroes. How many to discard?
    size_t zeroes_remaining = select - query + 1;
    uint_block mask = 1 << (position - 1);
    while (zeroes_remaining > 0)
    {
        --position;
        zeroes_remaining -= (n->blocks[i] & mask) == 0;
        mask >>= 1;
    }
    return i * block_size + position + 1;
}

void SBVTree::printNode(size_t i)
{
    printNode(getNodeWithIthBit(i, NULL));
}

void SBVTree::printSubtree(SBVNode *node)
{
    printNode(node);
    if (node != getNil())
    {
        printSubtree(node->getLeft());
        printSubtree(node->getRight());
    }
}

SBVTree *SBVTree::copy()
{
    SBVTree *tree = new SBVTree();
    tree->root = ((SBVNode *)root)->copy(this, tree, tree->getNil());
    return tree;
}

bool SBVTree::operator[](size_t i)
{
    uint64_t infos[2] = {0, 0};
    SBVNode *x = getNodeWithIthBit(i, infos);
    i -= infos[POSITIONS] + 1;                        // We are starting at 1.

    /*
       [i >> log_bs]: get the right entry equiv. to i/block_size
       1 << (i & (block_size - 1)): 1 << (i % block_size) -> get the right position in the block
       */
    return x->blocks[i >> log_bs] & 1 << (i & (block_size - 1));
}

size_t SBVTree::rank1(size_t i)
{
    if (i == getLength() + 1) --i;

    uint64_t infos[2] = {0, 0};
    SBVNode *x = getNodeWithIthBit(i, infos);
    infos[RANK] += getLocalRank(x, i - infos[POSITIONS] - 1);
    return infos[RANK];
}

size_t SBVTree::rank0(size_t i)
{
    if (!getLength()) return 0;
    return (i - rank1(i));
}

size_t SBVTree::select1(size_t i)
{
    //find the corresponding node:
    SBVNode *current_node = getRoot();
    size_t min_right;
    SBVNode *left_node;
    size_t left_subtreeRank, left_subtreePositions;
    size_t position = 0;
    bool not_found = true;

    do
    {
        left_node = current_node->getLeft();
        if (left_node == getNil())
        {
            left_subtreeRank = left_subtreePositions = 0;
        }
        else
        {
            left_subtreeRank = left_node->subtreeRank;
            left_subtreePositions = left_node->subtreePositions;
        }
        min_right = left_subtreeRank + current_node->myRank;

        if (i <= left_subtreeRank)
        {
            current_node = left_node;
        }
        else if (i > min_right)
        {
            position += left_subtreePositions + current_node->myPositions;
            current_node = current_node->getRight();
            i -= min_right;
        }
        else
        {
            i -= left_subtreeRank;
            position += left_subtreePositions;
            not_found = false;
        }
    } while (not_found);

    position += getLocalSelect1(current_node, i);

    return position;
}

size_t SBVTree::select0(size_t i)
{
    //find the corresponding node:
    SBVNode *current_node = getRoot();
    size_t min_right;
    SBVNode *left_node;
    size_t left_subtreeRank, left_subtreePositions;
    size_t position = 0;
    bool not_found = true;

    do
    {
        left_node = current_node->getLeft();
        if (left_node == getNil())
        {
            left_subtreeRank = left_subtreePositions = 0;
        }
        else
        {
            left_subtreeRank = left_node->subtreePositions - left_node->subtreeRank;
            left_subtreePositions = left_node->subtreePositions;
        }
        min_right = left_subtreeRank + (current_node->myPositions - current_node->myRank);

        if (i <= left_subtreeRank)
        {
            current_node = left_node;
        }
        else if (i > min_right)
        {
            position += left_subtreePositions + current_node->myPositions;
            current_node = current_node->getRight();
            i -= min_right;
        }
        else
        {
            i -= left_subtreeRank;
            position += left_subtreePositions;
            not_found = false;
        }
    } while (not_found);

    position += getLocalSelect0(current_node, i);

    return position;
}

void SBVTree::updateCountersOnPathToRoot(SBVNode *walk)
{

    while (walk != getNil())
    {
        updateCounters(walk);
        walk = walk->getParent();
    }
}

//deletes the SBVNode and all its children, destroys reb-black-tree property!
void SBVTree::deleteNode(SBVNode *n)
{
    if (n == getNil()) return;

    if (n->getLeft() != getNil()) deleteNode(n->getLeft());
    if (n->getRight() != getNil()) deleteNode(n->getRight());

    delete n;
}

void SBVTree::deleteBit(size_t i)
{
    SBVNode *x;
    bool bit;
    uint64_t infos[2] = {0, 0};

    x = getNodeWithIthBit(i, infos);
    i -= infos[POSITIONS] + 1;

    size_t current_block = i >> log_bs;
    size_t current_position = i & (block_size - 1);

    // gets the value of the deleted bit
    bit = x->blocks[current_block] & (1 << current_position);
    lastDeletedBit = bit;
    lastDeletedRank = infos[RANK] + getLocalRank(x, i);
    if (!bit)
        lastDeletedRank = infos[POSITIONS] + i + 1 - lastDeletedRank;

    // We have to shift by one position everything which is beyond the deletion position!

    // First, inside the block
    uint_block mask = BS_MAX ^ mask_positions[current_position]; // mask for the bits we have to shift.
    uint_block shifted = ((x->blocks[current_block]) & mask) >> 1;
    if (current_position > 0)
        x->blocks[current_block] = (x->blocks[current_block] & mask_positions[current_position - 1]) | shifted;
    else
        x->blocks[current_block] =  shifted;

    // Then, between blocks
    size_t nb_blocks = (x->myPositions >> log_bs);
    size_t remainder = (x->myPositions & (block_size - 1));
    if (remainder > 0)
        ++nb_blocks;
    ++current_block;
    while (current_block < nb_blocks)
    {
        // We put the first bit of this block at the last position of the previous block
        x->blocks[current_block - 1] = x->blocks[current_block - 1] | ((x->blocks[current_block] & 1) << (block_size - 1));
        // and now we shift the block
        x->blocks[current_block] >>= 1;
        ++current_block;
    }

#ifndef NDEBUG
    if (i > x->myPositions)
    {
        std::cerr << "error: B, position " << i << " in block not available, only " << x->myPositions << " positions.\n"; //shouldn't happen
        exit(EXIT_FAILURE);
    }
#endif

    --x->myPositions;
    if (bit) --x->myRank;

    updateCountersOnPathToRoot(x);

    if (!x->myPositions)
    {

        rbDelete(x, callUpdateCountersOnPathToRoot);
        delete x;
    }
    else if (x->myPositions <= MIN_MERGE * superblock_size) // merge (and rotate), if necessary:
    {

        SBVNode *rightSibling = (SBVNode *)treeSuccessor(x);
        SBVNode *leftSibling = (SBVNode *)treePredecessor(x);

        uint64_t nbPositionsRight = 0, nbPositionsLeft = 0;
        bool merge = false, merge_n_split = false;
        uint64_t rank;

        if (rightSibling != getNil())
            nbPositionsRight = rightSibling->myPositions;
        if (leftSibling != getNil())
            nbPositionsLeft = leftSibling->myPositions;

        /* We are considering merging with right sibling:
           - there are less than MAX_AFTER_MERGE*superblock_size bits and more in left sibling  -> We can merge (no problem)
           - there are more bits than in left sibling (and there are more than (MAX_AFTER_MERGE*superblock_size bits) -> We have to move bits from right sibling to current node (a kind of merge & split).
           */
        if (nbPositionsRight
                && (merge_n_split = (x->myPositions + nbPositionsRight >= MAX_AFTER_MERGE * superblock_size
                        && (nbPositionsRight > x->myPositions)
                        && ((nbPositionsRight - x->myPositions) >> 1) >= MIN_EXCHANGE * superblock_size
                        && ((nbPositionsRight + x->myPositions) <= (2 * superblock_size * MAX_AFTER_MERGE))
                        && (nbPositionsRight <= nbPositionsLeft || !nbPositionsLeft))
                    || (merge = (nbPositionsRight + x->myPositions < MAX_AFTER_MERGE * superblock_size
                            && (nbPositionsLeft >= nbPositionsRight || !nbPositionsLeft)))))
        {
            // Merging
            if (merge)
            {
                //shift the bits in the right sibling
                array_copyright(rightSibling->blocks, rightSibling->blocks, x->myPositions, rightSibling->myPositions);
                // put all the bits from the current node in the right sibling
                array_copyright(rightSibling->blocks, x->blocks, 0, x->myPositions);
                rightSibling->myPositions += x->myPositions;
                rightSibling->myRank += x->myRank;

                updateCountersOnPathToRoot(rightSibling);
                rbDelete(x, callUpdateCountersOnPathToRoot);
                delete x;
            }
            else
            {
                // `` Merge and split ''
                // We get bits from the right sibling and put them in the current node
                uint64_t gain = (rightSibling->myPositions - x->myPositions) >> 1;
                rank = array_copyright(x->blocks, rightSibling->blocks, x->myPositions, gain);
                // Shift the bits in right sibling to discard the ones we just
                // copied in x.
                array_copyleft(rightSibling->blocks, rightSibling->blocks, gain, rightSibling->myPositions - gain);

                x->myPositions += gain;
                x->myRank += rank;
                rightSibling->myPositions -= gain;
                rightSibling->myRank -= rank;
                updateCountersOnPathToRoot(x);
                updateCountersOnPathToRoot(rightSibling);
            }
        }
        else
        {
            // We did not do anything with the right sibling. Maybe it does not
            // exist or the left sibling is a better choice for merging.
            if (nbPositionsLeft
                    && (merge_n_split = (x->myPositions + nbPositionsLeft >= MAX_AFTER_MERGE * superblock_size
                            && nbPositionsLeft > x->myPositions
                            && (nbPositionsLeft - x->myPositions) >> 1 >= MIN_EXCHANGE * superblock_size
                            && nbPositionsLeft + x->myPositions <= 2 * superblock_size * MAX_AFTER_MERGE)
                        || (merge = nbPositionsLeft + x->myPositions < MAX_AFTER_MERGE * superblock_size)))
            {
                // Merging
                if (merge)
                {
                    // put all the bits from the current node in the left sibling
                    array_copyright(leftSibling->blocks, x->blocks, leftSibling->myPositions, x->myPositions);
                    leftSibling->myPositions += x->myPositions;
                    leftSibling->myRank += x->myRank;

                    updateCountersOnPathToRoot(leftSibling);
                    rbDelete(x, callUpdateCountersOnPathToRoot);
                    delete x;
                }
                else
                {
                    // `` Merge and split ''

                    // nb of bits we get from the left sibling: (left-x)/2
                    uint64_t gain = (nbPositionsLeft - x->myPositions) >> 1;
                    // Shift the bits in the current node, so that we have enough space
                    // to copy the bits from the left sibling.

                    array_copyright(x->blocks, x->blocks, gain, x->myPositions);

                    // We get bits from the left sibling and put them in the current node
                    rank = array_copyleft(x->blocks, leftSibling->blocks, nbPositionsLeft - gain, gain);

                    x->myPositions += gain;
                    x->myRank += rank;
                    leftSibling->myPositions -= gain;
                    leftSibling->myRank -= rank;
                    updateCountersOnPathToRoot(x);
                    updateCountersOnPathToRoot(leftSibling);
                }
            }
            else
            {
                // We did not merge anything although we have a only 'few' bits in
                // the current node: we (probably) have only one node and thus
                // we cannot do anything else!
            }
        }
    }
}

bool SBVTree::getLastDeletedBit()
{
    return lastDeletedBit;
}

size_t SBVTree::getLastDeletedRank()
{
    return lastDeletedRank;
}

void SBVTree::displayBits()
{
    size_t length = getLength();
    for (size_t t = 1; t <= length; ++t)
    {
        if ((*this)[t])
            std::cout << "1";
        else
            std::cout << "0";
    }
    std::cout << std::endl;
}

uint_block *SBVTree::getBits()
{
    size_t len = getLength();
    uint64_t W = sizeof(uint_block) * 8;
    uint64_t bitsLength = len / W + (len % W > 0);
    uint_block *bits = new uint_block[bitsLength];

    SBVNode *n = (SBVNode *)treeMinimum(root);
    uint64_t current_pos = 0;
    while (n != getNil())
    {
        array_copyright(bits, n->blocks, current_pos, n->myPositions);
        current_pos += n->myPositions;
        n = (SBVNode *)treeSuccessor(n);
    }

    return bits;
}

void SBVTree::appendBit(bool bit)
{
    size_t pos = 1;
    if (root != getNil()) pos = getRoot()->subtreePositions + 1;

    insertBit(bit, pos);
}

void SBVTree::setBit(size_t i)
{
    uint64_t infos[2] = {0, 0};
    SBVNode *x = getNodeWithIthBit(i, infos);
    i -= infos[POSITIONS] + 1;
    if (!(x->blocks[i >> log_bs] & 1 << (i & (block_size - 1))))
    {
        // bit at pos. i
        ++x->myRank;
        ++x->subtreeRank;
        updateCountersOnPathToRoot(x);
    }
    x->blocks[i >> log_bs] |= 1 << (i & (block_size - 1));
}

void SBVTree::unsetBit(size_t i)
{
    uint64_t infos[2] = {0, 0};
    SBVNode *x = getNodeWithIthBit(i, infos);
    i -= infos[POSITIONS] + 1;
    if (x->blocks[i >> log_bs] & 1 << (i & (block_size - 1)))
    {
        // bit at pos. i
        --x->myRank;
        --x->subtreeRank;
        updateCountersOnPathToRoot(x);
    }
    x->blocks[i >> log_bs] &= BS_MAX ^ (1 << (i & (block_size - 1)));
}

void SBVTree::changeBit(size_t i, bool bit)
{
    if (bit)
        setBit(i);
    else
        unsetBit(i);
}

void SBVTree::insertBit(bool bit, size_t i)
{
    SBVNode *x;
    uint64_t infos[2] = {0, 0};
    if (getRoot() == getNil())
    {
        SBVNode *newNode = new SBVNode(getNil());
        newNode->color = dfmi::RBNode::BLACK;
        setRoot(newNode);
        x = newNode;
    }
    else
    {
        x = getNodeWithIthBit(i, infos);
    }

    i -= infos[POSITIONS] + 1;

    bool new_right = false, new_left = false;

    SBVNode *new_node;

    // If the node is already full we can't insert a bit yet.
    if (x->myPositions == superblock_size)
    {
        SBVNode *rightSibling = (SBVNode *)treeSuccessor(x);
        SBVNode *leftSibling = (SBVNode *)treePredecessor(x);

        uint64_t nbPositionsRight = 0, nbPositionsLeft = 0;
        uint64_t rank;
        uint64_t nb_shift;

        if (rightSibling != getNil())
            nbPositionsRight = rightSibling->myPositions;
        if (leftSibling != getNil())
            nbPositionsLeft = leftSibling->myPositions;

        bool merge_right = nbPositionsRight
            && (nbPositionsRight < nbPositionsLeft || !nbPositionsLeft)
            && (superblock_size + nbPositionsRight) / 2 <= MAX_AFTER_MERGE * superblock_size;
        bool merge_left = !merge_right && nbPositionsLeft
            && (superblock_size + nbPositionsLeft) / 2 <= MAX_AFTER_MERGE * superblock_size;

        if (!merge_right && !merge_left)
        {
            // Split the node

            // Creates the new node.
            new_node = new SBVNode(getNil());
            //find place for new node:
            SBVNode *y;// some parent of new node
            if (x->getRight() != getNil())
            {
                y = (SBVNode *)treeMinimum(x->getRight());
                y->setLeft(new_node);
            }
            else
            {
                y = x;
                y->setRight(new_node);
            }
            rightSibling = new_node;
            new_right = true;
            nbPositionsRight = 0;
            new_node->setParent(y);
        }
        if (merge_right || new_right)
        {
            // We have to shift some bits from the current node to the right sibling
            nb_shift = (superblock_size - nbPositionsRight) / 2;
            array_copyright(rightSibling->blocks, rightSibling->blocks, nb_shift, rightSibling->myPositions);
            rank = array_copyleft(rightSibling->blocks, x->blocks, superblock_size - nb_shift, nb_shift);

//          std::cout << " Put " << nb_shift << " bits in right sibling. RS had " << rightSibling->myPositions << " (now " << rightSibling->myPositions + nb_shift << "), current node had " << x->myPositions << " (now " << x->myPositions - nb_shift << ")" << std::endl;
            x->myPositions -= nb_shift;
            rightSibling->myPositions += nb_shift;
            x->myRank -= rank;
            rightSibling->myRank += rank;

            updateCountersOnPathToRoot(x);
            updateCountersOnPathToRoot(rightSibling);

            if (i >= superblock_size - nb_shift)
            {
                // The position of the bit to insert is now in the next node
                x = rightSibling;
                i -= superblock_size - nb_shift;
            }
        }
        else if (merge_left || new_left)
        {
            // We have to shift some bits from the current node to the left sibling
            nb_shift = (superblock_size - nbPositionsLeft) / 2;
            rank = array_copyright(leftSibling->blocks, x->blocks, leftSibling->myPositions, nb_shift);
            array_copyleft(x->blocks, x->blocks, nb_shift, x->myPositions - nb_shift);
//          std::cout << " Put " << nb_shift << " bits in left sibling. RS had " << leftSibling->myPositions << " (now " << leftSibling->myPositions + nb_shift << "), current node had " << x->myPositions << " (now " << x->myPositions - nb_shift << ")" << std::endl;
            x->myPositions -= nb_shift;
            x->myRank -= rank;

            updateCountersOnPathToRoot(x);

            if (i < nb_shift)
            {
                x = leftSibling;
                i += leftSibling->myPositions;
            }
            else
            {
                i -= nb_shift;
            }

            leftSibling->myPositions += nb_shift;
            leftSibling->myRank += rank;

            updateCountersOnPathToRoot(leftSibling);
        }
    }

    uint64_t current_block = i >> log_bs;
    uint64_t current_position = i & (block_size - 1);

    // We have to shift by one position everything which is beyond the insertion position
    uint_block mask_last_bit = 1 << (block_size - 1);

    bool overflow_bit = x->blocks[current_block] & mask_last_bit, overflow_bit2;
    uint_block mask;
    if (!current_position)
        mask = BS_MAX;
    else
        mask = BS_MAX ^ mask_positions[current_position - 1];

    uint_block shifted = ((x->blocks[current_block]) & mask) << 1 | bit << current_position; // bit is 'inserted' by the way
    if (current_position > 0)
        x->blocks[current_block] = (x->blocks[current_block] & mask_positions[current_position - 1]) | shifted;
    else
        x->blocks[current_block] = shifted;

    ++x->myPositions;
    if (bit) ++x->myRank;
    // Then, between blocks
    size_t nb_blocks = (x->myPositions >> log_bs);
    size_t remainder = (x->myPositions & (block_size - 1));
    if (remainder > 0)
        ++nb_blocks;
    ++current_block;
    while (current_block < nb_blocks)
    {
        overflow_bit2 = x->blocks[current_block] & mask_last_bit;
        // We shift the block and put
        // the last bit of the previous block at the first position of this block
        x->blocks[current_block] = (x->blocks[current_block] << 1) | overflow_bit;
        ++current_block;
        overflow_bit = overflow_bit2;
    }

    updateCountersOnPathToRoot(x);
    if (new_left || new_right)
        rbInsertFixup(new_node, callUpdateCounters);
}

void SBVTree::checkSubTree(SBVNode *n)
{
    size_t lP = 0;
    size_t lR = 0;
    size_t rP = 0;
    size_t rR = 0;

    if (n->getLeft() != getNil())
    {
        lP = n->getLeft()->subtreePositions;
        lR = n->getLeft()->subtreeRank;
        if (n->getLeft()->getParent() != n)
        {
            std::cout << "au" << std::endl;
            exit(1);
        }
    }

    if (n->getRight() != getNil())
    {
        rP = n->getRight()->subtreePositions;
        rR = n->getRight()->subtreeRank;
        if (n->getRight()->getParent() != n)
        {
            std::cout << "au" << std::endl;
            exit(1);
        }
    }

    if ((n->subtreePositions != (n->myPositions + lP + rP)) ||
            (n->subtreeRank != (n->myRank + lR + rR)))
    {
        std::cout << "checkSubTree: error" << std::endl;
        std::cout << "lP: " << lP << std::endl;
        std::cout << "lR: " << lR << std::endl;
        std::cout << "rP: " << rP << std::endl;
        std::cout << "rR: " << rR << std::endl;
        std::cout << "n->myPositions + lP + rP: " << n->myPositions + lP + rP << std::endl;
        std::cout << "n->myRank + lR + rR: " << n->myRank + lR + rR << std::endl;
        printNode(n);
        printNode(n->getLeft());
        printNode(n->getRight());
        exit(1);
    }

    if (n->getLeft() != getNil()) checkSubTree(n->getLeft());
    if (n->getRight() != getNil()) checkSubTree(n->getRight());
}

void callUpdateCounters(RBNode *n, RBTree *T)
{
    ((SBVTree *)T)->updateCounters((SBVNode *)n);
}

void callUpdateCountersOnPathToRoot(RBNode *n, RBTree *T)
{
    ((SBVTree *)T)->updateCountersOnPathToRoot((SBVNode *)n);
}

} // namespace
