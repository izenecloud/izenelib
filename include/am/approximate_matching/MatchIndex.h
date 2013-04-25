#if !defined(_MATCH_INDEX_)
#define _MATCH_INDEX_

#include <util/ustring/UString.h>
#include <vector>
#include <list>
#include <fstream>
#include <math.h>
//using namespace boost;

#define MaxDepth  3 //Mean the max length of segment in a pattern
#define MaxListSize 1000 //Increase MaxListSize can Save memory and build time,but will lose the search performance.MaxListSize can't too large or it
#define LargeNum 100000000
#define MaxErrorDepth 0 //0 means tradional Q-gram, >0 means  postlist can permit errors in it;

//#define TimeDebug
boost::posix_time::time_duration ortime ;
boost::posix_time::time_duration andtime ;
boost::posix_time::time_duration uniquetime ;
boost::posix_time::time_duration movetime ;
boost::posix_time::time_duration begintime ;
boost::posix_time::time_duration runtime ;
boost::posix_time::time_duration mergesorttime ;
boost::posix_time::time_duration amtime ;
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
bool IDEqual (const OneSubstr& left ,const OneSubstr& right)
{
    return left.ID==right.ID;
}
OneSubstr operator & (const OneSubstr& left ,const OneSubstr& right)
{
    OneSubstr ret;
    ret.ID=left.ID;
    ret.BeginPos=min(left.BeginPos,right.BeginPos);
    ret.EndPos=max(left.EndPos,right.EndPos);
    return ret;
}
struct Candidate
{
    std::vector<OneSubstr> k;
    void Show()
    {
        cout<<"candidate num"<<k.size()<<endl;
    }

};
void BeginWith  ( Candidate& right,int startPos,vector<Candidate>& answer,int maxError=0)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif

    //vector<Candidate> answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=right.k.begin(),SubstrIter_a_end=right.k.end();
    answer.resize(50);
    while(SubstrIter_a < SubstrIter_a_end)
    {
        if(SubstrIter_a->BeginPos<size_t(startPos+1))
        {

            if(SubstrIter_a->BeginPos<size_t(startPos+1-maxError))
            {
                if(answer.size()<startPos+1-maxError-SubstrIter_a->BeginPos+1)
                    answer.resize(startPos+1-maxError-SubstrIter_a->BeginPos+1);
                answer[startPos+1-maxError-SubstrIter_a->BeginPos].k.push_back(*SubstrIter_a);
            }
            else
            {
                answer[0].k.push_back(*SubstrIter_a);
            }
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
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    begintime =begintime+( time_then - time_now);//
#endif

    //return answer;
}
vector<Candidate> MergeInOrder ( Candidate& left , Candidate& right)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
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
            if(SubstrIter_b->BeginPos<SubstrIter_a->EndPos+1)
            {
                if(SubstrIter_b->BeginPos>SubstrIter_a->BeginPos+1)
                {
                    if(answer.size()==0)
                    {
                        answer.resize(1);
                    }
                    answer[0].k.push_back((*SubstrIter_b)&(*SubstrIter_a));
                }
            }
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
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    andtime =andtime+( time_then - time_now);//
#endif
    return answer;


}
vector<uint32_t> MergeSort ( vector<Candidate> input)//Multi-way merge sort
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    vector<uint32_t>  answer;
    if(input.size()==0)
    {
        return answer;
    }
    std::vector< std::vector<OneSubstr>::iterator> iter,iterend;
    iter.resize(input.size());
    iterend.resize(input.size());
    for(size_t i=0; i<input.size(); i++)
    {
        iter[i]=input[i].k.begin();
        iterend[i]=input[i].k.end();
    }
    while(true)
    {
        size_t IDNow =LargeNum;
        size_t j = 0;
        for(size_t i = 0; i < input.size(); ++i)
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
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    mergesorttime =mergesorttime+( time_then - time_now);//
#endif
    return answer;


}
int RBinarySearchIntersection ( OneSubstr& x,     std::vector<OneSubstr>  &right,int front,  vector<Candidate> &answer)
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
        if(mid<0)
        {
            break;
        }
    }
    mid++;
    int orginmid=mid;
    while(right[mid].ID==x.ID)
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
        if(mid<0||mid>int(right.size()-1)) break;

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
        if(mid>=int(right.size()))break;
    }
    return orginmid;

}
vector<Candidate> BinarySearchIntersection ( Candidate& left , Candidate& right, bool rightmore)
{
    vector<Candidate> answer;
    int front=0;
    for(size_t i=0; i<left.k.size(); i++)
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

    vector<Candidate> answer;
    size_t leftsize=left.k.size();
    size_t rightsize=right.k.size();
    if(leftsize==0||rightsize==0)
    {
         return answer;
    }
/*
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
   // int ratel=left.k.size()/right.k.size()+1;
   // int rater=right.k.size()/left.k.size()+1;
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif


    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_b=right.k.begin(),SubstrIter_a_end=left.k.end(),SubstrIter_b_end=right.k.end();
    while(SubstrIter_a < SubstrIter_a_end && SubstrIter_b < SubstrIter_b_end)
    {
        if((*SubstrIter_a).ID< (*SubstrIter_b).ID)
        {
            SubstrIter_a++;
/*
            if((*(SubstrIter_a+ratel)).ID< (*SubstrIter_b).ID)
            SubstrIter_a+=ratel;
            else
            SubstrIter_a++;
*/
        }
        else if((*SubstrIter_b).ID < (*SubstrIter_a).ID)
        {
            SubstrIter_b++;
/*
            SubstrIter_b+=rater;
            if((*(SubstrIter_b+rater)).ID< (*SubstrIter_a).ID)
            SubstrIter_b+=rater;
            else
            SubstrIter_b++;
*/
        }
        else
        {
#ifdef TimeDebug
       //     boost::posix_time::ptime time_now2 = boost::posix_time::microsec_clock::local_time();
#endif
            if(SubstrIter_b->BeginPos<SubstrIter_a->EndPos+1)
            {
                if(SubstrIter_b->BeginPos>SubstrIter_a->BeginPos+1)
                {
                    if(answer.size()==0)
                    {
                        answer.resize(1);
                    }
                    answer[0].k.push_back((*SubstrIter_b)&(*SubstrIter_a));
                }
            }
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
#ifdef TimeDebug
         //   boost::posix_time::ptime time_then2 = boost::posix_time::microsec_clock::local_time();
         //   amtime =amtime+( time_then2 - time_now2);//
#endif
        }
    }
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    andtime =andtime+( time_then - time_now);//
#endif
    return answer;

   // return  MergeInOrder ( left , right);

}

