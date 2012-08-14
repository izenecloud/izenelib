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

#ifndef DFMI_LCP_H
#define DFMI_LCP_H

#include "intTree.h"
#include "sbvtree.h"
#include "utils.h"


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dfmi
{

class IntTree;
class SBVTree;

template <class CharT>
class DynSA;

template <class CharT>
class LCP
{
public:
    typedef CharT char_type;
    typedef DynSA<CharT> dsa_type;
    typedef LCP<CharT> self_type;

    LCP(dsa_type *sa, bool use_it = true);
    ~LCP();

    uint64_t getLCP(uint64_t i);
    uint64_t getSize();

    /**
     * Tells that we have a new element at position pos
     */
    void newSuffixAtPos(uint64_t pos);
    /**
     * Tells that we have deleted an element at position pos.
     */
    void deleteSuffixAtPos(uint64_t pos);
    /**
     * Updates all the values (the ones marked in the bit vectors as well as
     * the ones that have not been marked during the reordering step).
     * The two parameters tell where we stopped the reordering.
     */
    void update(uint64_t last_cs, uint64_t order);

private:
    size_t updateValue(uint64_t pos, uint64_t next_cs);

    // Computes the LCP value by retrieving two factors
    // The LCP value is bounded by lower_bound and the upper_bound
    uint64_t LCPRetrieveFactors(uint64_t pos, uint64_t pos2, uint64_t upper_bound, uint64_t lower_bound);

    bool updateNextLCPValues(size_t pos, size_t previous_pos, int variance, int variance_ref, size_t lower_bound);

private:
    IntTree *lcp;               /* a tree that stores the values of LCP */
    SBVTree *to_update;         /* positions that have to be updated */
    SBVTree *bv_updates;        /* mask for the values that are stored */
    SBVTree *deletions;         /* marks the deletions in the LCP array */
    IntTree *stored_values;     /* stored values for remembering the old values, before update */
    dsa_type *dsa;              /* A reference to the DSA this structure corresponds to. */
};

template <class CharT>
LCP<CharT>::LCP(dsa_type *sa, bool use_it)
    : dsa(sa)
{
    if (use_it)
    {
        lcp = new IntTree();
        to_update = new SBVTree();
        bv_updates = new SBVTree();
        deletions = new SBVTree();
        stored_values = new IntTree();
    }
    else
    {
        lcp = NULL;
    }
}

template <class CharT>
LCP<CharT>::~LCP()
{
    if (lcp)
    {
        delete lcp;
        delete to_update;
        delete bv_updates;
        delete stored_values;
        delete deletions;
    }
}

template <class CharT>
uint64_t LCP<CharT>::getLCP(uint64_t i)
{
    if (!lcp) return 0;
    return lcp->getElement(i);
}

template <class CharT>
uint64_t LCP<CharT>::getSize()
{
    if (!lcp) return 0;
    return lcp->getSize();
}

template <class CharT>
void LCP<CharT>::newSuffixAtPos(uint64_t pos)
{
    if (!lcp) return;

    if (!getSize())
    {
        // Just initialising
        to_update->insertBit(false, pos);
        bv_updates->insertBit(false, pos);
        deletions->insertBit(false, pos);
        lcp->insertElement(0, pos);
        return;
    }

    // A new suffix has been inserted.
    // The value at position pos - 1 has also to be updated.
    if (pos > 2 && !(*to_update)[pos - 1])
    {
        to_update->setBit(pos - 1);
        bv_updates->setBit(pos - 1);
        // We store the old value
        stored_values->insertElement(getLCP(pos - 1), bv_updates->rank(true, pos - 1));
    }
    to_update->insertBit(true, pos);
    bv_updates->insertBit(false, pos);
    if (pos > getSize())
        deletions->insertBit(false, deletions->getLength() + 1);
    else
        deletions->insertBit(false, deletions->select0(pos));
    lcp->insertElement(0, pos);
}

template <class CharT>
void LCP<CharT>::deleteSuffixAtPos(uint64_t pos)
{
    if (!lcp) return;
    // If we delete where we stored a value, we have to delete the value as well
    if ((*bv_updates)[pos])
        stored_values->deleteElement(bv_updates->rank(true, pos));
    bv_updates->deleteBit(pos);
    to_update->deleteBit(pos);
    size_t pos_delet = deletions->select0(pos);
    deletions->setBit(pos_delet);
    // We have to update at the position before the deletion.
    if (pos > 2)
    {
        to_update->setBit(pos - 1);
    }
    lcp->deleteElement(pos);
}

template <class CharT>
void LCP<CharT>::update(uint64_t last_cs, uint64_t order)
{
    if (!lcp) return;

    uint64_t pos, next_cs, nb_iter;
    // we update all the stuff we have to update.
    while (to_update->rank(true, to_update->getLength()))
    {
        pos = to_update->select(true, 1);
        next_cs = dsa->invLF(pos);
        nb_iter = 1;
        while ((*to_update)[next_cs])
        {
            pos = next_cs;
            next_cs = dsa->invLF(pos);
            ++nb_iter;
        }
        while (nb_iter > 0)
        {
            updateValue(pos, next_cs);
            next_cs = pos;
            --nb_iter;
            pos = dsa->LFmapping(pos);
        }
        while ((*to_update)[pos])
        {
            updateValue(pos, next_cs);
            next_cs = pos;
            pos = dsa->LFmapping(pos);
        }
    }

    // Now we care about the values that have not been marked.
    // That is the values for cyclic shifts of order greater than 'order'.
    size_t new_pos = last_cs, old_pos = dsa->invLF(last_cs);
    bool has_to_update = true;
    while (has_to_update && new_pos != 1)
    {
        has_to_update = updateNextLCPValues(new_pos, old_pos, -1, -1, order);
        has_to_update = updateNextLCPValues(new_pos, old_pos, 1, 0, order) || has_to_update ;
        ++order;
        old_pos = new_pos;
        new_pos = dsa->LFmapping(new_pos);
    }

    // Deletes all the values that have been stored and reset the bv_updates bit
    // vector.
    while (bv_updates->rank(true, getSize()))
    {
        pos = bv_updates->select(true, 1);
        bv_updates->unsetBit(pos);
        stored_values->deleteElement(1);
    }

    // reset the deletions bit vector.
    while (deletions->rank(true, deletions->getLength()))
    {
        deletions->deleteBit(deletions->select1(1));
    }
}

// PRIVATE

// Update the value at position pos.
template <class CharT>
size_t LCP<CharT>::updateValue(uint64_t pos, uint64_t next_cs)
{
    // Gets the next cyclic shift
//  uint64_t next_cs = dsa->invLF(pos);
    uint64_t upper_bound;

//  // But before, we recompute its LCP value if needed
//  if ((*to_update)[next_cs])
//    upper_bound = updateValue(next_cs) + 1;
//  else
    // This is a known upper bound for the LCP value.
    upper_bound = getLCP(next_cs) + 1;

    uint64_t lcp_value;
    bool lcp_defined = false;
    uint64_t size = dsa->getSize();

    // If we have a repetition (LF(i + 1) == LF(i) + 1)
    // We take profit from this property and we deduce the new LCP value
    // from that property.
    if (pos < size && next_cs < size
            && dsa->LFmapping(next_cs + 1) == pos + 1)
    {
        // Therefore the value is either upper_bound or 0.
        if ((*dsa)[next_cs] == (*dsa)[next_cs + 1])
            lcp_value = upper_bound;
        else
            lcp_value = 0;
#ifdef DBG
        cout << "upper_bound used" << endl;
#endif
        lcp_defined = true;
    }
    else if (pos == size)
    {
        lcp_value = 0;
        lcp_defined = true;
    }

    uint64_t lower_bound = 0;
    // If we didn't define the lcp value yet ... still have some work !
    if (!lcp_defined)
    {
        uint64_t curr_pos = deletions->select0(pos);
        bool deletion_before = false, deletion_after = false;
        if (curr_pos + 1 <= deletions->getLength())
            deletion_after = (*deletions)[curr_pos + 1];
        if (curr_pos - 1 > 0)
            deletion_before = (*deletions)[curr_pos - 1];

        if (!deletion_before && !deletion_after)
        {
            // Using the old values we can get a lower bound
            if ((*bv_updates)[pos])
                lower_bound = stored_values->getElement(bv_updates->rank1(pos));
            else if ((*bv_updates)[pos - 1])
                lower_bound = stored_values->getElement(bv_updates->rank1(pos - 1));
            else if ((*to_update)[pos - 1])
                lower_bound = getLCP(pos - 1);
        }
    }

    // Using our bounds (lower and upper) we get the factors
    // and compute their LCPs
    if (!lcp_defined)
    {
        lcp_value = LCPRetrieveFactors(pos, pos + 1, upper_bound, lower_bound);
        lcp_defined = true;
    }

//  cout << "Updating at position " << pos << ", new value is: " << lcp_value << endl;
//  uint64_t sa = dsa->getSA(pos);
//  uint64_t length_lcp = size - sa < lcp_value + 2 ? (size - sa) : lcp_value + 2;
//  cout << dsa->retrieveTextFactor(sa, length_lcp) << " (" << sa << ")" << endl;
//  if (pos < size)
//  {
//    sa = dsa->getSA(pos + 1);
//    length_lcp = size - sa < lcp_value + 2 ? (size - sa) : lcp_value + 2;
//    cout << dsa->retrieveTextFactor(sa, length_lcp) << " (" << sa << ")" << endl << endl;
//  }
    lcp->updateElement(pos, lcp_value);
    to_update->unsetBit(pos);
    //  to_update->insertBit(false, pos);
    return lcp_value;
}

// Computes the LCP value by retrieving two factors
// The LCP value is bounded by lower_bound and the LCP value +1 at previous_pos
template <class CharT>
uint64_t LCP<CharT>::LCPRetrieveFactors(uint64_t pos, uint64_t pos2, uint64_t upper_bound, uint64_t lower_bound)
{
    uint64_t SA_k, SA_k2, size = dsa->getSize();
    if (pos > size || pos2 > size)
        return 0;
    if (upper_bound <= lower_bound)
    {
        return lower_bound;
    }
    SA_k = dsa->getSA(pos);
    SA_k2 = dsa->getSA(pos2);

    if (SA_k + upper_bound > size)
    {
        upper_bound = size - SA_k;
        if (SA_k + lower_bound > size)
        {
            return size - SA_k;
        }
    }
    if (SA_k2 + upper_bound > size)
    {
        upper_bound = size - SA_k2;
        if (SA_k2 + lower_bound > size)
        {
            return size - SA_k2;
        }
    }

    char_type *factor1 = dsa->retrieveTextFactor(SA_k + lower_bound, upper_bound - lower_bound);
    char_type *factor2 = dsa->retrieveTextFactor(SA_k2 + lower_bound, upper_bound - lower_bound);

    uint64_t i = 0;
    while (i < upper_bound - lower_bound)
    {
        if (factor1[i] == factor2[i]) ++i;
        else break;
    }
    delete factor1;
    delete factor2;
    return i + lower_bound;
}

template <class CharT>
bool LCP<CharT>::updateNextLCPValues(size_t pos, size_t previous_pos, int variance, int variance_ref, size_t lower_bound)
{
    size_t old_LCP = getLCP(pos + variance_ref);
    if (old_LCP < lower_bound)
        return false;
    size_t new_LCP;
    bool updated = false;
    size_t upper_bound = getLCP(previous_pos + variance_ref) + 1;
    uint64_t size = dsa->getSize();
    if (pos + variance == 0 || pos + variance > size)
        return updated;
    // We use the property over LF
    if (previous_pos + variance > 0 && previous_pos + variance < size
            && (pos + variance == dsa->LFmapping(previous_pos + variance)))
    {
        if ((*dsa)[previous_pos] == (*dsa)[previous_pos + variance])
            new_LCP = upper_bound;
        else
            new_LCP = 0;
    }
    else
    {
        // Or we retrieve factors
        new_LCP = LCPRetrieveFactors(pos, pos + variance, upper_bound, lower_bound);
    }
    if (new_LCP != old_LCP)
    {
        lcp->updateElement(pos + variance_ref, new_LCP);
        updated = true;
    }
    return updated;
}

}
}

NS_IZENELIB_AM_END

#endif
