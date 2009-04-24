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

using namespace std;

namespace idmanager {

/*
 * Class: LexicalTrie
 *
 * This class implements a lexical trie.  A trie is a tree-like data structure
 * to store words, where each branch of the tree is tagged with a letter and
 * indicates the addition of that vector. At tree nodes, a boolean flag
 * indicates if the word up to that point is in fact contained in the trie.
 */
template<typename NameString> class LexicalTrie {
	typedef typename NameString::value_type charType;
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
	void addWord(const NameString & word, const unsigned int id);

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
	bool matchRegExp(const NameString& exp, std::vector<unsigned int> & results);

	struct TrieNode; // forward declaration

	TrieNode * root_; // root of the trie - created in Trie constructor

	int numWords_; // number of words in the trie

	// LetterTriePair: small data structure to store pairs of letters and trie
	// pointers.
	struct LetterTriePair {
		//char letter_; // the letter
		charType letter_; //the letter

		TrieNode * trie_; // the trie node corresponding to this letter

		// default constructor
		LetterTriePair() {
			letter_ = 0;
			trie_ = NULL;
		}

		// argument-constructor
		LetterTriePair(charType letter, TrieNode * trie) :
			letter_(letter), trie_(trie) {
		}

		// comparison operators (for the vector template)
		bool operator ==(const LetterTriePair & another) const {
			return ( (letter_ == another.letter_) && (trie_ == another.trie_) );
		}
		bool operator !=(const LetterTriePair & another) const {
			return ( (letter_ != another.letter_) || (trie_ != another.trie_) );
		}

		// sort operator
		bool operator<(const LetterTriePair & another) const {
			return letter_ < another.letter_;
		}
	};

	// PairCompare: small comparison function to sort the letters in the vector
	struct PairCompare :
			std::binary_function<LetterTriePair, LetterTriePair, bool> {
		bool operator()(const LetterTriePair & left, const LetterTriePair & right) {
			return !std::greater<charType>() (left.letter_, right.letter_);
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

		TrieNode() {
			isWord_ = false;

		}

		~TrieNode() {
			// Clear children
			for (typename std::vector<LetterTriePair>::iterator it = letters_.begin(); it
					!= letters_.end(); it++) {
				delete it->trie_;
			}

		}

		// Add this word (recursive)
		// Return true if actually added
		bool addWord(const NameString & word, const unsigned int id)
		{
			// recursive base case:
			if (word.length()== 0) {
				bool added = false;

				// was this not already a word?
				if (!isWord_)
				added = true;

				isWord_ = true;
				id_ = id;
				return added;
			}

			LetterTriePair * pair = findLetterPair(word[0]);
			NameString subWord;
			word.substr(subWord, 1);

			if (pair) {
				// pair exists so update it...
				return pair->trie_->addWord(subWord, id);
			} else {
				// pair doesn't exist, so create it
				TrieNode * newTrie = new TrieNode();

				// add the word recursively to the new trie
				newTrie->addWord(subWord, id);

				letters_.push_back(LetterTriePair(word[0], newTrie));

				// keep the vector sorted
				sort(letters_.begin(), letters_.end());

				// in this case, the word was added because we didn't have
				// this branch to begin with...
				return true;
			}

		}

		// Find all regex matches (recursive)
		inline void matchRegExp(std::vector<unsigned int> & results,
				const NameString & pattern,
				const NameString & soFar)
		{
			// do the pattern and string match? (i.e. no wildcards)
			if (pattern.length() == 0 && soFar.length() != 0) {
				if (this->isWord_)
				results.push_back(this->id_);
				return;
			}

			typename std::vector<LetterTriePair>::iterator it;
			switch (pattern[0]) {
				//case '*':
				case 42: {
					// Try matching 1 or more characters
					for (it = letters_.begin(); it != letters_.end(); it++) {
						NameString newSoFar(soFar);
						newSoFar += it->letter_;
						it->trie_->matchRegExp(results, pattern, newSoFar);
					}
					// Try matching 0 characters

					NameString newPattern;
					pattern.substr(newPattern, 1);
					this->matchRegExp(results, newPattern, soFar);
					break;
				}
				//case '?':
				case 63: {
					NameString newPattern;
					pattern.substr(newPattern, 1);

					// Try matching no character
					this->matchRegExp(results, newPattern, soFar);

					// Try matching one character

					for (it = letters_.begin(); it != letters_.end(); it++) {
						NameString newSoFar(soFar);
						newSoFar += it->letter_;
						it->trie_->matchRegExp(results, newPattern, newSoFar);
					}
					break;
				}
				default:
				// just make sure the letter matches - see if we have that letter from here
				LetterTriePair * pair = this->findLetterPair(pattern[0]);
				if (pair) {
					// we have it - remove it from pattern,
					// add to soFar, and continue
					NameString newSoFar(soFar);
					newSoFar += pair->letter_;
					NameString newPattern;
					pattern.substr(newPattern, 1);

					pair->trie_->matchRegExp(results, newPattern, newSoFar);
				} else {
					// we don't have the letter... abort
					return;
				}
				break;
			}
		}

		// Find the letter/trie pair corresponding to this letter, or NULL if
		// not in the array
		LetterTriePair * findLetterPair(charType letter) {
			for (typename std::vector<LetterTriePair>::iterator it = letters_.begin(); it
					!= letters_.end(); it++) {
				if (it->letter_ == letter)
				return &(*it);
			}
			return NULL;
		}
	};

}; // end - class LexicalTrie

template<typename NameString> LexicalTrie<NameString>::LexicalTrie() {
	root_ = new TrieNode();
	numWords_ = 0;
}

template<typename NameString> LexicalTrie<NameString>::~LexicalTrie() {
	delete root_;
}
template<typename NameString> void LexicalTrie<NameString>::addWord(
		const NameString & word, const unsigned int id) {
	bool added = root_->addWord(word,id);
	if (added)
	numWords_++;

}
template<typename NameString> bool LexicalTrie<NameString>::matchRegExp(
		const NameString & exp, std::vector<unsigned int> & results) {
	NameString sofar;
	root_->matchRegExp(results, exp, sofar);
	return results.size()>0 ? true : false;
}
template<typename NameString> int LexicalTrie<NameString>::numWords() {
	return numWords_;
}

} // end - namespace sf1v5 

#endif
