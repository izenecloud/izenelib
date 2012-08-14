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
/* ------ Dynamic Suffix Array -------
 papers : M. Salson, T. Lecroq, M. L\'eonard, L. Mouchard.
          Dynamic Burrows-Wheeler Transform.
          PSC 2008, pp. 13--25, 2008.

          M. Salson, T. Lecroq, M. L\'eonard, L. Mouchard.
          A Four-Stage Algorithm for Updating a Burrows-Wheeler Transform
          Theoretical Computer Science. To appear

          M. Salson, T. Lecroq, M. L\'eonard, L. Mouchard.
          Dynamic Extended Suffix Arrays.
          Journal of Discrete Algorithms. To appear
*/

#ifndef DFMI_DYN_SA_H
#define DFMI_DYN_SA_H

#include "gRankS.h"
#include "lcp.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <cstdlib>

#ifndef SAMPLE
#define SAMPLE 42
#endif

#ifndef BUFFER
#define BUFFER 1048576
#endif

#if SAMPLE != 0
#include "DSASampling.h"
#endif


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dfmi
{

//TODO in Klasse schieben
const size_t sampleInterval = SAMPLE;

template <class CharT = unsigned char>
class DynSA
{
public:
    typedef CharT char_type;
    typedef DynRankS<CharT> wavelet_tree_type;
    typedef LCP<CharT> lcp_type;
#if SAMPLE != 0
    typedef DSASampling<CharT> sampling_type;
#endif
    typedef DynSA<CharT> self_type;

    /* Constants used to know the current state of our structure.
     * Knowing this state is useful since a discrepancy is introduced
     * in the BWT when we are dealing with modifications.
     */
    enum DSA_STAT
    {
        DSA_IDLE = 0,           /* We are not modifying the structure */
        DSA_INSERTING = 1,      /* We are inserting char(s) */
        DSA_DELETING = 2,       /* We are deleting char(s)  */
        DSA_SUBSTITUTING = 4,   /* We are substituting char(s) -- not used (yet?) */
        DSA_REORDERING = 8      /* We are reordering cyclic shifts */
    };

    size_t nb_updates_lcp;

    /**
     * Constructs an empty dyn SA.
     * If the boolean is false we do NOT compute the LCP table.
     */
    DynSA(size_t alphabet_num = (uint64_t)std::numeric_limits<char_type>::max() + 1, bool computeLCP = false);

    void initEmptyDynSA(const float *f); //argument: array containing relative frequencies of characters

    /**
     * Adds the string str of length n at position pos in the original text.
     */
    size_t addChars(char_type *str, size_t n, size_t pos);
    /**
     * Deletes n chars starting at position pos in the text.
     */
    size_t deleteChars(size_t n, size_t pos);

    size_t addTextFromSA(const size_t *SA, char_type *str, size_t n);
    size_t addText(char_type *str, size_t n); // length n of text must include "\0"!
    size_t addTextFromFile(char *filename, size_t n); //data is read directly from disk using only BUFFER(macro) bytes
    char_type *retrieveText();

    /**
     * If T is the text, retrieves T[pos..length+pos-1]
     */
    char_type *retrieveTextFactor(size_t pos, uint64_t length);

    size_t count(const char_type *pattern, size_t n, size_t &m); // only counting, no locate!
    size_t *locate(const char_type *pattern, size_t n, size_t &m); // needs macro SAMPLE > 0! (default)
    // returns array: [length{, position}^length]

    std::pair<size_t, size_t> *backwardSearch(const char_type *pattern, size_t n, size_t &m);
    // returns pair: (lower bound, upper bound)

    //LF(i)-mapping: C[L[i]] + rank_L[i](L, i)
    size_t LFmapping(size_t i);

    /**
     * Inverse of the LF-mapping: LFmapping(invLF(i)) == i
     */
    size_t invLF(size_t i);
    /*     size_t oldLFmapping(size_t i); */

    /**
     * returns ISA[pos]
     */
    size_t getISA(size_t pos);

    /**
     * returns the whole BWT
     */
    char_type *getBWT();
    std::ostream& getBWTStream(std::ostream& stream);

    /**
     * returns LCP[pos]
     */
    uint64_t getLCP(uint64_t pos);
    /**
     * returns SA[pos]
     */
    uint64_t getSA(uint64_t pos);

    /**
     * Returns the current state (DSA_INSERTING, DSA_DELETING, DSA_SUBSTITUTING,
     * DSA_REORDERING, DSA_IDLE)
     */
    int getAction();

    size_t getNumberOfSymbolsSmallerThan(char_type c);
    /**
     * Inverse of getNumberOfSymbolsSmallerThan
     * Returns the rank of C[pos].
     * The letter c is passed by reference so that we can get it in the calling
     * function.
     */
    size_t getRankInF(size_t pos, char_type &c);

    /* Operations on the BWT */

    /**
     * returns L[i]
     */
    char_type operator[](size_t i);

    /**
     * returns rank_c(L, i)
     */
    size_t rank(char_type c, size_t i);

    /**
     * returns select_c(L, i)
     */
    size_t select(char_type c, size_t i);

    /**
     * inserts c in L at position i
     */
    void insert(char_type c, size_t i);

    /**
     * deletes symbol at position i in L
     */
    void deleteSymbol(size_t i);

    /**
     * returns the length of L
     */
    size_t getSize();

    ~DynSA();

public:
    size_t alphabet_num;
    std::vector<size_t> C;

    wavelet_tree_type *L;

    int action;                     /* one of the state DSA_*  */
    char_type letter_substituted;   /* the letter which is replaced in L during stage Ib of our algorithm */
    char_type new_letter_L;         /* the letter we are to insert in L */
    size_t position_mod_bwt;        /* the position where the modificatio takes place in the BWT */
    size_t previous_cs;             /* the position of the previous cyclic shift (meaning the cyclic shift that starts at the position the left) */
    size_t pos_first_modif;         /* position where the first modification in the BWT took place*/
    size_t current_modif;           /* number of modification remaining (not used in deletions)*/

#if SAMPLE != 0
    sampling_type *sample;          /* sample of ISA and SA */
#endif

    lcp_type *lcp;

private:
    /* performs stage Ib */
    size_t substitutionIb(size_t pos, char_type c, size_t &rank);

    /* reorders cyclic shifts until they are all well-ordered.
     * returns the number of cyclic shift reordered. */
    size_t reorderCS();

    // small help functions

    static size_t binaryTree_parent(size_t i)
    {
        return i / 2;
    }

    static size_t binaryTree_leftChild(size_t i)
    {
        return 2 * i;
    }

    static size_t binaryTree_rightChild(size_t i)
    {
        return 2 * i + 1;
    }

    static bool binaryTree_isLeftChild(size_t i)
    {
        return i % 2 == 0;
    }

    static bool binaryTree_isRightChild(size_t i)
    {
        return i % 2 == 1;
    }

    static size_t binaryTree_leftSibling(size_t i)
    {
//      if (!binaryTree_isRightChild(i)) return 0;
        return i - 1;
    }

    static size_t binaryTree_rightSibling(size_t i)
    {
//      if (!binaryTree_isLeftChild(i)) return 0;
        return i + 1;
    }
};

template <class CharT>
DynSA<CharT>::DynSA(size_t alphabet_num, bool computeLCP)
    : alphabet_num(alphabet_num)
    , L(new wavelet_tree_type(alphabet_num))
    , action(DSA_IDLE)
#if SAMPLE != 0
    , sample(new sampling_type(this, sampleInterval))
#endif
    , lcp(new lcp_type(this, computeLCP))
{
}

template <class CharT>
DynSA<CharT>::~DynSA()
{
    delete lcp;
#if SAMPLE != 0
    delete sample;
#endif
#ifndef NFREE_L
    delete L;
#endif
}

template <class CharT>
CharT *DynSA<CharT>::getBWT()
{
    size_t length = L->getSize();
    ++length;
    char_type *text = new char_type[length];
    bool data = true;
    char_type c;
    size_t i = 0;

    L->iterateReset();

    while (data)
    {
        c = L->iterateGetSymbol();
        if (!c)
            text[i] = '$';
        else
            text[i] = c;
        data = L->iterateNext();
        ++i;
    }
    text[length - 1] = 0;

    return text;
}

template <class CharT>
std::ostream& DynSA<CharT>::getBWTStream(std::ostream& stream)
{
    bool data = true;

    L->iterateReset();

    char_type c;
    while (data)
    {
        c = L->iterateGetSymbol();
        if (!c)
            stream << '$';
        else
            stream << c ;
        data = L->iterateNext();
    }

    return stream;
}

template <class CharT>
void DynSA<CharT>::initEmptyDynSA(const float *f)
{
    L->initEmptyDynRankS(f);
    // array C needed for backwards search
    C.resize(alphabet_num * 2);
}

template <class CharT>
void DynSA<CharT>::insert(char_type c, size_t i)
{
    // If we are modifying the BWT we have to take care of the discrepancy
    // introduced.
    if (action & (DSA_INSERTING | DSA_DELETING | DSA_SUBSTITUTING))
    {
        // When we insert before the previous cyclic shift, its position is
        // shifted by one position.
        if (i <= previous_cs)
        {
            ++previous_cs;
        }
        // Same phenomenon for the position of first modification
        if (i <= pos_first_modif)
            ++pos_first_modif;
    }
    L->insert(c, i);

    // Updating the array C
    int j = alphabet_num + c;
    while (j > 1)
    {
        ++C[j];
        j = binaryTree_parent(j);
    }
    ++C[j];
}

template <class CharT>
void DynSA<CharT>::deleteSymbol(size_t i)
{
    char_type c = L->deleteSymbol(i);

    int j = alphabet_num + c;
    while (j > 1)
    {
        --C[j];
        j = binaryTree_parent(j);
    }
    --C[j];

    // In case of modification, we have to update some extra values
    if (action & (DSA_INSERTING | DSA_DELETING | DSA_SUBSTITUTING))
    {
        // When we delete a cyclic shift at position i all the cyclic shifts
        // after pos. i are shifted. We have to modify the position of the
        // previous cyclic shift (and the position of the first modification)
        // corresdpondingly
        if (previous_cs >= i)
            --previous_cs;
        if (pos_first_modif >= i)
            --pos_first_modif;
    }
}

template <class CharT>
CharT DynSA<CharT>::operator[](size_t i)
{
    return (*L)[i];
}

template <class CharT>
size_t DynSA<CharT>::rank(char_type c, size_t i)
{
    size_t result = L->rank(c, i);
    // In case of deletion, rank, after the position of the initial
    // substitution (stage Ib) for the letter deleted, has to be corrected.
    // In practice, the letter has not been substituted yet ... hence we have
    // to correct the result.
    if (action == DSA_DELETING && pos_first_modif < i && letter_substituted == c)
        --result;
    return result;
}

template <class CharT>
size_t DynSA<CharT>::select(char_type c, size_t i)
{
    return L->select(c, i);
}

template <class CharT>
size_t DynSA<CharT>::getSize()
{
    return L->getSize();
}

template <class CharT>
std::pair<size_t, size_t> *DynSA<CharT>::backwardSearch(const char_type *pattern, size_t n, size_t &m)
{
    std::pair<size_t, size_t> *p = new std::pair<size_t, size_t>();
    size_t n_orig = n;

    size_t sp = 1;
    size_t ep = L->getSize();

    char_type s;

    while (n > 0)
    {
        s = pattern[n - 1];

        sp = getNumberOfSymbolsSmallerThan(s) + rank(s, sp - 1UL) + 1UL;
        ep = getNumberOfSymbolsSmallerThan(s) + rank(s, ep);
        if (sp > ep) break;

        p->first = sp;
        p->second = ep;
        --n;
    }

    m = n_orig - n;
    return p;
}

template <class CharT>
size_t DynSA<CharT>::count(const char_type *pattern, size_t n, size_t &m)
{
    std::pair<size_t, size_t> *p = backwardSearch(pattern, n, m);

    size_t result = p->second - p->first + 1;

    delete p;

    if (!m) return 0;
    return result;
}

#if SAMPLE != 0
template <class CharT>
size_t *DynSA<CharT>::locate(const char_type *pattern, size_t n, size_t &m)
{
    std::pair<size_t, size_t> *p = backwardSearch(pattern, n, m);
    size_t sp = p->first, ep = p->second;

    delete p;

    size_t numberOfMatches = ep - sp + 1;
    size_t *matches = new size_t[numberOfMatches + 1];
    matches[0] = numberOfMatches;

    for (size_t i = sp, k = 1; k <= numberOfMatches; ++i, ++k)
    {
        // each match
        matches[k] = sample->getSA(i);
    }

    return matches;
}
#else
template <class CharT>
size_t *DynSA<CharT>::locate(const char_type *pattern, size_t n, size_t &m)
{
    // if SAMPLE is turned off
    return 0;
}
#endif

template <class CharT>
size_t DynSA<CharT>::getISA(size_t pos)
{
#if SAMPLE != 0
    return sample->getISA(pos);
#else
    return 1;
#endif
}

template <class CharT>
size_t DynSA<CharT>::LFmapping(size_t i)
{
    // We are modifying the BWT and requesting LF for the current  position of
    // modification? Easy! That's the value of previous_cs.
    if (action != DSA_IDLE && i == position_mod_bwt)
        return previous_cs;
    char_type s = (*L)[i];
    uint32_t smaller = getNumberOfSymbolsSmallerThan(s), r = rank(s, i);
    // If we are modifying the BWT
    if (action != DSA_IDLE)
    {
        // If we are reordering or we have inserted the last letter
        //  (that is we are going to reorder).
        if (action == DSA_REORDERING || (action == DSA_INSERTING && !current_modif))
        {
            char_type s2 = (*L)[position_mod_bwt];
            size_t tmplf = getNumberOfSymbolsSmallerThan(s2) + rank(s2, position_mod_bwt);
            size_t result = smaller + r;
            // the result is between tmplf and previous_cs.
            // previous_cs has not moved yet hence we have to correct the result
            if (result <= tmplf && result >= previous_cs)
                ++result;
            else if (result >= tmplf && result <= previous_cs)
                --result;
            return result;
        }
        else if (action == DSA_INSERTING)
        {
            char_type s2 = (*L)[position_mod_bwt];
            // Taking into account the substituted letter
            if (s == letter_substituted && i > pos_first_modif)
                ++r;
            // New letter in L has been inserted in F as well, but we have
            // not inserted the c.s. beginning with this letter yet
            if (s > new_letter_L)
                --smaller;
            // This letter must not be taken into account since its previous c.s.
            // doesn't exist yet
            if (s2 == s && i > position_mod_bwt)
                --r;
        }
    }
    return smaller + r;
}

template <class CharT>
size_t DynSA<CharT>::invLF(size_t i)
{
    char_type c;
    i = getRankInF(i, c);
    return select(c, i);
}

template <class CharT>
size_t DynSA<CharT>::addChars(char_type *str, size_t n, size_t pos)
{
    size_t rank_store;
    size_t oldbwt_pos;

    if (pos > getSize())
        pos = getSize();

    // Step Ib in our algorithm : replaces in L, at the position of T^{[i]},
    // the letter with str[n - 1].
    position_mod_bwt = substitutionIb(pos, str[n - 1], rank_store);

    new_letter_L = str[n - 1];
    action = DSA_INSERTING;

    // Computes the position of $T^{[i - 1]}
    previous_cs = getNumberOfSymbolsSmallerThan(letter_substituted) + rank_store;
    if (letter_substituted > new_letter_L)
        --previous_cs;
    oldbwt_pos = position_mod_bwt;

    // Computes the position of insertion
    position_mod_bwt = getNumberOfSymbolsSmallerThan(str[n - 1]) + rank(str[n - 1], oldbwt_pos);

    // Step IIa : insertion of the elements.
    for (size_t k = n - 1; k > 0; --k)
    {
        insert(str[k - 1], position_mod_bwt);
        current_modif = k;
        new_letter_L = str[k - 1];

#if SAMPLE != 0
        sample->insertBWT(pos, position_mod_bwt);
#endif

        if (position_mod_bwt <= oldbwt_pos)
            ++oldbwt_pos;
        lcp->newSuffixAtPos(position_mod_bwt);

        // Updating the values stored with respect to the insertion position.
        oldbwt_pos = position_mod_bwt;

        position_mod_bwt = getNumberOfSymbolsSmallerThan(str[k - 1])
            + rank(str[k - 1], position_mod_bwt);
        // Consider the case where we have computed rank on the missing letter
        if (pos_first_modif < oldbwt_pos && str[k - 1] == letter_substituted)
            ++position_mod_bwt;
    }

    new_letter_L = letter_substituted;
    L->insert(letter_substituted, position_mod_bwt);
    current_modif = 0;

    if (position_mod_bwt <= previous_cs)
    {
        ++previous_cs;
    }
    if (position_mod_bwt <= pos_first_modif)
        ++pos_first_modif;

#if SAMPLE != 0
    sample->insertBWT(pos, position_mod_bwt);
#endif

    if (position_mod_bwt <= oldbwt_pos)
        ++oldbwt_pos;

    lcp->newSuffixAtPos(position_mod_bwt);

    // Step IIb : reordering step
    uint64_t shift = reorderCS();

    // The sample is treated now. So far we have just inserted 0s
    // in the bit vectors. Now we are adding the sample(s) so that
    // the sample respects the min and max distances.
#if SAMPLE != 0
    sample->insertBWT(pos);
#endif
    lcp->update(position_mod_bwt, shift);

    return shift;
}

template <class CharT>
size_t DynSA<CharT>::deleteChars(size_t n, size_t pos)
{
    size_t tmp_rank;
    size_t old_length = getSize();
    char_type current_letter;

    // We don't want to delete after the text nor the last symbol
    if (n + pos - 1 >= old_length)
        n = old_length - pos + 1;

    pos_first_modif = getISA(n + pos);
    letter_substituted = (*L)[pos_first_modif];

    action = DSA_DELETING;
    position_mod_bwt = getNumberOfSymbolsSmallerThan(letter_substituted)
        + rank(letter_substituted, pos_first_modif);

    // Step IIa : deletion of the elements.
    for (size_t k = n - 1; k > 0; --k)
    {
        current_letter = (*L)[position_mod_bwt];
        // Computes the rank before we delete at this position!
        tmp_rank = rank(current_letter, position_mod_bwt);

        deleteSymbol(position_mod_bwt);

        lcp->deleteSuffixAtPos(position_mod_bwt);
#if SAMPLE != 0
        sample->deleteBWT(pos + k, position_mod_bwt);
#endif
        // Next position to delete
        position_mod_bwt = getNumberOfSymbolsSmallerThan(current_letter) + tmp_rank;
    }

    current_letter = (*L)[position_mod_bwt];
    tmp_rank = rank(current_letter, position_mod_bwt);
#ifdef DBG
    cout << "Deleting " << current_letter << " at position " << position_mod_bwt << std::endl;
#endif
    deleteSymbol(position_mod_bwt);

    lcp->deleteSuffixAtPos(position_mod_bwt);
#if SAMPLE != 0
    sample->deleteBWT(pos, position_mod_bwt);
#endif
    previous_cs = tmp_rank + getNumberOfSymbolsSmallerThan(current_letter);

#ifdef DBG
    cout << getBWT() << std::endl;
    cout << "Substitution by " << current_letter << " at position " << pos_first_modif << " (current letter at this position is " << (*L)[pos_first_modif] << ")" << std::endl;
#endif
    // Changes the last letter of the cyclic shift T^{[pos + 1]}
    size_t backup = previous_cs;
    position_mod_bwt = pos_first_modif;
    deleteSymbol(position_mod_bwt);
    new_letter_L = current_letter;
    insert(current_letter, position_mod_bwt);
    // Need a backup in the case we delete all the letters.
    // In that situation previous_cs equals 0, which is impossible!
    previous_cs = backup;

    // Step IIb : reordering step
    size_t shift = reorderCS();

    // The sample is treated now. Until now, we have just deleted
    // elements in the bit vectors.
    // Now we are adding/removing  sample(s) so that
    // DSASampling still respects the min and max distances.
#if SAMPLE != 0
    sample->deleteBWT(pos);
#endif

    lcp->update(position_mod_bwt, shift);

    return shift;
}

template <class CharT>
size_t DynSA<CharT>::addTextFromSA(const size_t *SA,  char_type *str, size_t n)
{
    size_t i, pos;

    for (i = 0; i < n; ++i)
    {
        pos = SA[i];
        if (!pos)
            pos = n - 1;
        else
            --pos;
        insert(str[pos], i + 1);
    }

    return n;
}

// length n of text must include "\0"!
template <class CharT>
size_t DynSA<CharT>::addText(char_type *str, size_t n)
{
    // Initialises the different structures
    insert(str[n - 1], 1);
#if SAMPLE != 0
    sample->insertBWT(1, 1);
    sample->insertBWT(1);
#endif
    lcp->newSuffixAtPos(1);
    addChars(str, n - 1, 1);
    return 1;
}

template <class CharT>
size_t DynSA<CharT>::addTextFromFile(char *filename, size_t n)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs)
    {
        std::cerr << "error reading file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    char c;
    size_t i = 0;
    char_type *text = new char_type[n + 1];

    while (ifs.get(c) && i < n)
    {
        text[i] = c;//(c == '\n' || c == '\r') ? 'X' : c;
        ++i;
    }
    ifs.close();
    text[i] = '\0';
    i = addText(text, i + 1);
    delete[] text;
    return i;
}

