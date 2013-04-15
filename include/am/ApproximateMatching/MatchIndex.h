#if !defined(_MATCH_INDEX_)
#define _MATCH_INDEX_

#include <util/ustring/UString.h>
#include <vector>

//#include <ir/id_manager/IDManager.h>
//#include <am/succinct/fm-index/wavelet_matrix.hpp>
//#include "bit_trie.hpp"
//#include <am/ApproximateMatching/wavelet_matrix.hpp>
#include <list>
#include <fstream>
#include <math.h>
//using namespace boost;

#define MaxDepth 3  //Mean the max length of segment in a pattern
#define MaxListSize 1000 //Increase MaxListSize can Save memory and build time,but will lose the search performance.MaxListSize can't too large or it
#define LargeNum 100000000000
#define MaxErrorDepth 0 //0 means tradional Q-gram, >0 means  postlist can permit errors in it;
namespace izenelib
{
namespace am
{
struct OneSubstr
{
    uint32_t ID;
    uint32_t BeginPos;
    uint32_t EndPos;
    void Show()
    {
        cout<<"ID"<<ID<<"BeginPos"<<BeginPos<<"EndPos"<<EndPos<<endl;
    }
};
bool operator < (const OneSubstr& left ,const OneSubstr& right)
{
    return left.ID==right.ID?(left.BeginPos==right.BeginPos?(left.EndPos<right.EndPos):left.BeginPos<right.BeginPos):left.ID<right.ID;
}
bool operator == (const OneSubstr& left ,const OneSubstr& right)
{
    return left.ID==right.ID&&left.BeginPos==right.BeginPos&&left.EndPos==right.EndPos;
}
struct Candidate
{
    std::vector<OneSubstr> k;
    void Show()
    {
        cout<<"candidate num"<<k.size()<<endl;
    }

};
vector<Candidate> BeginWith  ( Candidate& right,int startPos)
{
    vector<Candidate> answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=right.k.begin(),SubstrIter_a_end=right.k.end();
    while(SubstrIter_a < SubstrIter_a_end)
    {
        if(SubstrIter_a->BeginPos<size_t(startPos+1))
        {
            answer.resize(max(size_t(1),answer.size()));
            answer[0].k.push_back(*SubstrIter_a);
            ++SubstrIter_a;
            continue;
        }
        if(SubstrIter_a->BeginPos-startPos>answer.size())
        {
            answer.resize(SubstrIter_a->BeginPos-startPos);
        }
        answer[SubstrIter_a->BeginPos-startPos-1].k.push_back(*SubstrIter_a);
        ++SubstrIter_a;
    }
    return answer;
}
vector<Candidate> MergeInOrder ( Candidate& left , Candidate& right)
{
    vector<Candidate> answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_b=right.k.begin(),SubstrIter_a_end=left.k.end(),SubstrIter_b_end=right.k.end();
    while(SubstrIter_a < SubstrIter_a_end && SubstrIter_b < SubstrIter_b_end)
    {
        if((*SubstrIter_a).ID< (*SubstrIter_b).ID)
        {
            ++SubstrIter_a;
        }
        else if((*SubstrIter_b).ID < (*SubstrIter_a).ID)
        {
            ++SubstrIter_b;
        }
        else
        {
            if(SubstrIter_b->BeginPos<SubstrIter_a->EndPos+1);
            else
            {
                if(SubstrIter_b->BeginPos-SubstrIter_a->EndPos>answer.size())
                    answer.resize(SubstrIter_b->BeginPos-SubstrIter_a->EndPos);
                OneSubstr mergeSubstr;
                mergeSubstr.ID=  SubstrIter_a->ID;
                mergeSubstr.BeginPos= SubstrIter_a->BeginPos;
                mergeSubstr.EndPos= SubstrIter_b->EndPos;
                answer[SubstrIter_b->BeginPos-SubstrIter_a->EndPos-1].k.push_back(mergeSubstr);
            }
            if((*SubstrIter_a).EndPos+1 < (*SubstrIter_b).BeginPos)
                ++SubstrIter_a;
            else
                ++SubstrIter_b;
        }
    }
    return answer;


}
vector<uint32_t> MergeSort ( vector<Candidate> input)//Multi-way merge sort
{
    vector<uint32_t>  answer;
    if(input.size()==0)
    {
        return answer;
    }
    std::vector< std::vector<OneSubstr>::iterator> iter,iterend;
    iter.resize(input.size());
    iterend.resize(input.size());
    for(int i=0;i<input.size();i++)
    {
        iter[i]=input[i].k.begin();
        iterend[i]=input[i].k.end();
    }
    while(true)
    {
        int IDNow =LargeNum;
        int j = 0;
        for(int i = 0; i < input.size(); ++i)
        {
              if(iter[i]!=iterend[i] && iter[i]->ID <= IDNow)
              {
                    IDNow = iter[i]->ID;
                    j=i;
              }
        }
        if(j == 0 && iter[0]==iterend[0]) break; 
        iter[j]++;
        answer.push_back(IDNow);
    }
    return answer;


}
int RBinarySearchIntersection ( OneSubstr& x,     std::vector<OneSubstr>  &right,int front,  vector<Candidate> &answer)
{

    int end=right.size()-1;
    int mid=(front+end)/2;
    //<<"RBinarySearchIntersection"<<front<<" "<<end<<endl;
    while(front<end&&right[mid].ID!=x.ID)
    {
       if(right[mid].ID<x.ID)front=mid+1;
       if(right[mid].ID>x.ID)end=mid-1;
       mid=front + (end - front)/2;
    }
    if(right[mid].ID!=x.ID)
    {
        return mid;
    }
    while(right[mid].ID==x.ID)
    {
       mid--;
       if(mid<0)
       {
          break;
       }
    }
    mid++;
    int orginmid=mid;
    while(right[mid].ID==x.ID)//&&(x.EndPos+1 < right[mid].BeginPos))
    {
            if(right[mid].BeginPos<x.EndPos+1);
            else
            {

                if(right[mid].BeginPos-x.EndPos>answer.size())
                    answer.resize(right[mid].BeginPos-x.EndPos);
                OneSubstr mergeSubstr;
                mergeSubstr.ID=  x.ID;
                mergeSubstr.BeginPos= x.BeginPos;
                mergeSubstr.EndPos= right[mid].EndPos;
                answer[right[mid].BeginPos-x.EndPos-1].k.push_back(mergeSubstr);
            }
            mid++;
            if(mid<0||mid>right.size()-1) break;

    }
    return orginmid;
    
}
int LBinarySearchIntersection ( OneSubstr& x,     std::vector<OneSubstr>  &right,int front,  vector<Candidate> &answer)
{

    int end=right.size()-1;
    int mid=(front+end)/2;
    while(front<end&&right[mid].ID!=x.ID)
    {
       if(right[mid].ID<x.ID)front=mid+1;
       if(right[mid].ID>x.ID)end=mid-1;
       mid=front + (end - front)/2;
    }
    if(right[mid].ID!=x.ID)
    {
        return mid;
    }
    while(right[mid].ID==x.ID)
    {
       mid--;
       if(mid<0) break;
    }
    mid++;
    int orginmid=mid;
    while(right[mid].ID==x.ID&&( right[mid].EndPos+1 < x.BeginPos))
    {
            if(x.BeginPos<right[mid].EndPos+1);
            else
            {
                if(x.BeginPos-right[mid].EndPos>answer.size())
                    answer.resize(x.BeginPos-right[mid].EndPos);
                OneSubstr mergeSubstr;
                mergeSubstr.ID=  right[mid].ID;
                mergeSubstr.BeginPos= right[mid].BeginPos;
                mergeSubstr.EndPos= x.EndPos;
                answer[x.BeginPos-right[mid].EndPos-1].k.push_back(mergeSubstr);
            }
            mid++;
            if(mid>=right.size())break;
    }
    return orginmid;
    
}
vector<Candidate> BinarySearchIntersection ( Candidate& left , Candidate& right, bool rightmore)
{
    vector<Candidate> answer;
    int front=0;
    for(size_t i=0;i<left.k.size();i++)
    {
        if(rightmore)
         front=RBinarySearchIntersection ( left.k[i], right.k,front,answer);
        else
         front=LBinarySearchIntersection ( left.k[i], right.k, front,answer);
    }
    return answer;
}

vector<Candidate> operator & ( Candidate& left , Candidate& right)
{
    /*
    vector<Candidate> answer;
    size_t leftsize=left.k.size();
    size_t rightsize=right.k.size();
    if(leftsize==0||rightsize==0)
    {
         return answer;
    }

    if(leftsize*(log(rightsize)/log(2))-leftsize<rightsize+leftsize)
    {
        
         return  BinarySearchIntersection ( left , right, true);
    }
    
    if(rightsize*(log(leftsize)/log(2))-leftsize<leftsize+rightsize)  time_now = boost::posix_time::microsec_clock::local_time(); 
    cout<<"MatchEnd"<<boost::posix_time::to_iso_string(time_now)<<endl;
    {
         return  BinarySearchIntersection ( right , left, false);
    }
    */
    return  MergeInOrder ( left , right);
    
}

Candidate operator >> (Candidate left ,size_t n)
{
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_a_end=left.k.end();
    while(SubstrIter_a < SubstrIter_a_end)
    {

        (*SubstrIter_a).EndPos+=n;
        ++SubstrIter_a;
    }
    return left;
}
Candidate operator << (Candidate left ,size_t n)
{
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_a_end=left.k.end();
    while(SubstrIter_a < SubstrIter_a_end)
    {

        (*SubstrIter_a).BeginPos-=((*SubstrIter_a).BeginPos<n)?(*SubstrIter_a).BeginPos:n;
        ++SubstrIter_a;
    }
    return left;
}

Candidate operator | ( Candidate left , Candidate right)
{

    Candidate answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_b=right.k.begin(),SubstrIter_a_end=left.k.end(),SubstrIter_b_end=right.k.end();
    OneSubstr last;
    last.ID=LargeNum;
    last.BeginPos=0;
    last.EndPos=0;
    while(SubstrIter_a < SubstrIter_a_end&&SubstrIter_b < SubstrIter_b_end)
    {

        if((*SubstrIter_a)< (*SubstrIter_b))
        {
            if(*SubstrIter_a==last);
            else
            {
                answer.k.push_back(*SubstrIter_a);
                last=*SubstrIter_a;
            }
            ++SubstrIter_a;
        }
        else
        {
            if((*SubstrIter_b)<(*SubstrIter_a))
            {
                if(*SubstrIter_b==last);
                else
                {
                    answer.k.push_back(*SubstrIter_b);
                    last=*SubstrIter_b;
                }
            }
            ++SubstrIter_b;
        }
    }
    if (SubstrIter_a < SubstrIter_a_end)
    {
        while(SubstrIter_a < SubstrIter_a_end)
        {
            if(*SubstrIter_a==last);
            else
            {
                answer.k.push_back(*SubstrIter_a);
                last=*SubstrIter_a;
            }
            ++SubstrIter_a;
        }
    }
    else if (SubstrIter_b < SubstrIter_b_end)
    {
        while(SubstrIter_b < SubstrIter_b_end)
        {
            if(*SubstrIter_b==last);
            else
            {
                answer.k.push_back(*SubstrIter_b);
                last=*SubstrIter_b;
            }
            ++SubstrIter_b;
        }
    }
    return answer;

}
struct PostingList
{
    uint32_t MaxError;
    uint32_t StrLength;
    std::vector<Candidate> Candidates;
    uint32_t Size()
    {
        uint32_t j=0;
        for(uint32_t i=0; i<Candidates.size(); i++)
        {
            j+=Candidates[i].k.size();
        }
        return j;
    }
    void Show()
    {
        cout<<"Size"<<Size()<<endl;
        cout<<"MaxError"<<MaxError<<endl;
        cout<<"Candidates"<<Candidates.size()<<endl;
        for(uint32_t i=0; i<Candidates.size(); i++)
        {
            cout<<"Error"<<i;
            Candidates[i].Show();
        }
    }
};
PostingList BeginWihthMaxError ( PostingList& right,uint32_t MaxError,int startPos=-1)
{
    PostingList ret;
    ret.MaxError=right.MaxError;
    //cout<<"    ret.MaxError"<<right.MaxError<<endl;
    ret.StrLength=right.StrLength;
    ret.Candidates.resize(MaxError+1);

    for(size_t j=0; j<right.Candidates.size(); j++)
    {
        if(j>MaxError) break;
        vector<Candidate>  mergeResult=BeginWith(right.Candidates[j],startPos);

        for(size_t k=j; k<ret.Candidates.size(); k++)
        {
            if(k-j<mergeResult.size())
            {
                ret.Candidates[k]=ret.Candidates[k]|mergeResult[k-j];
            }
        }

    }


    return ret;
}
PostingList mergeWihthMaxError ( PostingList& left , PostingList& right,uint32_t MaxError,bool match=false)
{
    if(left.StrLength==0&&match)
    {
        return  BeginWihthMaxError(right,MaxError);
    }
    PostingList ret;
    uint32_t leftLength=left.StrLength;
    uint32_t rightLength=right.StrLength;
    //if(!match)
    //     ret.MaxError=MaxError;
    //else
    ret.MaxError=left.MaxError+right.MaxError+1;
    ret.StrLength=leftLength+rightLength;
    ret.Candidates.resize(MaxError+1);
    for(size_t i=0; i<left.Candidates.size(); i++)
    {
        for(size_t j=0; j<right.Candidates.size(); j++)
        {
            if(i+j>MaxError) break;
            vector<Candidate>  mergeResult=left.Candidates[i]&right.Candidates[j];
            cout<<"mergeResult"<<i<<"  "<<j<<endl;
            for(size_t k=0; k<min(mergeResult.size(),size_t(3)); k++)
            {
                 cout<<mergeResult[k].k.size()<<endl;
            }
            for(size_t k=i+j; k<ret.Candidates.size(); k++)
            {

                if(k-i-j<mergeResult.size())
                {
                    ret.Candidates[k]=ret.Candidates[k]|mergeResult[k-i-j];
                }
            }
        }
    }
    cout<<"CandidatesCorrect"<<ret.Candidates[0].k.size()<<endl;
    //cout<<" 1"<<endl;
    for(size_t i=0; i<left.Candidates.size(); i++)
    {
        //cout<<" 2"<<endl;
        if(MaxError>= i+right.Candidates.size())
        {
            for(size_t k=0; k<=rightLength; k++)
                ret.Candidates[right.Candidates.size()+i]=(ret.Candidates[right.Candidates.size()+i]|(left.Candidates[i]>>k));
        }
    }
    for(size_t j=0; j<right.Candidates.size(); j++)
    {
        //cout<<" 3"<<endl;
        //cout<<"    for(size_t j=0; j<right.Candidates.size(); j++)"<<endl;
        //cout<<" j"<<j<<"MaxError"<<MaxError<<"left.MaxError"<<left.MaxError<<endl;
        if(MaxError>= j+left.MaxError+1)
        {
            //cout<<"           if(MaxError>= j+left.Candidates.size()+1) "<<endl;
            if(match)
            {
                int StartPos=leftLength-1;
                vector<Candidate>  mergeResult=BeginWith(right.Candidates[j],StartPos);
                //cout<<"startpos"<<StartPos<<"mergeResult"<<mergeResult[0].k.size()<<endl;
                for(size_t k=0; k<mergeResult.size(); k++)
                {
                    if(j+left.MaxError+1+k<=MaxError)
                    {
                        ret.Candidates[j+left.MaxError+1+k]=ret.Candidates[j+left.MaxError+1+k]|mergeResult[k];
                    }
                }
            }
            else
            {
                for(size_t k=0; k<=leftLength; k++)
                    ret.Candidates[left.Candidates.size()+j]=(ret.Candidates[left.Candidates.size()+j]|(right.Candidates[j]<<k));
            }
        }
    }
    return ret;

}

PostingList operator | ( PostingList& left , PostingList& right)
{

    return mergeWihthMaxError(left,right,min(left.MaxError+right.MaxError+1,left.Size()+right.Size()>MaxListSize?0:uint32_t(MaxErrorDepth)));
}



std::ostream& operator<<(std::ostream &f, const OneSubstr &mi)
{
    f.write((const char*)&mi.ID, sizeof(mi.ID));
    f.write((const char*)&mi.BeginPos, sizeof(mi.BeginPos));
    f.write((const char*)&mi.EndPos, sizeof(mi.EndPos));
    return f;

}

std::istream& operator>>(std::istream &f, OneSubstr &mi )
{
    f.read(( char*)&mi.ID, sizeof(mi.ID));
    f.read(( char*)&mi.BeginPos, sizeof(mi.BeginPos));
    f.read(( char*)&mi.EndPos, sizeof(mi.EndPos));
    return f;
}

std::ostream& operator<<(std::ostream &f, const Candidate &mi)
{
    size_t size=mi.k.size();
    f.write((const char*)&size,sizeof(size));
    for(size_t i=0; i<mi.k.size(); i++)
    {
        f<<mi.k[i];
    }
    return f;

}

std::istream& operator>>(std::istream &f, Candidate &mi )
{
    size_t size;
    f.read(( char*)&size, sizeof(size));
    mi.k.resize(size);
    for(size_t i=0; i<size; i++)
    {
        f>>mi.k[i];
    }
    return f;
}
std::ostream& operator<<(std::ostream &f, const PostingList &mi)
{
    f.write((const char*)&mi.MaxError, sizeof(mi.MaxError));
    f.write((const char*)&mi.StrLength, sizeof(mi.StrLength));
    size_t size=mi.Candidates.size();
    f.write((const char*)&size,sizeof(size));
    for(size_t i=0; i<mi.Candidates.size(); i++)
    {
        f<<mi.Candidates[i];
    }
    return f;

}

std::istream& operator>>(std::istream &f, PostingList &mi )
{
    f.read((char*)&mi.MaxError, sizeof(mi.MaxError));
    f.read((char*)&mi.StrLength, sizeof(mi.StrLength));

    size_t size;
    f.read(( char*)&size, sizeof(size));
    mi.Candidates.resize(size);
    for(size_t i=0; i<size; i++)
    {

        f>>mi.Candidates[i];
    }
    return f;
}


class  MatchIndex
{


    std::map<UString,PostingList > prefixVec;
    vector<UString > Context;
    bool build;
public:
    MatchIndex();
    ~MatchIndex();
    void Add(UString text);
    void Add(vector<UString> textVec);
    void BuildIndex();
    bool Hasbuild();
    vector<UString> Match(UString  query,int MaxError);
    void Save(ostream &ofs);
    void Load(istream &ifs);
    int  EditDistance(UString source,UString target);
    void Show();
    void Clear();
    friend istream& operator>> ( istream &f, MatchIndex &mi );
    friend ostream& operator<< ( ostream &f, const MatchIndex &mi );
private:
    bool getCandidate(vector< vector<std::pair<size_t,UString> > > &Matrix,size_t i,unsigned j,vector<uint32_t>& ret);
    UString GetText(int num);
    int AddText(UString text);
    int TextSize();


};




std::ostream& operator<<(std::ostream &f, MatchIndex &mi)
{
    mi.Save(f);
    return f;

}

std::istream& operator>>(std::istream &f, MatchIndex &mi )
{
    mi.Load(f);
    return f;
}
};

};
/**/
#endif
