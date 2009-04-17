/**
 * @file	LexicalTrie.h
 * @brief	Header file of LexicalTrie Class 
 * @author	Yingfeng Zhang
 * @date    2009-02-02
 * @details
 */
#ifndef LEXICAL_TRIE_H_
#define LEXICAL_TRIE_H_

#include <string>
#include <iostream>
#include <set>
#include <vector>

#include <wiselib/ustring/UString.h>

namespace idmanager 
{

    /*
     * Class: LexicalTrie
     *
     * This class implements a lexical trie.  A trie is a tree-like data structure
     * to store words, where each branch of the tree is tagged with a letter and
     * indicates the addition of that vector. At tree nodes, a boolean flag
     * indicates if the word up to that point is in fact contained in the trie.
     */
    class LexicalTrie
    {
    public:

        LexicalTrie();

        ~LexicalTrie();

        /*
         * Method: addWord
         *
         * Adds the specified word to the trie.
         *
         * Parameters:
         *  word - Word to add.
         */
        void addWord(const wiselib::UString & word, const unsigned int id);

        /** Returns the number of words in the trie.
         *
         * \return The number of words.
         */
        int numWords();

        /*
         * Method: matchRegExp
         *
         * Matches a regular expression against the contents of the trie. Stores
         * the matches in the set 'results'.
         *
         * Regular expression syntax:
         *
         * An asterisk indicates '0 or more characters'.  A question mark indicates
         * '0 or 1 characters'.  Any other character means the literal character.
         *
         *
         * Parameters:
         *  exp - The regular expression.
         *  results - The set to store results in.
         */
        bool matchRegExp(const wiselib::UString & exp, std::vector<unsigned int> & results);

        struct TrieNode; // forward declaration

        TrieNode * root_; // root of the trie - created in Trie constructor

        int numWords_; // number of words in the trie

        // LetterTriePair: small data structure to store pairs of letters and trie
        // pointers.
        struct LetterTriePair
        {
            //char letter_; // the letter
            wiselib::UCS2Char letter_; //the letter

            TrieNode * trie_; // the trie node corresponding to this letter

            // default constructor
            LetterTriePair()
            {
                letter_ = 0;
                trie_ = NULL;
            }

            // argument-constructor
            LetterTriePair(wiselib::UCS2Char letter, TrieNode * trie)
                    : letter_(letter), trie_(trie)
            {
            }

            // comparison operators (for the vector template)
            bool operator == (const LetterTriePair & another) const
            {
                return ( (letter_ == another.letter_) && (trie_ == another.trie_) );
            }
            bool operator != (const LetterTriePair & another) const
            {
                return ( (letter_ != another.letter_) || (trie_ != another.trie_) );
            }

            // sort operator
            bool operator< (const LetterTriePair & another) const
            {
                return letter_ < another.letter_;
            }
        };

        // PairCompare: small comparison function to sort the letters in the vector
        struct PairCompare
                    : std::binary_function<LetterTriePair, LetterTriePair, bool>
        {
            bool operator() (const LetterTriePair & left,
                             const LetterTriePair & right)
            {
                return !std::greater<wiselib::UCS2Char>() (left.letter_, right.letter_);
            }
        };

        // TrieNode: support structure that stores a trie node, i.e. whether the
        // chain of letters up to that point constitutes a word, and all the other
        // letters that can be used to form words from this point onwards.
        struct TrieNode
        {
            bool isWord_; // do the letters up to here form a word?
            
            unsigned int id_;
            
            std::vector<LetterTriePair> letters_; // children

            TrieNode();

            ~TrieNode();

            // Add this word (recursive)
            // Return true if actually added
            bool addWord(const wiselib::UString & word, const unsigned int id);

            // Find all regex matches (recursive)
            inline void matchRegExp(std::vector<unsigned int> & results,
                                    const wiselib::UString & pattern,
                                    const wiselib::UString & soFar);

            // Find the letter/trie pair corresponding to this letter, or NULL if
            // not in the array
            LetterTriePair * findLetterPair(wiselib::UCS2Char letter);
        };

    }; // end - class LexicalTrie

} // end - namespace sf1v5 

#endif
