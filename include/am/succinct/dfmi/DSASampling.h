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

/*
 * This is the implementation of the method for sampling a SA and an ISA
 * described in:
 * Dynamic Extended Suffix Arrays.
 * M. Salson and T. Lecroq and M. L\'eonard and L. Mouchard
 * Journal of Discrete Algorithms
 * To appear
 */


#ifndef DFMI_DSA_SAMPLING_H
#define DFMI_DSA_SAMPLING_H

#include "sbvtree.h"
#include "lpermutation.h"

#include <iostream>

namespace dfmi
{

template <class CharT>
class DynSA;

/**
 * DSASampling is a class that manages the sampling for a Dynamic Suffix Array
 * It mainly consists in two bit vectors that mark positions and values sampled
 * in ISA as well as a permutation for mapping a position to its value.
 */
template <class CharT>
class DSASampling
{
public:
    typedef CharT char_type;
    typedef DynSA<CharT> dsa_type;
    typedef DSASampling self_type;

    /* max sampling is calculated to be at  3.5*min */
    DSASampling(dsa_type *dynSA, uint64_t min = 20);
    ~DSASampling();

    /**
     * returns the length of the sampling
     */
    uint64_t getSize();

    /**
     * returns the value of ISA[index]
     */
    uint64_t getISA(uint64_t index);

    /**
     * returns the value of SA[index]
     */
    uint64_t getSA(uint64_t index);

    /**
     * If pos_bwt != 0:
     *   Warns that we have inserted an element in the text at position pos_text
     *   and that it was inserted in the BWT at the position pos_bwt.
     * If pos_bwt == 0, we check if the sample has to be updated at pos_text
     *   (new sample(s) may be added).
     *
     * This strategy allows to check the need for an update after having
     * inserted everything in the BWT.
     */
    void insertBWT(uint64_t pos_text, uint64_t pos_bwt = 0);

    /**
     * If pos_bwt != 0:
     *  Warns that we have deleted an element in the text at position pos_text.
     *  This element was also deleted at position pos_bwt in the BWT.
     * If pos_bwt == 0, we check if the sample has to be updated at pos_text
     *   (sample(s) may be deleted).
     */
    void deleteBWT(uint64_t pos_text, uint64_t pos_bwt = 0);

    /**
     * Tells us that the element at position original_bwt_pos is moved
     * to the position new_bwt_pos.
     * We update accordingly the structure but the density of the samples
     * is not affected by this modification.
     */
    void moveBWT(uint64_t original_bwt_pos, uint64_t new_bwt_pos);

private:
    dsa_type *dSA;			/* the dynamic SA sampling is linked to */
    uint64_t min_sampling_d;		/* Min distance between two sampled positions */
    uint64_t max_sampling_d;		/* Max distance between two sampled positions */
    SBVTree *ISApositions;		/* positions (of ISA) that are sampled */
    SBVTree *ISAvalues;		/* sampled values of ISA */
    LPermutation *pi;		/* mapping between positions and values */
    uint64_t size;