template <class CharT>
CharT *DynSA<CharT>::retrieveText()
{
    size_t n = L->getSize();
    if (!n) return NULL;

    char_type *text = new char_type[n]; // last byte 0 for cout

    for (size_t t = n - 1, i = 1; t > 0; --t)
    {
        if (action == DSA_INSERTING && i == position_mod_bwt)
            text[t - 1] = letter_substituted;
        else
            text[t - 1] = (*L)[i];
        i = LFmapping(i);
    }
    text[n - 1] = 0;
    return text;
}

template <class CharT>
size_t DynSA<CharT>::getNumberOfSymbolsSmallerThan(char_type c)
{
    size_t j = alphabet_num + c;
    size_t r = 0;
    while (j > 1)
    {
        if (binaryTree_isRightChild(j))
            r += C[binaryTree_leftSibling(j)];

        j = binaryTree_parent(j);
    }

    // We have not really deleted the "letter_substituted" in L, so it is still
    // taken into account in F as well.
    // The deletion of letter_substituted will only take place at the end of
    // the algorithm. Once we know with which letter we have to substitute it.
    if (action == DSA_DELETING)
    {
        if (c > letter_substituted)
            --r;
    }
    return r;
}

template <class CharT>
size_t DynSA<CharT>::getRankInF(size_t pos, char_type &c)
{
    size_t j = 1, sag;
    while (j < alphabet_num)
    {
        size_t next = binaryTree_leftChild(j);
        sag = C[next];
        if (sag >= pos)
        {
            j = next;
        }
        else
        {
            j = binaryTree_rightSibling(next);
            pos -= sag;
        }
    }
    c = j - alphabet_num;
    return pos;
}

