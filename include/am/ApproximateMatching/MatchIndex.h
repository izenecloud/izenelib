#if !defined(_MATCH_INDEX_)
#define _MATCH_INDEX_

#include <util/ustring/UString.h>
#include <vector>

//#include <ir/id_manager/IDManager.h>
//#include <am/succinct/fm-index/wavelet_matrix.hpp>
#include "bit_trie.hpp"
#include <am/ApproximateMatching/wavelet_matrix.hpp>
#include <list>
#include <fstream>
using namespace std;
//using namespace boost;
using namespace izenelib::util;
using namespace izenelib::am::succinct::fm_index;

#define v 1
#define MaxDepth 6

namespace izenelib
{

struct Myclasscmp
{
    unsigned depth;
    bool operator()  (const std::pair<UString,uint32_t> & pair1,const std::pair<UString,uint32_t>  & pair2) 
    {
        return pair1.first[depth]<pair2.first[depth];

    }

};
bool comp(UString ustr1,UString ustr2)
{
   for(unsigned depth=0;depth<MaxDepth;depth++)
        return ustr1[depth]<ustr2[depth];

};

class  MatchIndex
{

    //typedef izenelib::ir::idmanager::AutoFillIDManager IDManger;
    //WaveletMatrix<uint32_t>* wa_;
    izenelib::WaveletMatrix* wa_;
/*
void intersect(
            const std::vector<std::pair<size_t, size_t> > &patterns,
            size_t thres,
            size_t max_count,
            std::vector<char_type> &results) const;
*/
    //boost::scoped_ptr<IDManger> idManager_;
    vector<std::pair<UString,uint32_t> > prefixVec;
    vector<UString > Context;
    vector<Myclasscmp> cmp;
    vector<UString> prefix;
    bool build;
public:
    MatchIndex();
    ~MatchIndex();
    void add(UString text);
    void add(vector<UString> textVec);
    void BuildIndex();
    bool Hasbuild();
    vector<UString> Match(UString  query,int MaxError);
    void save(ofstream ofs);
    void load(ifstream ifs);
    int  EditDistance(UString source,UString target);
    vector<uint32_t>  DynamicSpilt(UString  query,unsigned MaxError);
    void show();
private:
    bool getCandidate(vector< vector<std::pair<size_t,UString> > > &Matrix,size_t i,unsigned j,vector<uint32_t>& ret);
    UString GetText(int num);  
    size_t  getLowBound(UString pre);
    size_t  getUperBound(UString pre);
    std::pair<size_t, size_t>  getBoundary(UString pre);
    size_t  getNum(UString pre);
    int AddText(UString text);
    void sort();
    void BuildWavletTree();
    int TextSize();
    void sortInDepth(unsigned start,unsigned end,unsigned depth);

};

};
/**/
#endif
