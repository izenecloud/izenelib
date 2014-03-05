/**
* @file       AttrScoreInvertedIndexTestFixture.h
* @author     Hongliang
* @version    1.0
* @brief Fixture to test Inverted Index module.
*
*/

#ifndef ATTR_SCORE_INVERTED_INDEX_TEST_FIXTURE_H
#define ATTR_SCORE_INVERTED_INDEX_TEST_FIXTURE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <ir/Zambezi/AttrScoreInvertedIndex.hpp>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{
    typedef std::vector<std::string> DocIdListT;
    typedef std::map<uint32_t, DocIdListT> DocIDTermMapT;

    const unsigned int DefullNum = 5000000;
    class AttrScoreInvertedIndexTestFixture
    {
    public:
        AttrScoreInvertedIndexTestFixture()
            : index_(NULL)
        {
        }

        void initIndex(bool isReverse)
        {
            index_ = new AttrScoreInvertedIndex(1 << 28, 4, 4194304, isReverse);
        }

        ~AttrScoreInvertedIndexTestFixture()
        {
            delete index_;
        }

        void prepareDocument(DocIDTermMapT& DocIdTermMap, uint32_t docNumber)
        {
            std::vector<uint32_t> DocIdList;
            for (unsigned int i = 1; i <= docNumber; ++i)
            {
                DocIdList.push_back(i);
            }

            std::vector<std::string> termList;
            std::vector<std::string> term_tmp;
            term_tmp.push_back("abc");
            term_tmp.push_back("abd");
            term_tmp.push_back("abe");
            term_tmp.push_back("abf");
            term_tmp.push_back("abg");
            term_tmp.push_back("abh");
            term_tmp.push_back("abi");
            term_tmp.push_back("abj");
            term_tmp.push_back("abk");
            term_tmp.push_back("abl");
            term_tmp.push_back("abm");
            term_tmp.push_back("abn");
            term_tmp.push_back("abo");
            term_tmp.push_back("abp");
            term_tmp.push_back("abq");
            unsigned int x = 0;
            for (std::vector<uint32_t>::iterator it = DocIdList.begin(); it != DocIdList.end(); ++it)
            {
                termList.push_back(term_tmp[x]);
                DocIdTermMap[*it] = termList;
                if (++x == term_tmp.size())
                {
                    termList.clear();
                    x = 0;
                }
            }
        }

        void prepareWordList()
        {
            std::vector<std::string> charString;
            charString.push_back("a");
            charString.push_back("b");
            charString.push_back("c");
            charString.push_back("d");
            charString.push_back("e");
            charString.push_back("f");
            charString.push_back("g");
            charString.push_back("h");
            charString.push_back("i");
            charString.push_back("j");
            charString.push_back("k");
            charString.push_back("l");
            charString.push_back("m");
            charString.push_back("n");
            charString.push_back("o");
            charString.push_back("p");
            charString.push_back("q");
            charString.push_back("r");
            charString.push_back("s");
            charString.push_back("t");
            charString.push_back("u");
            charString.push_back("v");
            charString.push_back("w");
            charString.push_back("x");
            charString.push_back("y");
            charString.push_back("z");
            charString.push_back("1");
            charString.push_back("2");
            charString.push_back("3");
            charString.push_back("4");
            charString.push_back("5");
            charString.push_back("6");
            charString.push_back("7");
            charString.push_back("8");
            charString.push_back("9");
            charString.push_back("0");
            charString.push_back("-");
            charString.push_back("鞋");
            charString.push_back("衣");
            charString.push_back("裙");
            charString.push_back("裤");
            charString.push_back("运");
            charString.push_back("动");
            int termNumber = 4096;
            //build word;
            srand(0);
            for (int i = 0; i < termNumber; ++i)
            {
                int wordlen = rand()%5 + 2;
                std::string newword;
                for (int i = 0; i < wordlen; ++i)
                {
                    int randchar = rand()%charString.size();
                    newword += charString[randchar];
                }
                wordlist_.push_back(newword);
            }
        }
        void prepareBigDocument(DocIDTermMapT& DocIdTermMap, uint32_t docNumber, uint32_t lastDocid)
        {
            //build term;
            std::string freq_word = "123abc";
            std::vector<uint32_t> DocIdList;
            std::cout << "document lastDocid:" << lastDocid << std::endl;
            for (unsigned int i = 1 + lastDocid; i <= docNumber + lastDocid; ++i)
            {
                DocIdList.push_back(i);
            }
            for (std::vector<uint32_t>::iterator i = DocIdList.begin(); i != DocIdList.end(); ++i)
            {
                std::vector<std::string> termList;
                int wordNumber = rand()%4 + 5;
                for (int j = 0; j < wordNumber; ++j)
                {
                    int randwrod = rand()%wordlist_.size();
                    termList.push_back(wordlist_[randwrod]);
                }
                termList.push_back(freq_word);
                DocIdTermMap[*i] = termList;
            }
        }

        void initBIGIndexer(uint32_t docNumber, bool reverse)
        {
            DocIDTermMapT docTermMap;
            prepareWordList();
            initIndex(reverse);
            uint32_t number = docNumber%DefullNum;
            int times = docNumber/DefullNum;
            int lastDocid = 0;
            while (times >= 1)
            {
                prepareBigDocument(docTermMap, DefullNum, lastDocid);
                buildIndex(docTermMap);
                docTermMap.clear();
                times--;
                lastDocid += DefullNum;
            }
            if (number != 0)
            {
                prepareBigDocument(docTermMap, number, lastDocid);
                buildIndex(docTermMap);
            }
            docTermMap.clear();
        }

        void initIndexer(uint32_t docNumber, bool reverse, bool search_buffer = false)
        {
            DocIDTermMapT docTermMap;

            prepareDocument(docTermMap, docNumber);

            initIndex(reverse);

            buildIndex(docTermMap, search_buffer);
        }

        void buildIndex(const DocIDTermMapT& docTermMap, bool search_buffer = false)
        {
            int count = 1;
            for (DocIDTermMapT::const_iterator i = docTermMap.begin(); i != docTermMap.end(); ++i)
            {
                std::vector<uint32_t> score_list;
                score_list.resize(i->second.size(), 1);
                index_->insertDoc(i->first, i->second, score_list);
                if (count % 100000 == 0)
                {
                    //std::cout << "insert document:" << count << std::endl;
                }
                count++;
            }
            if (!search_buffer)
                index_->flush();
        }

        void search(const std::vector<std::string>& term_list, std::vector<uint32_t>& docid_list)
        {
            std::vector<float> score_list;
            std::vector<std::pair<std::string, int> > term_list_1;
            for (unsigned int i = 0; i < term_list.size(); ++i)
            {
                term_list_1.push_back(make_pair(term_list[i], 0));
            }
            uint32_t hits = 10000000;
            FilterBase* filter = new FilterBase;
            index_->retrieve(
                SVS,
                term_list_1,
                filter,
                hits,
                docid_list,
                score_list);
        }

        void saveIndex(const std::string& path)
        {
            fstream fileIndex;
            fileIndex.open(path.c_str(), ios::binary|ios::out);
            index_->save(fileIndex);
        }
        void loadIndex(const std::string& path)
        {
            fstream fileIndex;
            fileIndex.open(path.c_str(), ios::binary|ios::in);

            if (fileIndex.is_open())
            {
                index_->load(fileIndex);
            }
        }

        std::vector<std::string>& getWordList()
        {
            return wordlist_;
        }

        uint32_t getTotalDocCount() const
        {
            return index_->totalDocNum();
        }

    private:
        AttrScoreInvertedIndex* index_;
        std::vector<std::string> wordlist_;
    };
}

NS_IZENELIB_IR_END

#endif ///NEW_INVERTED_INDEX_TEST_FIXTURE_H
