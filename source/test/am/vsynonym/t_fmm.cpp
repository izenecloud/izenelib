/*
 * \file t_fmm.cpp
 * \brief simple fmm algorithin using VTrie
 * \date May 18, 2010
 * \author Vernkin Chen
 */
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include <am/vsynonym/VTrie.h>

using izenelib::am::VTrie;
using izenelib::am::VTrieNode;

namespace fmminner{

class UTF8EncType
{
public:
    unsigned int getByteCount(const char* p) const
    {
        unsigned char val = (unsigned char)p[0];

        if( val == 0 )        // length   byte1     byte2     byte3    byte4
            return 0;
        else if( val < 0x80 ) //    1    0xxxxxxx
            return 1;
        else if( val < 0xE0 ) //    2    110yyyxx  10xxxxxx
            return 2;
        else if( val < 0xF0)  //    3    1110yyyy  10yyyyxx  10xxxxxx
            return 3;
        return 4;             //    4    11110zzz  10zzyyyy  10yyyyxx  10xxxxxx
    }

    bool isSpace( const char* p ) const
    {
        if( *p == ' ' )
            return true;
        const unsigned char* up = (const unsigned char*)p;
        return up[0] == 0xE3 && up[1] == 0x80 && up[2] == 0x80;
    }
};

template<class CType>
void toCombine(VTrie *trie, CType* type, vector<string>& src,
            int begin, int lastWordEnd, vector<string>& dest){
    if(begin == lastWordEnd){
        string& str = src[begin];
        if(!type->isSpace(str.c_str()))
            dest.push_back(str);
        return;
    }
    bool isOK = true;

    if(isOK){
        string buf = src[begin];
        for(int k=begin+1; k<=lastWordEnd; ++k)
            buf.append(src[k]);
        dest.push_back(buf);
    }else{
        for(int k=begin; k<=lastWordEnd; ++k){
            string& str = src[k];
            if(!type->isSpace(str.c_str()))
                dest.push_back(str);
        }
    }
}

template<class CType>
void combineRetWithTrie(VTrie *trie, vector<string>& src,
        vector<string>& dest, CType* type) {

    int begin = -1;
    int lastWordEnd = -1;

    int n = (int)src.size();
    VTrieNode node;
    for (int i = 0; i < n; ++i) {
        string& str = src[i];

        size_t strLen = str.length();
        size_t j = 0;
        for (; node.moreLong && j < strLen; ++j) {
            trie->find(str[j], &node);
        }

        //did not reach the last bit
        if (j < strLen) {
            //no exist in the dictionary
            if (begin < 0)
            {
                if(!type->isSpace(str.c_str()))
                    dest.push_back(str);
            }
             else {
                if(lastWordEnd < begin)
                    lastWordEnd = begin;
                toCombine<CType>(trie, type, src, begin, lastWordEnd, dest);
                begin = -1;
                //restart that node
                i = lastWordEnd; //another ++ in the loop
            }
            node.init();
        } else {
            if (node.moreLong && (i < n - 1)) {
                if(begin < 0)
                    begin = i;
                if(node.data > 0)
                    lastWordEnd = i;
            } else {
                if (begin < 0)
                {
                    if(!type->isSpace(str.c_str()))
                        dest.push_back(str);
                }
                else {
                    if(node.data > 0)
                        lastWordEnd = i;
                    else if(lastWordEnd < begin)
                        lastWordEnd = begin;

                    toCombine<CType>(trie, type, src, begin, lastWordEnd, dest);
                    begin = -1;
                    i = lastWordEnd;
                }
                node.init();
            }
        }
    } //end for

    if(begin >= 0){
        toCombine<CType>(trie, type, src, begin, n-1, dest);
    }
}

template<class CType>
bool fmm(VTrie *trie, CType* type, const string& input, vector<string>& output )
{
    const char* curPtr = input.c_str();
    vector<string> inputVec;
    size_t pos = 0;
    do{
       unsigned int len = type->getByteCount( curPtr );
       if( len == 0 )
           break;
       inputVec.push_back( input.substr( pos, len ) );
       pos += len;
       curPtr += len;
    }while(true);

    if( inputVec.empty() == true )
        return false;

    combineRetWithTrie<CType>( trie, inputVec, output, type );
    return true;
}

bool loadVTrie( const char* fileName, VTrie* trie )
{
    ifstream in(fileName);
    if( !in )
        return false;

    VTrieNode node;
    node.data = 1;

    string line;
    while(!in.eof()){
        getline( in, line );
        if( line.empty() == true )
            continue;

        //TODO here only fecth the first token
        size_t endPos = line.find( ' ' );
        if( endPos == line.npos )
            endPos = line.length();
        string val = line.substr( 0, endPos );
        if( val.empty() == true )
            continue;
        trie->insert( val.c_str(), &node );
    }

    in.close();
    return true;
}

}

int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        cerr << "Usage: ./t_fmm dictFile" << endl;
        cerr << "\tdictFile must be utf8 encoding in this testing" << endl;
        return 1;
    }

    const char* dictFile = argv[1];
    VTrie trie;
    if( fmminner::loadVTrie( dictFile, &trie) == false )
    {
        cerr << "Error: Fail to load dictionary from " << dictFile << endl;
        return 1;
    }

    fmminner::UTF8EncType utf8Type;
    string line;
    do{
        cout << "Input a sentence ('x' or 'X' to exit) : ";
        getline( cin, line );
        if( line == "x" || line == "X" )
            break;

        vector<string> output;
        fmminner::fmm<fmminner::UTF8EncType>( &trie, &utf8Type, line, output );
        cout << "Result: ";
        for( vector<string>::iterator itr = output.begin(); itr != output.end(); ++itr )
        {
            cout << *itr << " ";
        }
        cout << endl;

    }while(true);

    cout << "Bye!" << endl;
    return 0;
}
