
#include <am/ApproximateMatching/MatchIndex.h>

using namespace std;
//using namespace boost;
using namespace izenelib;
using namespace izenelib::util;
//using namespace izenelib::am::succinct::fm_index;
string toString(UString us)
{
    string str;
    us.convertString(str, izenelib::util::UString::UTF_8);
    return str;
}

MatchIndex::MatchIndex()
{
   cmp.resize(MaxDepth);
   for(unsigned i=0;i<MaxDepth;i++)
   {
      cmp[i].depth=i;
   }
}
MatchIndex::~MatchIndex()
{
}
void MatchIndex::add(UString text)
{
   uint32_t id=AddText(text);
   int length=text.length();
   for(unsigned i=0;i<MaxDepth;i++)
   text.append(UString(" ",izenelib::util::UString::UTF_8));
   for(unsigned i=0;i<length;i++)
   prefixVec.push_back(std::make_pair(text.substr(i,MaxDepth),id));
   
}

void MatchIndex::add(vector<UString> textVec)
{
   for(unsigned i=0;i<=textVec.size();i++)
   add(textVec[i]);
}


void MatchIndex::BuildIndex()
{
   cout<<"sort   ";
   sort();
   cout<<"BuildWavletTree   "<<endl;
   BuildWavletTree();
   cout<<"BuildIndex done  "<<endl;
}
vector<UString> MatchIndex::Match(UString  query,int MaxError)
{
    vector<UString> ret;
   cout<<"DynamicSpilt   "<<endl;
    vector<uint32_t> can=DynamicSpilt(query,MaxError);
   cout<<"DynamicSpilt  end   "<<endl;
    for(unsigned i=0;i<can.size();i++)
    {  
         if (EditDistance(GetText(can[i]),query)<MaxError)
         ret.push_back(GetText(can[i]));
    }
    return ret;
}

