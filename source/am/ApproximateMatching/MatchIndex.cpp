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

}
MatchIndex::~MatchIndex()
{
}
void MatchIndex::Add(UString text)
{
    uint32_t id=AddText(text);
    int length=text.length();
    for(unsigned i=0; i<length; i++)
    {
        for(unsigned k=1; k<=MaxDepth; k++)
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

void MatchIndex::Add(vector<UString> textVec)
{
    for(unsigned i=0; i<=textVec.size(); i++)
        Add(textVec[i]);
}


void MatchIndex::BuildIndex()
{
    std::map<UString,PostingList >::iterator it;
/*
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
vector<UString> MatchIndex::Match(UString  query,int MaxError)
{
    vector<UString> ret;
    if(MaxError>=query.length())
        return ret;
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time(); 
    uint32_t i=0;
    int k=1;
    PostingList ps;
    ps.StrLength=0;
    ps.MaxError=0;
    bool begin=true;
    while((i+k)<=query.length())
    {
        UString  temp;
        while(prefixVec.find(query.substr(i,k))!=prefixVec.end()&&(i+k)<=query.length()&&MaxError<ps.MaxError+1+(query.length()-i-k)+1)
        {
            temp=query.substr(i,k);
            k++;
        }
        //cout<<"prefixVec.find(query.substr(i,k))!=prefixVec.end()"<<(prefixVec.find(query.substr(i,k))!=prefixVec.end())<<((i+k)<=query.length())<<(MaxError<ps.MaxError+(query.length()-i-k))<<endl;
        //cout<<"MaxError"<<MaxError<<"ps.MaxError"<<ps.MaxError<<"i"<<i<<"k"<<k<<"length"<<query.length()<<endl;
        if(temp.length()==0)
        {
            PostingList Empty;
            Empty.MaxError=0;
            Empty.StrLength=1;
            i++;
            k=1;
            ps=mergeWihthMaxError ( ps,Empty,MaxError,true);
            continue;
        }
        ps=mergeWihthMaxError ( ps,prefixVec[temp],MaxError,true);
        cout<<toString(temp)<<endl;
        //prefixVec[temp].Show();
        ps.Show();
        time_now = boost::posix_time::microsec_clock::local_time(); 
        cout<<"MatchEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
        i=i+k-1;
        k=1;
    }
    time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"CandidatesEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
    /*
    for(unsigned i=0; i<ps.Candidates.size(); i++)
    {
        for(unsigned j=0; j<ps.Candidates[i].k.size(); j++)
        {
            ret.push_back(GetText(ps.Candidates[i].k[j].ID));
        }
    }
    */

    vector<uint32_t> IDs=MergeSort (ps.Candidates);//Multi-way merge sort
    //cout<<IDs.size()<<endl;
    time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"InsertEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
    //std::sort(ret.begin(),ret.end());
    vector<uint32_t>::iterator end_unique=unique(IDs.begin(),IDs.end());
    IDs.erase(end_unique,IDs.end());
    time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"removeEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
    //cout<<IDs.size()<<endl;
    vector<UString> result;
    for(unsigned i=0; i<IDs.size(); i++)
    {
        UString ustr=GetText(IDs[i]);
        //cout<<toString(ustr)<<endl;
        if(ustr.length()>query.length()+MaxError||query.length()>ustr.length()+MaxError)
            continue;
        if (EditDistance(ustr,query)<=MaxError)
            result.push_back(ustr);
    }
    //cout<<result.size()<<endl;
    time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"filterEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
    return result;
    /*
       return ret;
    */
}


void MatchIndex::Save(ostream &ofs)
{

    size_t size=Context.size();
    ofs.write((const char*)&size,sizeof(size));

    for(int i=0; i<Context.size(); i++)
    {

        std::string str;
        Context[i].convertString(str, UString::UTF_8);
        size_t len=sizeof(str[0])*(str.size());
        ofs.write((const char*)&len, sizeof(len));
        ofs.write((const char*)&str[0], sizeof(str[0])*(str.size()) );
        //Context[i];
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
        //f<<Context[i];
    }


}
void MatchIndex::Load(istream &ifs)
{
    size_t size;
    ifs.read(( char*)&size, sizeof(size));
    Context.resize(size);
    size_t len;
    //cout<<"Context"<<size<<endl;
    for(int i=0; i<size; i++)
    {
        ifs.read(( char*)&len, sizeof(len));
        std::string str;
        str.resize(len);
        ifs.read(( char*)&str[0], len);
        //cout<<"context"<<str<<endl;
        Context[i]=UString(str,UString::UTF_8);
    }
    ifs.read(( char*)&size, sizeof(size));
    //cout<<"prefixVec"<<size<<endl;
    for(int i=0; i<size; i++)
    {
        size_t len;
        ifs.read(( char*)&len, sizeof(len));
        std::string str;
        str.resize(len);
        ifs.read(( char*)&str[0], len);
        //cout<<"prefix"<<str<<str.size()<<endl;
        PostingList ps;
        ifs>>ps;
        //cout<<"psload"<<endl;
        prefixVec.insert(make_pair(UString(str,UString::UTF_8), ps) );

    }
    cout<<"Context"<<Context.size()<<endl;
    cout<<"prefixVec"<<prefixVec.size()<<endl;
    build=true;
}
int  MatchIndex::EditDistance(UString source,UString target)
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

UString MatchIndex::GetText(int num)
{
    return Context[num];
}
int MatchIndex::AddText(UString text)
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
/**/



};



};
