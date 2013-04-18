#include <boost/date_time/posix_time/posix_time.hpp>
#include <am/ApproximateMatching/MatchIndex.h>

using namespace std;
using namespace izenelib;
using namespace izenelib::util;


namespace izenelib
{
namespace am
{
string toString(UString us)
{
    string str;
    us.convertString(str, izenelib::util::UString::UTF_8);
    return str;
}

MatchIndex::MatchIndex()
{
    build=false;
}
MatchIndex::~MatchIndex()
{
}
void MatchIndex::Add(const UString& text)
{
    uint32_t id=AddText(text);
    size_t length=text.length();
    for(size_t i=0; i<length; i++)
    {
        for(size_t k=1; k<=min(size_t(MaxDepth),(length-i)); k++)
        {
            OneSubstr substr;
            substr.ID=id;
            substr.BeginPos=i;
            substr.EndPos=i+k-1;
            if(prefixVec.find(text.substr(i,k))!=prefixVec.end())
            {
                prefixVec[ text.substr(i,k)].Candidates[0].k.push_back(substr);
            }
            else
            {
                PostingList ps;
                ps.MaxError=min(MaxErrorDepth,int(k-1));
                ps.StrLength=k;
                ps.Candidates.resize(1);
                ps.Candidates[0].k.push_back(substr);
                prefixVec[ text.substr(i,k)]= ps;
            }
        }
    }
}

void MatchIndex::Add(vector<UString>& textVec)
{
    for(unsigned i=0; i<=textVec.size(); i++)
        Add(textVec[i]);
}


void MatchIndex::BuildIndex()
{
    /*
        std::map<UString,PostingList >::iterator it;
        for(size_t i=2; i<=MaxDepth; i++)
        {
            int k=0;
            for ( it = prefixVec.begin(); it != prefixVec.end(); it++ )
            {
                UString ustr=it->first;
                k++;
                if(k%1000==0)
                cout<<k<<endl;
                if(ustr.length()==i)
                {
                    it->second=prefixVec[ustr.substr(0,i-1)]|prefixVec[ustr.substr(i-1)];
                }

            }
        }
    */
    build=true;
}
int MatchIndex::SizeOfPosting(const UString &query)
{
    if(prefixVec.find(query)!=prefixVec.end())
    {
        return prefixVec[query].Candidates[0].k.size();
    }
    return 0;
}

void MatchIndex::Spilit(const UString&  query,const int& MaxError,std::pair<int,std::vector<UString> > & ret)
{
    int n=query.length();
    int m=MaxError;
    typedef vector< vector<int> >  Tmatrix;
    typedef vector< vector<vector<UString > > >  STmatrix;
    Tmatrix matrix(n+1);
    STmatrix segment(n+1);
    for(int i=0; i<=n; i++)
    {
        matrix[i].resize(m+1);
        segment[i].resize(m+1);
    }
    for(int i=1; i<=n; i++)
        for(int j=1; j<=m; j++) matrix[i][j]=LargeNum;


    for(int i=1; i<=n; i++)
    {
        matrix[i][0]=SizeOfPosting(query.substr(max(0,(i-MaxDepth)),min(MaxDepth,i)));

        int k=0;
        while(k<max(0,(i-MaxDepth)))
        {
            segment[i][0].push_back(query.substr(k,min(MaxDepth,i-MaxDepth-k)));
            k=k+MaxDepth;
        }
        segment[i][0].push_back(query.substr(max(0,(i-MaxDepth)),min(MaxDepth,i)));
    }
    for(int i=1; i<=n; i++)
    {
        for(int j=1; j<=min(m,i-1); j++)
        {
            for(int k=max(1,i-MaxDepth); k<i; k++)
            {
                if(SizeOfPosting(query.substr(k,(i-k)))+matrix[i][j-1]<matrix[i][j])
                {
                    matrix[i][j]=SizeOfPosting(query.substr(k,(i-k)))+matrix[k][j-1];
                    segment[i][j]=segment[k][j-1];
                    segment[i][j].push_back(query.substr(k,i-k));
                    /*
                    if(j==m&&matrix[i][j]<20)
                    {
                       while(k<(n-MaxDepth))
                       {
                            segment[i][0].push_back(query.substr(k,MaxDepth));
                            k=k+MaxDepth;
                       }
                       segment[i][0].push_back(query.substr(k));
                       return   make_pair(matrix[i][j],segment[i][j]);
                    }
                    */
                }
            }
        }
    }


    ret=make_pair(matrix[n][m],segment[n][m]);
}
void MatchIndex::Match(const UString&  query,const int& MaxError, vector<UString>& result )
{
    if(MaxError>=int(query.length()))
        return;
    std::pair<int,std::vector<UString> > seg;
    Spilit(query,MaxError,seg);
    PostingList ps;
    ps.StrLength=0;
    ps.MaxError=0;
    for(unsigned i=0; i<seg.second.size(); i++)
    {
        UString  temp=seg.second[i];
        if(SizeOfPosting(temp)==0)
        {
            PostingList Empty;
            Empty.MaxError=0;
            Empty.StrLength=temp.length();
            ps=mergeWihthMaxError ( ps,Empty,MaxError,true);
            continue;
        }
        ps=mergeWihthMaxError ( ps,prefixVec[temp],MaxError,true);
    }
    vector<Candidate> temp;
    temp.resize(ps.Candidates.size());
    for(unsigned i=0; i<ps.Candidates.size(); i++)
    {
        for(unsigned j=0; j<ps.Candidates[i].k.size(); j++)
        {

            {
                temp[i].k.push_back(ps.Candidates[i].k[j]);
            }
        }
    }
    vector<uint32_t> IDs=MergeSort (temp);//Multi-way merge sort
    vector<uint32_t>::iterator end_unique=unique(IDs.begin(),IDs.end());
    IDs.erase(end_unique,IDs.end());
    for(unsigned i=0; i<IDs.size(); i++)
    {
        UString ustr=GetText(IDs[i]);
        if(ustr.length()>query.length()+MaxError||query.length()>ustr.length()+MaxError)
            continue;
        if (EditDistance(ustr,query)<=MaxError)
            result.push_back(ustr);
    }
}

void MatchIndex::NaiveMatch(const UString&  query,const int& MaxError,  vector<UString>& result)
{
    for(unsigned i=0; i<Context.size(); i++)
    {
        UString ustr=GetText(i);
        if (EditDistance(ustr,query)<=MaxError)
            result.push_back(ustr);
    }
}
void MatchIndex::Save(ostream &ofs)
{

    size_t size=Context.size();
    ofs.write((const char*)&size,sizeof(size));

    for(size_t i=0; i<Context.size(); i++)
    {

        std::string str;
        Context[i].convertString(str, UString::UTF_8);
        size_t len=sizeof(str[0])*(str.size());
        ofs.write((const char*)&len, sizeof(len));
        ofs.write((const char*)&str[0], sizeof(str[0])*(str.size()) );
    }

    size=prefixVec.size();
    ofs.write((const char*)&size,sizeof(size));
    for (    std::map<UString,PostingList >::iterator it = prefixVec.begin(); it != prefixVec.end(); it++ )
    {
        std::string str;
        it->first.convertString(str, UString::UTF_8);
        size_t len=sizeof(str[0])*(str.size());
        ofs.write((const char*)&len, sizeof(len));
        ofs.write((const char*)&str[0], sizeof(str[0])*(str.size()) );
        ofs<<it->second;
    }


}
void MatchIndex::Load(istream &ifs)
{
    size_t size;
    ifs.read(( char*)&size, sizeof(size));
    Context.resize(size);
    size_t len;
    for(size_t i=0; i<size; i++)
    {
        ifs.read(( char*)&len, sizeof(len));
        std::string str;
        str.resize(len);
        ifs.read(( char*)&str[0], len);
        Context[i]=UString(str,UString::UTF_8);
    }
    ifs.read(( char*)&size, sizeof(size));
    for(size_t i=0; i<size; i++)
    {
        size_t len;
        ifs.read(( char*)&len, sizeof(len));
        std::string str;
        str.resize(len);
        ifs.read(( char*)&str[0], len);
        PostingList ps;
        ifs>>ps;
        prefixVec.insert(make_pair(UString(str,UString::UTF_8), ps) );

    }
    build=true;
}
int MatchIndex::EditDistance(const UString& source,const UString& target)
{
    int n=source.length();
    int m=target.length();
    if (m==0) return n;
    if (n==0) return m;

    typedef vector< vector<int> >  Tmatrix;
    Tmatrix matrix(n+1);
    for(int i=0; i<=n; i++)  matrix[i].resize(m+1);
    for(int i=1; i<=n; i++) matrix[i][0]=i;
    for(int i=1; i<=m; i++) matrix[0][i]=i;

    for(int i=1; i<=n; i++)
    {
        const UCS2Char si=source[i-1];
        for(int j=1; j<=m; j++)
        {
            const UCS2Char dj=target[j-1];
            int cost;
            if(si==dj)
            {
                cost=0;
            }
            else
            {
                cost=1;
            }
            const int above=matrix[i-1][j]+1;
            const int left=matrix[i][j-1]+1;
            const int diag=matrix[i-1][j-1]+cost;
            matrix[i][j]=min(above,min(left,diag));
        }
    }
    return matrix[n][m];

}

UString MatchIndex::GetText(const int& num)
{
    return Context[num];
}
int MatchIndex::AddText(const UString& text)
{
    Context.push_back(text);
    return Context.size()-1;
}
int MatchIndex::TextSize()
{
    return Context.size();
}
bool MatchIndex::Hasbuild()
{
    return build;
}
void MatchIndex::Clear()
{

    prefixVec.clear();
    Context.clear();
    build=false;
}
void MatchIndex::Show()
{

}



};



};
