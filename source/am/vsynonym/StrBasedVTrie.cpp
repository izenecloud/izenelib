/*
 * \file StrBasedVTrie.cpp
 * \brief 
 * \date May 4, 2010
 * \author Vernkin Chen
 */

#include <am/vsynonym/StrBasedVTrie.h>
#include <am/vsynonym/VTrie.h>

using namespace izenelib::am;

bool createVTrie( const vector<string>& words, VTrie& trie )
{
    VTrieNode node;
    node.data = 1;
    for( vector<string>::const_iterator itr = words.begin(); itr != words.end(); ++itr )
    {
        trie.insert( itr->data(), &node );
    }
    return true;
}


StrBasedVTrie::StrBasedVTrie( VTrie* pTrie )
    : trie(pTrie), node(new VTrieNode),
    completeSearch(false)
{
}

StrBasedVTrie::~StrBasedVTrie()
{
    delete node;
}

void StrBasedVTrie::reset()
{
    completeSearch = true;
    node->init();
}

void StrBasedVTrie::getCurrentNode(VTrieNode& rnode) const
{
    if (node)
    {
        rnode.data = node->data;
        rnode.offset = node->offset;
        rnode.moreLong = node->moreLong;
        rnode.state = node->state;
    }
}

bool StrBasedVTrie::search( const char *p )
{
    if(!completeSearch)
        return false;

    while(node->moreLong && *p)
    {
        trie->find(*p, node);
        //cout<<"finding,"<<node<<endl;
        ++p;
    }

    //the node.data can be negative (as no pos tags)
    completeSearch = !(*p) && (node->data > 0 || node->moreLong);

    return completeSearch;
}


bool StrBasedVTrie::firstSearch( const char* p )
{
    reset();
    return search( p );
}


bool StrBasedVTrie::exists()
{
    return completeSearch && node->data > 0;
}


