#include <iostream>
#include <fstream>
#include <algorithm>

#include <LexicalTrie.h>

using namespace std;

namespace idmanager {

    LexicalTrie::LexicalTrie()
    {
        root_ = new TrieNode();
        numWords_ = 0;
    }
    LexicalTrie::~LexicalTrie()
    {
        delete root_;
    }

    void LexicalTrie::addWord(const wiselib::UString & word, const unsigned int id)
    {
        bool added = root_->addWord(word,id);

        if (added)
            numWords_++;

    }

    bool LexicalTrie::matchRegExp(const wiselib::UString & exp, std::vector<unsigned int> & results)
    {
        wiselib::UString sofar;
        root_->matchRegExp(results, exp,sofar);
        return results.size()>0?true:false;
    }

    int LexicalTrie::numWords()
    {
        return numWords_;
    }

    /*********************************
     * TrieNode                     *
     *********************************/

    // Constructor
    LexicalTrie::TrieNode::TrieNode()
    {
        isWord_ = false;
    }
    // Destructor
    LexicalTrie::TrieNode::~TrieNode()
    {
        // Clear children
        for (std::vector<LetterTriePair>::iterator it = letters_.begin();
                it != letters_.end();
                it++)
        {
            delete it->trie_;
        }
    }

    bool LexicalTrie::TrieNode::addWord(const wiselib::UString & word, const unsigned int id)
    {
        // recursive base case:
        if ( word.length()== 0 )
        {
            bool added = false;

            // was this not already a word?
            if (!isWord_)
                added = true;

            isWord_ = true;
            id_ = id;
            return added;
        }

        LetterTriePair * pair = findLetterPair(word[0]);
        wiselib::UString subWord;
        word.subString(subWord, 1);

        if (pair)
        {
            // pair exists so update it...
            return pair->trie_->addWord(subWord,id);
        }
        else
        {
            // pair doesn't exist, so create it
            TrieNode * newTrie = new TrieNode();

            // add the word recursively to the new trie
            newTrie->addWord(subWord,id);

            letters_.push_back(LetterTriePair(word[0], newTrie));

            // keep the vector sorted
            sort(letters_.begin(), letters_.end());

            // in this case, the word was added because we didn't have
            // this branch to begin with...
            return true;
        }
    }

    inline void LexicalTrie::TrieNode::matchRegExp(std::vector<unsigned int> & results,
            const wiselib::UString & pattern,
            const wiselib::UString & soFar)
    {
        // do the pattern and string match? (i.e. no wildcards)
        if ( pattern.length() == 0 && soFar.length() != 0 )
        {
            if ( this->isWord_ )
                results.push_back(this->id_);
            return;
        }

        std::vector<LetterTriePair>::iterator it;
        switch ( pattern[0] )
        {
        //case '*':
        case 42:
        {
            // Try matching 1 or more characters
            for (it = letters_.begin(); it != letters_.end(); it++)
            {
                wiselib::UString newSoFar(soFar);
                newSoFar += it->letter_;
                it->trie_->matchRegExp(results, pattern,newSoFar);
            }

            // Try matching 0 characters
            
            wiselib::UString newPattern;
            pattern.subString(newPattern, 1);
            this->matchRegExp(results, newPattern, soFar);
            break;
        }
        //case '?':
        case 63:
        {
            wiselib::UString newPattern;
            pattern.subString(newPattern, 1);

            // Try matching no character
            this->matchRegExp(results, newPattern, soFar);

            // Try matching one character

            for (it = letters_.begin(); it != letters_.end(); it++)
            {
                wiselib::UString newSoFar(soFar);
                newSoFar += it->letter_;
                it->trie_->matchRegExp(results, newPattern,newSoFar);
            }
            break;
        }
        default:
            // just make sure the letter matches - see if we have that letter from here
            LetterTriePair * pair = this->findLetterPair( pattern[0] );
            if (pair)
            {
                // we have it - remove it from pattern,
                // add to soFar, and continue
                wiselib::UString newSoFar(soFar);
                newSoFar += pair->letter_;
                wiselib::UString newPattern;
                pattern.subString(newPattern, 1);
                
                pair->trie_->matchRegExp(results,newPattern, newSoFar);
            }
            else
            {
                // we don't have the letter... abort
                return;
            }
            break;
        }
    }

    /* TrieNode::FindLetterPair
     * -------------------------
     * Search a trie node for a given letter child node. Return the pair structure
     * (that contains both the letter and the pointer to the child trie.)
     */
    LexicalTrie::LetterTriePair * LexicalTrie::TrieNode::findLetterPair(wiselib::UCS2Char letter)
    {
        for (std::vector<LetterTriePair>::iterator it = letters_.begin();
                it != letters_.end(); it++ )
        {
            if ( it->letter_ == letter )
                return &(*it);
        }
        return NULL;
    }

} // end - namespace sf1v5




/*
int main()
{
    LexicalTrie trie;

    ifstream inf("dict.txt");
    string str;
    unsigned int id = 0;
    while (inf>>str)
    {
        wiselib::UString ustr(str,wiselib::UString::UTF_8);
        trie.addWord(ustr,++id);
        str.clear();
    }

    clock_t t1 = clock();

    std::vector<unsigned int> results;
    wiselib::UString pattern("*人*",wiselib::UString::UTF_8);
    trie.matchRegExp(pattern, results);

    printf("eclipse: %lf seconds\n", double(clock()- t1)/CLOCKS_PER_SEC);

    cout<<"numWords: "<<trie.numWords()<<endl;
    for(std::vector<unsigned int>::iterator iter = results.begin(); iter != results.end(); ++iter)
      	cout<<*iter<<endl;
    return 0;
}

*/
