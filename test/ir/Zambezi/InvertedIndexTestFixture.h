/**
* @file       InvertedIndexTestFixture.h
* @author     Hongliang
* @version    1.0
* @brief Fixture to test Inverted Index module.
*
*/

#ifndef INVERTED_INDEX_TEST_FIXTURE_H
#define INVERTED_INDEX_TEST_FIXTURE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ir/Zambezi/InvertedIndex.hpp>
NS_IZENELIB_IR_BEGIN

namespace Zambezi
{
    typedef std::vector<std::string> DocIdListT;
    typedef std::map<uint32_t, DocIdListT> DocIDTermMapT;

    class InvertedIndexTestFixture
    {
    public:
        InvertedIndexTestFixture()
            :index_(NULL)
        {
        }

        void initIndex(bool isReverse)
        {
            index_ = new InvertedIndex(NON_POSITIONAL, isReverse);
        }

        ~InvertedIndexTestFixture()
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
            for (std::vector<uint32_t>::iterator i = DocIdList.begin(); i != DocIdList.end(); ++i)
            {
                termList.push_back(term_tmp[x]);
                DocIdTermMap[*i] = termList;
                x++;
                if (x == term_tmp.size() )
                {
                    termList.clear();
                    x = 0;
                }
            }
        }

        void initIndexer(uint32_t docNumber, bool reverse)
        {
            DocIDTermMapT docTermMap;

            prepareDocument(docTermMap, docNumber);

            initIndex(reverse);

            buildIndex(docTermMap);
        }

        void buildIndex(const DocIDTermMapT& docTermMap)
        {
            for (DocIDTermMapT::const_iterator i = docTermMap.begin(); i != docTermMap.end(); ++i)
                index_->insertDoc(i->first, i->second);
            
            index_->flush();
        }

        void search(const std::vector<std::string>& term_list, std::vector<uint32_t>& docid_list, Algorithm algorithm)
        {
            std::vector<float> score_list;
            uint32_t hits = 10000000;
            index_->retrieval(
                algorithm,
                term_list,
                hits,
                docid_list,
                score_list);
        }

    private:
        InvertedIndex* index_;
        std::string indePath_;
    };
}

NS_IZENELIB_IR_END

#endif ///INVERTED_INDEX_TEST_FIXTURE_H