Candidate& operator >> (Candidate& left ,size_t n)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_a_end=left.k.end();
    while(SubstrIter_a < SubstrIter_a_end)
    {

        (*SubstrIter_a).EndPos+=n;
        ++SubstrIter_a;
    }
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    movetime =movetime+( time_then - time_now);//
#endif
    return left;
}
Candidate& operator << (Candidate& left ,size_t n)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_a_end=left.k.end();
    while(SubstrIter_a < SubstrIter_a_end)
    {
        (*SubstrIter_a).BeginPos-=((*SubstrIter_a).BeginPos<n)?(*SubstrIter_a).BeginPos:n;
        ++SubstrIter_a;
    }
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    movetime =movetime+( time_then - time_now);//
#endif
    return left;
}
void unique  ( Candidate& left )
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    if(left.k.size()==0)
    {
        return;
    }
    Candidate answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_a_end=left.k.end();
    OneSubstr last=*SubstrIter_a;
    while(SubstrIter_a < SubstrIter_a_end)
    {
        if(IDEqual(*SubstrIter_a,last))
        {
            last=last&(*SubstrIter_a);
        }
        else
        {
            answer.k.push_back(last);
            last=*SubstrIter_a;
        }
        SubstrIter_a++;
    }
    answer.k.push_back(last);
    left=answer;
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    uniquetime =uniquetime+( time_then - time_now);//
#endif

}
Candidate operator | (Candidate& left ,Candidate& right)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    Candidate answer;
    std::vector<OneSubstr>::iterator SubstrIter_a=left.k.begin(),SubstrIter_b=right.k.begin(),SubstrIter_a_end=left.k.end(),SubstrIter_b_end=right.k.end();
    //cout<<"0"<<endl;
    OneSubstr last;
    if(SubstrIter_a < SubstrIter_a_end&&SubstrIter_b < SubstrIter_b_end)
    last=(*SubstrIter_a < *SubstrIter_b)?(*SubstrIter_a):(*SubstrIter_b);
    else  if(SubstrIter_a < SubstrIter_a_end&&SubstrIter_b==SubstrIter_b_end)
    last=(*SubstrIter_a);
    else  if(SubstrIter_a == SubstrIter_a_end&&SubstrIter_b < SubstrIter_b_end)
    last=(*SubstrIter_b);

    //cout<<"1"<<endl;
    while(SubstrIter_a < SubstrIter_a_end&&SubstrIter_b < SubstrIter_b_end)
    {
        //    cout<<"2"<<endl;
        if((*SubstrIter_a)< (*SubstrIter_b))
        {
            if(IDEqual(*SubstrIter_a,last)){ last=last&(*SubstrIter_a);}
            else
            {
                answer.k.push_back(last);
                last=*SubstrIter_a;
            }
            ++SubstrIter_a;
        }
        else
        {
            if((*SubstrIter_b)<(*SubstrIter_a))
            {
                if(IDEqual(*SubstrIter_b,last)){ last=last&(*SubstrIter_b);}
                else
                {
                    answer.k.push_back(last);
                    last=*SubstrIter_b;
                }
            }
            ++SubstrIter_b;
        }
    }
        //    cout<<"3"<<endl;
    if (SubstrIter_a < SubstrIter_a_end)
    {
        while(SubstrIter_a < SubstrIter_a_end)
        {
            //cout<<"4"<<endl;
            if(IDEqual(*SubstrIter_a,last)){ last=last&(*SubstrIter_a);}
            else
            {
                answer.k.push_back(last);
                last=*SubstrIter_a;
            }
            ++SubstrIter_a;
        }
    }
    else if (SubstrIter_b < SubstrIter_b_end)
    {
        while(SubstrIter_b < SubstrIter_b_end)
        {
            //cout<<"4"<<endl;
            if(IDEqual(*SubstrIter_b,last)){ last=last&(*SubstrIter_b);}
            else
            {
                answer.k.push_back(last);
                last=*SubstrIter_b;
            }
            ++SubstrIter_b;
        }
    }
    if(left.k.size()+right.k.size()>0)
         answer.k.push_back(last);
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    ortime =ortime+( time_then - time_now);//
#endif
    //unique  ( answer );
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
        cout<<"StrLength"<<StrLength<<endl;
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
        vector<Candidate>  mergeResult;
        BeginWith(right.Candidates[j],startPos,mergeResult);

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
//PostingList
void mergeWihthMaxError ( PostingList& left , PostingList& right,uint32_t MaxError,uint32_t beginPos=0,bool match=false)
{
#ifdef TimeDebug
    boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
#endif
    if(left.StrLength==0&&match)
    {
#ifdef TimeDebug
        boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
        runtime =runtime+( time_then - time_now);//
#endif
        left=BeginWihthMaxError(right,MaxError,beginPos-1);
        return;
    }
    PostingList ret;
    uint32_t leftLength=left.StrLength;
    uint32_t rightLength=right.StrLength;
    ret.MaxError=left.MaxError+right.MaxError+1;
    ret.StrLength=leftLength+rightLength;
    ret.Candidates.resize(MaxError+1);
    for(size_t i=0; i<left.Candidates.size(); i++)
    {
        for(size_t j=0; j<right.Candidates.size(); j++)
        {
            if(i+j>MaxError) break;
            vector<Candidate>  mergeResult=left.Candidates[i]&right.Candidates[j];
            for(size_t k=i+j; k<ret.Candidates.size(); k++)
            {

                if(k-i-j<mergeResult.size())
                {

                    ret.Candidates[k]=ret.Candidates[k]|mergeResult[k-i-j];
                }
            }
        }
    }


    for(size_t i=0; i<left.Candidates.size(); i++)
    {
        if(MaxError>= i+right.Candidates.size())
        {
            ret.Candidates[right.Candidates.size()+i]=(ret.Candidates[right.Candidates.size()+i]|(left.Candidates[i]>>rightLength));
        }
    }
    for(size_t j=0; j<right.Candidates.size(); j++)
    {
        if(MaxError>= j+left.MaxError+1)
        {
            if(match)
            {
                int StartPos=leftLength-1;
                vector<Candidate>  mergeResult;
                BeginWith(right.Candidates[j],StartPos+beginPos,mergeResult,left.MaxError);
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
    left=ret;
#ifdef TimeDebug
    boost::posix_time::ptime time_then = boost::posix_time::microsec_clock::local_time();
    runtime =runtime+( time_then - time_now);//
#endif
    //return ret;

}

PostingList operator | ( PostingList& left , PostingList& right)
{

    mergeWihthMaxError(left,right,min(left.MaxError+right.MaxError+1,left.Size()+right.Size()>MaxListSize?0:uint32_t(MaxErrorDepth)));
    return left;
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

class QEPair
{

public:
    UString query;
    int MaxError;
    QEPair(UString &query_ , int &MaxError_):query(query_),MaxError(MaxError_)
    {};
};
bool operator < (const QEPair& left ,const QEPair& right)
{
    return left.query==right.query?left.MaxError==right.MaxError:left.query<right.query;
}
bool operator == (const QEPair& left ,const QEPair& right)
{
    return left.query==right.query&&left.MaxError==right.MaxError;
}
class  MatchIndex
{


    std::map<UString,PostingList > prefixVec;
    vector<UString > Context;
    bool build;
    boost::posix_time::time_duration spilt ;
    boost::posix_time::time_duration candidate;
    boost::posix_time::time_duration filter ;
    boost::posix_time::time_duration merge ;
    boost::posix_time::time_duration tg ;
public:
    MatchIndex();
    ~MatchIndex();
    void Add(const UString& text);
    void Add(vector<UString>& textVec);
    void BuildIndex();
    bool Hasbuild();
    void Match(const UString&  query,const int& MaxError, vector<UString>& ret );
    void NaiveMatch(const UString&  query,const int& MaxError,  vector<UString>& ret);
    void Save(ostream &ofs);
    void Load(istream &ifs);
    void Show();
    void Clear();
    friend istream& operator>> ( istream &f, MatchIndex &mi );
    friend ostream& operator<< ( ostream &f, const MatchIndex &mi );
private:
    void Spilit(const UString  &query,const int &MaxError,    std::pair<int,std::vector<UString> >& ret);
    int SizeOfPosting(const UString &query);
    int  EditDistance(const UString& source,const UString& target);
    UString GetText(const int& num);
    int AddText(const UString& text);
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