template <class CharT>
size_t DynSA<CharT>::substitutionIb(size_t pos, char_type c, size_t &rank_store)
{
    // Computing the position of the <pos>-th cyclic shift of the text, in the BWT.
    size_t bwt_pos = getISA(pos);

    if (getSize() > 0)
    {
        letter_substituted = (*L)[bwt_pos];
        rank_store = rank(letter_substituted, bwt_pos);

        L->deleteSymbol(bwt_pos);
    }
    else
    {
        rank_store = 0;
    }
    insert(c, bwt_pos);
    pos_first_modif = bwt_pos;
    return bwt_pos;
}

// Uses the value in new_letter, position_mod_bwt and previous_cs
template <class CharT>
size_t DynSA<CharT>::reorderCS()
{
    action = DSA_REORDERING;
    char_type L_store = new_letter_L;
    size_t expected_position = getNumberOfSymbolsSmallerThan(L_store) + rank(L_store, position_mod_bwt);
    size_t smaller;
    size_t shift = 0;

    smaller = getNumberOfSymbolsSmallerThan(L_store);

    while (expected_position != previous_cs)
    {
        ++shift;
        L_store = (*L)[previous_cs];
        smaller = getNumberOfSymbolsSmallerThan(L_store);
        size_t tmp_previous_cs = rank(L_store, previous_cs) + smaller;

        // Updating the LCP
        lcp->deleteSuffixAtPos(previous_cs);
        lcp->newSuffixAtPos(expected_position);

        // Places the letter at the right place
        deleteSymbol(previous_cs);
        insert(L_store, expected_position);

#if SAMPLE != 0
        sample->moveBWT(previous_cs, expected_position);
#endif

        // Go to the preivous cyclic shift.
        position_mod_bwt = expected_position;
        previous_cs = tmp_previous_cs;
        expected_position = smaller + rank(L_store, position_mod_bwt);
    }
    action = DSA_IDLE;

    position_mod_bwt = expected_position;
    return shift;
}

template <class CharT>
uint64_t DynSA<CharT>::getLCP(uint64_t pos)
{
    return lcp->getLCP(pos);
}

template <class CharT>
uint64_t DynSA<CharT>::getSA(uint64_t pos)
{
#if SAMPLE != 0
    return sample->getSA(pos);
#else
    return 1;
#endif
}

template <class CharT>
CharT *DynSA<CharT>::retrieveTextFactor(size_t pos, uint64_t length)
{
    uint64_t bwt_pos, orig_length = length;
    if (pos + length > L->getSize())
    {
        length = L->getSize() - pos;
    }
    bwt_pos = getISA(pos + length);

    char_type *chars = new char_type[orig_length + 1]();
    for (uint64_t i = 0; i < length; ++i)
    {
        chars[length - i - 1] = (*L)[bwt_pos];
        bwt_pos = LFmapping(bwt_pos);
    }
    return chars;
}

}
}

NS_IZENELIB_AM_END

#endif