vector<uint32_t> MatchIndex::DynamicSpilt(UString  query,unsigned MaxError)
{
    vector< vector<std::pair<size_t,UString> > > Matrix;
    Matrix.resize(query.length());
    for(size_t i=0;i<Matrix.size();i++)
    {
       Matrix[i].resize(MaxError+1);
    }
    UString EmptyUstr;
    cout<<"MatrixD"<<endl;
    for(int i=0;i<Matrix.size();i++)
    {
            //cout<<"i"<<i<<endl;
        for(size_t j=0;j<Matrix[i].size();j++)
        {
            //cout<<"j"<<j;
            Matrix[i][j]=make_pair(9999,EmptyUstr);
            if(j>i) 
            {
                          
                          if(i==0) Matrix[i][j].second=(query.substr(i,1));
                          else
                          {
                             Matrix[i][j]=Matrix[i-1][j];
                             Matrix[i][j].second=(query.substr(i,1));
                          }
                          //cout<<Matrix[i][j].second.size()<<"   ";
                          cout<<Matrix[i][j].first<<"   ";
                          continue;
            }  
            else
            {
               if(j==0)
               {
                  if(i<MaxDepth)
                  {
                    Matrix[i][j].first=getNum(query.substr(0,i-0+1));
                    Matrix[i][j].second=(query.substr(0,i-0+1));
                  }
                  else
                  {
                    for(size_t k=max(1,i+1-MaxDepth);k<=i-1;k++)
                    {                              
                       if(min(Matrix[k][j].first,getNum(query.substr(k,i-k)) )< Matrix[i][j].first)//TODO
                       {
                            Matrix[i][j].first=min(Matrix[k][j].first,getNum(query.substr(k,i-k)) );
                            Matrix[i][j].second=(query.substr(k,i-k)  );
                       }
                    }
                  }
                  
               }
               else
               {
                    for(size_t k=max(0,i+1-MaxDepth);k<=i-1;k++)
                    {    

                       if(min(Matrix[k][j].first,getNum(query.substr(k,i-k)))+Matrix[k][j-1].first< Matrix[i][j].first)//TODO
                       {

                            Matrix[i][j].first=min(Matrix[k][j].first,getNum(query.substr(k,i-k)))+Matrix[k][j-1].first;
                            Matrix[i][j].second=(query.substr(k,i-k)  );

                       }

                    }
                   
               }

            }
            //cout<<Matrix[i][j].second.size()<<"   ";
            cout<<Matrix[i][j].first<<"   ";
            if(Matrix[i][j].first==0)
                 cout<<toString(Matrix[i][j].second)<<endl;;
        }
         cout<<endl;
    }
    //cout<<"num"<<Matrix[Matrix.size()-1][MaxError].first<<endl;
    for(int i=0;i<Matrix.size();i++)
    {
        for(size_t j=0;j<Matrix[i].size();j++)
        {
            Matrix[i][j].first=9999;
        }
    }
    vector<uint32_t> ret;

    getCandidate(Matrix,Matrix.size()-1,MaxError,ret);
    //for(unsigned i=0;i<ret.size();i++)
     // cout<<"candidate"<<toString(GetText(ret[i]))<<endl;
    for(int i=0;i<Matrix.size();i++)
    {
        for(size_t j=0;j<Matrix[i].size();j++)
        {
             //cout<<Matrix[i][j].first<<"   ";
        }
        // cout<<endl;
    }
    return ret;
}
bool MatchIndex::getCandidate(vector< vector<std::pair<size_t,UString> > > &Matrix,size_t i,unsigned j,vector<uint32_t>& ret)
{
       //cout<<"i"<<i<<"j"<<j<<endl;
       if(j==0)
       {
           UString ustr=Matrix[i][j].second;
           BitTrie filter(prefix.size());
           wa_->QuantileRangeAll(getLowBound(ustr), getUperBound(ustr), ret,filter,false);
           Matrix[i][j].first=ret.size();
           //cout<<"i"<<i<<"j"<<j<<ret.size()<<"str"<<toString(ustr)<<getLowBound(ustr)<<" "<<getUperBound(ustr)<<endl;
           return true;
       }
       else if(i<j)
       {
           //cout<<"i"<<i<<"j"<<j<<"无穷"<<endl;
          Matrix[i][j].first=9999;
           return false; 
       }
       else
       {
           UString ustr=Matrix[i][j].second;
           unsigned k=i-ustr.length();
           BitTrie filter(prefix.size());
           vector<uint32_t> errorFilter;
           bool get=getCandidate(Matrix,k,j,errorFilter);
           filter.insert(errorFilter);

           wa_->QuantileRangeAll(getLowBound(ustr), getUperBound(ustr), ret,filter,get);

           vector<uint32_t> retadd;
           if(!getCandidate(Matrix,k,j-1,retadd))
           {
               cout<<"error"<<endl;
           }

           ret.insert(ret.end(),retadd.begin(),retadd.end());
           std::sort(ret.begin(),ret.end());  
           vector<uint32_t>::iterator end_unique=unique(ret.begin(),ret.end());  
           ret.erase(end_unique,ret.end());
           //cout<<"i"<<i<<"j"<<j<<ret.size()<<endl;
           Matrix[i][j].first=ret.size();
           return true;
       }

    
}
void MatchIndex::save(ofstream ofs)
{
}
void MatchIndex::load(ifstream ifs)
{
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
    for(int i=1;i<=n;i++) matrix[i][0]=i;
    for(int i=1;i<=m;i++) matrix[0][i]=i;

     for(int i=1;i<=n;i++)
     {
        const UCS2Char si=source[i-1];
        for(int j=1;j<=m;j++)
        {
            const UCS2Char dj=target[j-1];
            int cost;
            if(si==dj){
                cost=0;
            }
            else{
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

void MatchIndex::sort()
{
    sortInDepth(0,prefixVec.size() ,0);
    cout<<"done"<<endl;
}
void MatchIndex::BuildWavletTree()
{
    vector<uint32_t> wavePrepare;
    for(unsigned i=0;i<prefixVec.size();i++)
    {
       wavePrepare.push_back(prefixVec[i].second);
       prefix.push_back(prefixVec[i].first);
    }
    wa_=new izenelib::WaveletMatrix(TextSize(), false, true);
    wa_->build(&wavePrepare[0],wavePrepare.size());
}
size_t  MatchIndex::getLowBound(UString pre)
{
       int front=0;
       int end=prefix.size()-1;
       int mid=(front+end)/2;
       while(front<end-1)
       {
          if(prefix[mid]<pre)front=mid;
          else if(prefix[mid]==pre) break;
          else {end=mid;}
          mid=front + (end - front)/2;
       } 
       if(prefix[mid]<pre)     
       return mid+1;
       else
       {
          while(prefix[mid]==pre)
          mid--;
       }
       return mid+1;
      
}
size_t  MatchIndex::getUperBound(UString pre)
{
/*
       pre.append(UString("zzzz",izenelib::util::UString::UTF_8));
       return  getLowBound(pre)-1;
*/
       int front=0;
       int end=prefix.size()-1;
       int mid=(front+end)/2;
       while(front<end-1)
       {
          if(prefix[mid].substr(0,pre.length())<pre||prefix[mid].substr(0,pre.length())==pre)front=mid;
          else {end=mid;}
          mid=front + (end - front)/2;
       } 
       while(prefix[mid].substr(0,pre.length())<pre||prefix[mid].substr(0,pre.length())==pre)
       {
            mid++;
       }
       return mid-1;
}
std::pair<size_t, size_t>  MatchIndex::getBoundary(UString pre)
{
       return  make_pair(getLowBound(pre),getUperBound(pre));
}
size_t  MatchIndex::getNum(UString pre)
{
       return  getUperBound(pre)-getLowBound(pre)+1;
}
void MatchIndex::sortInDepth(unsigned start,unsigned end,unsigned depth)
{
   //cout<<" "<<start<<" "<<end<<" "<<depth<<endl;
   if(start<0||end>prefixVec.size()||start>end||end-start+1<v||depth>=MaxDepth)
   return;
   //cout<<" d"<<start<<" "<<end<<" "<<depth<<endl;
   std::sort(prefixVec.begin()+start,prefixVec.begin()+end,cmp[depth]);
   UCS2Char begin=prefixVec[start].first[depth];
   UCS2Char now;
   unsigned nextstart=start;
   for(unsigned i=start+1;i<end;i++)
   {
      now=prefixVec[i].first[depth];
      if(now!=begin)
      {
         sortInDepth(nextstart,i,depth+1);
         nextstart=i;
         begin=now;
      }

   }
   sortInDepth(nextstart,end,depth+1);
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
void MatchIndex::show()
{
/*
    for(unsigned i=0;i<prefixVec.size();i++)
    {  
       cout<<toString(prefixVec[i].first)<<"   "<<toString(GetText(prefixVec[i].second))<<endl;
    }
    BuildWavletTree();
    size_t up=getLowBound(UString("de",izenelib::util::UString::UTF_8)), down=getUperBound(UString("de",izenelib::util::UString::UTF_8));
    cout<<"wavelet tree begin"<<endl;
    cout<<up<<"  "<<down<<endl;
    for(unsigned i=up;i<=down;i++)
    {  
       cout<<toString(prefix[i])<<" "<<toString(GetText(wa_->access(i)))<<endl;
    }

    BitTrie  SubTrie(prefixVec.size());
    SubTrie.insert(4);
    SubTrie.insert(3);
    SubTrie.insert(2);
    SubTrie.insert(1);
    SubTrie.insert(0);
    up=getLowBound(UString("dsad",izenelib::util::UString::UTF_8)), down=getUperBound(UString("dsad",izenelib::util::UString::UTF_8));
    cout<<"wavelet tree begin2"<<endl;
    cout<<up<<"  "<<down<<endl;
vector<uint32_t> ret;
           wa_->QuantileRangeAll(16, 16, ret,SubTrie,true);
    for(unsigned i=0;i<ret.size();i++)
    {  
         cout<<ret[i]<<toString(GetText(ret[i]))<<endl;
    }
    UString query("dsadfegfa",izenelib::util::UString::UTF_8); 
    vector<uint32_t> can=DynamicSpilt(query,4);
    cout<<"dasdas"<<endl;
    for(unsigned i=0;i<can.size();i++)
    {  
         cout<<can[i]<<toString(GetText(can[i]))<<endl;
    }

    vector<UString> can=Match(UString("dsadfegfa",UString::UTF_8),5);
    for(unsigned i=0;i<can.size();i++)
    {  
         cout<<toString(can[i])<<endl;
    }
*/
}
/**/