    void addSample(uint64_t pos, uint64_t value, bool replaces = false);
    void deleteSample(uint64_t pos);
};

template <class CharT>
DSASampling<CharT>::DSASampling(dsa_type *dynSA, uint64_t min)
    : dSA(dynSA)
    , min_sampling_d(min)
    , max_sampling_d(7 * min / 2)
    , size(0)
{
    ISApositions = new SBVTree();
    ISAvalues = new SBVTree();
    pi = new LPermutation();
}

template <class CharT>
DSASampling<CharT>::~DSASampling()
{
#ifndef NFREE_ISA
    delete ISApositions;
    delete ISAvalues;
#endif
#ifndef NFREE_PERMUTATION
    delete pi;
#endif
}

template <class CharT>
uint64_t DSASampling<CharT>::getSize()
{
    return size;
}

template <class CharT>
uint64_t DSASampling<CharT>::getISA(uint64_t index)
{
    size_t nb_sampled = ISApositions->rank(true, index);
    size_t bwt_pos;
    if ((*ISApositions)[index])
    {
        return ISAvalues->select(true, pi->getIthElement(nb_sampled));
    }
    ++nb_sampled;
    size_t shift_pos = ISApositions->select(true, nb_sampled) - index;
    bwt_pos = ISAvalues->select(true, pi->getIthElement(nb_sampled));
    for (size_t i = 0; i < shift_pos; ++i)
    {
        bwt_pos = dSA->LFmapping(bwt_pos);
    }
    return bwt_pos;
}

template <class CharT>
uint64_t DSASampling<CharT>::getSA(uint64_t index)
{
    size_t shift = 0;
    while (!(*ISAvalues)[index])
    {
        index = dSA->LFmapping(index);
        ++shift;
    }
    uint64_t result = ISApositions->select(true, pi->getIthInvElement(ISAvalues->rank(true, index))) + shift;
    if (result > getSize())
        return result % getSize();
    return result;
}

template <class CharT>
void DSASampling<CharT>::insertBWT(uint64_t pos_text, uint64_t pos_bwt)
{
    uint64_t rank, pos_right_one, pos_left_one;
    if (pos_bwt!=0)
    {
        // Insert false bits for the moment, we'll check after the whole insertion
        // in the BWT if we need to add samples.
        ISApositions->insertBit(false, pos_text);
        ISAvalues->insertBit(false, pos_bwt);
        ++size;
        return ;
    }
    if (size == 1)
    {
        ISApositions->deleteBit(1);
        ISAvalues->deleteBit(1);
        ISApositions->insertBit(true, 1);
        ISAvalues->insertBit(true, 1);
        pi->insert(1, 1);
        return ;
    }
    // Testing the positions
    if (pos_text == 1)
        rank = 0;
    else
        rank = ISApositions->rank(true, pos_text - 1);

    pos_right_one = ISApositions->select(true, rank + 1);
    if (!rank)
        pos_left_one = 1;
    else
        pos_left_one = ISApositions->select(true, rank);

    if (pos_right_one - pos_left_one > max_sampling_d)
    {
        // We need to add some samplings
        uint64_t nb_pieces = 2 * (pos_right_one - pos_left_one + 1) / (max_sampling_d + min_sampling_d) - 1;
        for (uint64_t k = 0; k < nb_pieces; ++k)
        {
            uint64_t sample_pos = pos_right_one - (k + 1) * (min_sampling_d + max_sampling_d) / 2;
            uint64_t isa;
            if (size > 0)
                isa = getISA(sample_pos);
            else
                isa = 1;
            addSample(sample_pos, isa, true);
        }
    }
}

template <class CharT>
void DSASampling<CharT>::deleteBWT(uint64_t pos_text, uint64_t pos_bwt)
{
    uint64_t rank, pos_right_one, pos_left_one;

    // Just deleting the elements without considering min and max distances.
    if (pos_bwt)
    {
        if ((*ISApositions)[pos_text])
        {
            uint64_t rank = ISApositions->rank(true, pos_text);
            pi->deleteElem(pi->getIthElement(rank));
        }
        // Finally deleting the  bit
        ISApositions->deleteBit(pos_text);
        ISAvalues->deleteBit(pos_bwt);
        --size;

        return ;
    }

    if (pos_text == 1)
        rank = 0;
    else
        rank = ISApositions->rank(true, pos_text - 1);

    if (!rank) return;

    pos_right_one = ISApositions->select(true, rank + 1);
    pos_left_one = ISApositions->select(true, rank);

    if (pos_left_one - pos_right_one < min_sampling_d)
    {
        // We have to delete a sample (and maybe to add another one)
        if (rank == 1)
        {
            deleteSample(pos_left_one);
            if (pos_left_one > (max_sampling_d - min_sampling_d) / 2)
            {
                pos_left_one -= (max_sampling_d - min_sampling_d) / 2;
                uint64_t value = getISA(pos_left_one);
                addSample(pos_left_one, value, true);
            }
        }
        else
        {
            uint64_t pos_left_left_one = ISApositions->select(true, rank - 1);
            uint64_t new_pos;
            if (pos_left_one - pos_left_left_one > (min_sampling_d + max_sampling_d) / 2)
            {
                // The distance between the left and left left bit is sufficient to
                // move the left bit.
                new_pos = pos_left_one + min_sampling_d / 2 - (pos_left_one - pos_left_left_one) / 2;
                deleteSample(pos_left_one);
                uint64_t value = getISA(new_pos);
                addSample(new_pos, value, true);
            }
            else
            {
                // Distance between left and left left bit is not sufficient.
                // Therefore we delete the left bit.
                deleteSample(pos_left_one);
            }
        }
    }
}

template <class CharT>
void DSASampling<CharT>::moveBWT(uint64_t original_bwt_pos, uint64_t new_bwt_pos)
{
    uint64_t original_rank, new_rank;
    uint64_t bit = (*ISAvalues)[original_bwt_pos];
    if (bit)
        original_rank = ISAvalues->rank(true, original_bwt_pos);
    // Moves the bit
    ISAvalues->deleteBit(original_bwt_pos);
    ISAvalues->insertBit(bit, new_bwt_pos);
    if (bit)
    {
        new_rank = ISAvalues->rank(true, new_bwt_pos);
        // If the rank has been affected by the move, we need to update the
        // permutation as well
        if (original_rank != new_rank)
            pi->update(original_rank, new_rank);
    }
}

template <class CharT>
void DSASampling<CharT>::addSample(uint64_t pos, uint64_t value, bool replaces)
{
    uint64_t ith_value;
    if (value > 1)
        ith_value = ISAvalues->rank(true, value) + 1;
    else
        ith_value = 1;

    if (replaces)
        ISApositions->deleteBit(pos);
    ISApositions->insertBit(true, pos);
    uint64_t rank = ISApositions->rank(true, pos);

    ISAvalues->deleteBit(value);
    ISAvalues->insertBit(true, value);

    pi->insert(ith_value, rank);
    if (!replaces)
        ++size;
}

template <class CharT>
void DSASampling<CharT>::deleteSample(uint64_t pos)
{
    uint64_t rank = ISApositions->rank(true, pos);
    uint64_t ith_value = pi->getIthElement(rank);

    ISAvalues->deleteBit(ith_value);
    ISAvalues->insertBit(false, ith_value);
    pi->deleteElem(rank);
    ISApositions->deleteBit(rank);
    ISApositions->insertBit(false, rank);
}

}

#endif
