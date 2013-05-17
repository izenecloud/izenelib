#ifndef _OSACA_H_
#define _OSACA_H_

#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <iterator>
#include <iostream>
#include "custom_int.hpp"
#include "custom_uint.hpp"

namespace izenelib{ namespace am{ namespace succinct{ 
namespace osaca{
namespace osaca_private {
    // get s[i] at level>=0
    #define chr(i) ((level==0)?(uint64_t)(((char_type *)s)[i]):(uint64_t)(MASK0&((level_type *)s)[i]))

    // get s[i] at level>0
    #define chr1(i) (MASK0&((savalue_type *)s)[i])

    // get the type of an integer character in e
    #define tget(e) (EMPTY&e ? 1 : 0)

    template<typename string_type, typename index_type>
    void
    getBuckets(string_type s, index_type *bkt, index_type n, index_type k, bool end)
    {
        //cout << "getBuckets "<<endl;
        index_type i, sum=0;
        //clear all buckets
        for(i=0; i<k; i++)
            bkt[i]=0;
        //compute the size of each bucket
        for(i=0; i<n; i++)
            bkt[s[i]]++;
        for(i=0; i<k; i++)
        {
            sum += bkt[i];
            bkt[i] = end ? sum-1 : sum-bkt[i];
        }
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    putSuffix0(sarray_type SA, string_type s, index_type *bkt, index_type n, index_type k, index_type n1, level_type level)
    {
        //cout << "putSuffix0 "<<endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        typedef typeof(level) l_type;

        level_type i;
        level_type j;
        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);

        //find the end of each bucket
        getBuckets(s, bkt, n, k, true);

        //put the suffixes into their buckets
        for(i=n1-1; i>0; i--)
        {
            j=SA[i];SA[i] = EMPTY;
            SA[bkt[s[j]]--]=j;
        }
        SA[0]=n-1;
    }

    template<typename string_type, typename sarray_type, typename index_type>
    void
    induceSAl0(sarray_type SA, string_type s, index_type *bkt, index_type n,
            index_type k, bool suffix)
    {
        //cout << "induceSAl0 "<<" n: " << n << " k: " << k << endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;

        savalue_type i;
        savalue_type j;
        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);

        //find the head of each bucket
        getBuckets(s, bkt, n, k, false);

        //skip the virtual sentinel
        bkt[0]++;

        for(i=0; i<n; i++)
        {
            if(SA[i]!=EMPTY && SA[i]!=(savalue_type)0)
            {
                j=SA[i]-1;
                if(s[j]>=s[j+1])
                {
                    SA[bkt[s[j]]] = j;
                    bkt[s[j]]++;
                    if(!suffix && i>(savalue_type)0) SA[i] = EMPTY;
                }
            }
        }
    }

    template<typename string_type, typename sarray_type, typename index_type>
    void
    induceSAs0(sarray_type SA, string_type s, index_type *bkt, index_type n,
            index_type k, bool suffix)
    {
        //cout << "induceSAs0 "<< " n: " << n << " k: " << k << endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        savalue_type i;
        savalue_type j;

    	typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);

        //find the end of each bucket
        getBuckets(s, bkt, n, k, true);

        for(i=n-1; i>(savalue_type)0; i--)
        {
            if(SA[i]!=EMPTY && SA[i]!=(savalue_type)0)
            {
                j=SA[i]-1;
                if(s[j]<=s[j+1] && bkt[s[j]]<i)
                {
                    SA[bkt[s[j]]]=j;
                    bkt[s[j]]--;
                    if(!suffix) SA[i] = EMPTY;
                }
            }
        }
    }

    template<typename string_type, typename sarray_type, typename index_type>
    void
    putSubstr0(sarray_type SA, string_type s, index_type *bkt, index_type n,
            index_type k)
    {
        //cout << "putSubstr0 "<<endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        index_type i, cur_t, succ_t;

        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);

        //find the end of each bucket
        getBuckets(s, bkt, n, k, true);

        //set each item in SA as empty
        for(i=0; i<n; i++)SA[i]=EMPTY;

        succ_t=0; // s[n-2] must be L - type
        for(i=n-2; i>(index_type)0; i--)
        {
            cur_t = (s[i-1]<s[i] ||
                    (s[i-1]==s[i] && succ_t ==(index_type)1)
                    )?1:0;
            if(cur_t==(index_type)0 && succ_t ==(index_type)1) SA[bkt[s[i]]--]=i;
            succ_t = cur_t;
        }

        //set the single sentinel LMS-substring
        SA[0]=n-1;
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    putSuffix1(sarray_type SA, string_type s, index_type n1, level_type level)
    {
        //cout << "putSuffix1 "<<endl;
        typedef typeof(*SA) savalue_type;
        typedef typeof(level) op_type;
    	typedef savalue_type new_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        new_type i, j, pos, cur, pre = -1;
        for(i=n1-1; i>0; i--)
        {
            j=SA[i]; SA[i]=EMPTY;
            cur=chr1(j);
            if(cur!=pre)
            {
                pre=cur; pos=cur;
            }
            SA[pos--]=j;
        }
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    induceSAl1(sarray_type SA, string_type s, index_type n, level_type level, bool suffix)
    {
        //cout << "induceSAl1 "<<" n: " << n<<endl;
        typedef typeof(*SA) savalue_type;
        typedef typeof(level) op_type;
    	typedef savalue_type new_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        new_type h, i, j, step=1;

        for(i=0; (op_type)i<n; i+=step)
        {
            step=1; j=SA[i]-1;
            if(SA[i]<=0 || tget(s[j])) continue;
            // s[j] is L-type.
            new_type c=chr1(j), d=SA[c];
            if(d>=0)
            {
                // SA[c] is borrowed by the left
                //   neighbor bucket.
                // shift-left the items in the
                //   left neighbor bucket.
                savalue_type foo, bar;
                foo=SA[c];
                for(h=c-1; SA[h]>=0||SA[h]==(new_type)EMPTY; h--)
                {
                    bar=SA[h]; SA[h]=foo; foo=bar;
                }
                SA[h]=foo;
                if(h<i) step=0;
                d=EMPTY;
            }
            if(d==(new_type)EMPTY)
            { // SA[c] is empty.
                if((op_type)c<(n-1) && SA[c+1]==(new_type)EMPTY)
                {
                    SA[c]=-1; // init the counter.
                    SA[c+1]=j;
                }
                else
                    SA[c]=j; // a size-1 bucket.
            }
            else
            { // SA[c] is reused as a counter.
                new_type pos=c-d+1;
                if((op_type)pos>(n-1) || SA[pos]!=(new_type)EMPTY)
                {
                    // we are running into the right
                    //   neighbor bucket.
                    // shift-left one step the items
                    //   of bucket(SA, S, j).
                    for(h=0; h<-d; h++)
                        SA[c+h]=SA[c+h+1];
                    pos--;
                    if(c<i) step=0;
                }
                else
                    SA[c]--;

                SA[pos]=j;
            }
            if((!suffix || tget(s[j+1])) && i>0)
            {
                new_type i1=(step==0)?i-1:i;
                SA[i1]=EMPTY;
            }
        }
        // scan to shift-left the items in each bucket
        //   with its head being reused as a counter.
        for(i=1; (op_type)i<n; i++)
        {
            j=SA[i];
            if(j<0 && j!=(new_type)EMPTY)
            { // is SA[i] a counter?
                for(h=0; h<-j; h++)
                    SA[i+h]=SA[i+h+1];
                SA[i+h]=EMPTY;
            }

        }
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    induceSAs1(sarray_type SA, string_type s,index_type n, level_type level, bool suffix)
    {
        //cout << "induceSAs1 "<<endl;
        typedef typeof(*SA) savalue_type;
        typedef typeof(level) op_type;
    	typedef savalue_type new_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        new_type h, i, j, step=1;
        for(i=n-1; i>0; i-=step)
        {
            step=1; j=SA[i]-1;
            if(SA[i]<=0 || !tget(s[j])) continue;

            // s[j] is S-type

            new_type c=chr1(j), d=SA[c];
            if(d>=0)
            {
                // SA[c] is borrowed by the right
                //   neighbor bucket.
                // shift-right the items in the
                //   right neighbor bucket.
                new_type foo, bar;
                foo=SA[c];
                for(h=c+1; SA[h]>=0||SA[h]==(new_type)EMPTY; h++)
                {
                    bar=SA[h]; SA[h]=foo; foo=bar;
                }
                SA[h]=foo;
                if(h>i)
                    step=0;

                d=EMPTY;
            }

            if(d==(new_type)EMPTY)
            {
                // SA[c] is empty.
                if(SA[c-1]==(new_type)EMPTY)
                {
                    SA[c]=-1; // init the counter.
                    SA[c-1]=j;
                }
                else
                    SA[c]=j; // a size-1 bucket.
            }
            else
            {
                // SA[c] is reused as a counter.
                new_type pos=c+d-1;
                if(SA[pos]!=(new_type)EMPTY)
                {
                    // we are running into the left
                    //   neighbor bucket.
                    // shift-right one step the items
                    //   of bucket(SA, S, j).
                    for(h=0; h<-d; h++)
                        SA[c-h]=SA[c-h-1];
                    pos++;
                    if(c>i) step=0;
                }
                else
                    SA[c]--;

                SA[pos]=j;
            }

            if(!suffix)
            {
                new_type i1=(step==0)?i+1:i;
                SA[i1]=EMPTY;
            }
        }

        // scan to shift-right the items in each bucket
        //   with its head being reused as a counter.
        if(!suffix)
            for(i=n-1; i>0; i--)
            {
                j=SA[i];
                if(j<0 && j!=(new_type)EMPTY)
                {
                    // is SA[i] a counter?
                    for(h=0; h<-j; h++)
                        SA[i-h]=SA[i-h-1];
                    SA[i-h]=EMPTY;
                }
            }
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    putSubstr1(sarray_type SA, string_type s, index_type n, level_type level)
    {
        //cout << "putSubstr1 "<<endl;
        typedef typeof(*SA) savalue_type;
        typedef typeof(level) op_type;
        typedef savalue_type new_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        new_type h, i, j;

        for(i=0; i<n; i++) SA[i]=EMPTY;
        for(i=n-2; i>0; i--)
        {
            if(tget(s[i]) && !tget(s[i-1]))
            {
                // is s[i] a LMS-character?
                new_type c=chr1(i);
                if(SA[c]>=0)
                {
                    // SA[c] is borrowed by the right
                    //   neighbor bucket.
                    // shift-right the items in the
                    //   right neighbor bucket.
                    new_type foo, bar;
                    foo=SA[c];
                    for(h=c+1; SA[h]>=0; h++)
                    {
                        bar=SA[h]; SA[h]=foo; foo=bar;
                    }
                    SA[h]=foo;
                    SA[c]=EMPTY;
                }
                new_type d=SA[c];
                if(d==(new_type)EMPTY)
                {
                    // SA[c] is empty.
                    if(SA[c-1]==(new_type)EMPTY)
                    {
                        SA[c]=-1; // init the counter.
                        SA[c-1]=i;
                    }
                    else
                        SA[c]=i; // a size-1 bucket.
                }
                else
                {
                    // SA[c] is reused as a counter
                    new_type pos=c+d-1;
                    if(SA[pos]!=(new_type)EMPTY)
                    {
                        // we are running into the left
                        //   neighbor bucket.
                        // shift-right one step the items
                        //   of bucket(SA, S, i).
                        for(h=0; h<-d; h++)
                            SA[c-h]=SA[c-h-1];
                        pos++;
                    }
                    else
                        SA[c]--;
                    SA[pos]=i;
                }
            }
        }
        // scan to shift-right the items in each bucket
        //   with its head being reused as a counter.
        for(i=n-1; i>0; i--)
        {
            j=SA[i];
            if(j<0 && j!=(new_type)EMPTY)
            {
                // is SA[i] a counter?
                for(h=0; h<-j; h++)
                    SA[i-h]=SA[i-h-1];
                SA[i-h]=EMPTY;
            }
        }

        // put the single sentinel LMS-substring.
        SA[0]=n-1;
    }


    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    index_type
    getLengthOfLMS(string_type s, sarray_type SA, index_type n, level_type level, index_type x)
    {
        //cout << "getLengthOfLMS "<<" n: "<<n<<" level: "<<level<<" x: "<<x<<endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        index_type dist, i=1;
        while(1)
        {
            if(x+i>n-1 || chr(x+i)<chr(x+i-1)) break;
            i++;
        }
        while(1)
        {
            if(x+i>n-1 || chr(x+i)>chr(x+i-1))break;
            if(x+i==n-1 || chr(x+i)<chr(x+i-1)) dist=i;
            i++;
        }
        return dist+1;
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    index_type
    nameSubstr(sarray_type SA, string_type s, sarray_type s1, index_type n,
            index_type m, index_type n1, level_type level)
    {
        //cout << "nameSubstr "<<" n: "<<n <<" m: "<<m<<" n1: "<<n1 <<" level: " << level << endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        index_type i, j, cur_t, succ_t;

        // init the name array buffer
        for(i=n1; i<n; i++) SA[i]=EMPTY;

        // scan to compute the interim s1
        index_type name = 0;
        index_type name_ctr=0;
        index_type pre_pos = 0;
        index_type pre_len=0;

        for(i=0; i<n1; i++)
        {
            bool diff=false;
            index_type len, pos=SA[i];
            len=getLengthOfLMS(s, SA, n, level, pos);
            if(len!=pre_len) diff=true;
            else
                for(savalue_type d=0; d<len; d++)
                    if(pos+d==n-1 || pre_pos+d==n-1 ||
                        chr(pos+d)!=chr(pre_pos+d))
                    {
                        diff=true; break;
                    }
            if(diff)
            {
                name=i; name_ctr++;
                SA[name]=1; // a new name.
                pre_pos=pos; pre_len=len;
            }
            else
            SA[name]++; // count this name.
            j=(pos%2==(index_type)0)?pos/2:(pos-1)/2;
            SA[n1+j]=name;
        }

        // compact the interim s1 sparsely stored
        //   in SA[n1, n-1] into SA[m-n1, m-1].
        for(i=n-1, j=m-1; i>=n1; i--)
            if(SA[i]!=EMPTY) SA[j--]=SA[i];
	
        // rename each S-type character of the
        //   interim s1 as the end of its bucket
        //   to produce the final s1.
        succ_t=1;
        for(i=n1-1; i>(index_type)0; i--) {
            level_type ch=MASK0&s1[i], ch1=MASK0&s1[i-1];
            cur_t=(ch1< ch || (ch1==ch && succ_t==(index_type)1))?1:0;
            if(cur_t==(index_type)1) {
            s1[i-1]+=SA[s1[i-1]]-1;
            s1[i-1]|=EMPTY; // set s1[i-1] as S-type.
            }
            succ_t=cur_t;
        }

        return name_ctr;
    }

    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    void
    getSAlms(sarray_type SA, string_type s, sarray_type s1, index_type n,
            index_type n1, level_type level)
    {
        //cout<<"getSAlms  "<<" n: "<<n<<" n1: "<<n1<<" level: "<<level<<endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        //typedef unsigned int op_type;
        typedef savalue_type op_type;
        op_type EMPTY = ((op_type)1) << (sizeof(op_type)*8-1);
        op_type MASK0 = ~EMPTY;

        index_type i, j, cur_t, succ_t;

        j=n1-1; s1[j--] = n-1;
        succ_t = 0;
        for(i=n-2; i>(index_type)0; i--)
        {
            cur_t=(chr(i-1) < chr(i) ||
                    (chr(i-1) == chr(i) && succ_t==(index_type)1))?1:0;
            if(cur_t==(index_type)0 && succ_t==(index_type)1) s1[j--]=i;
            succ_t=cur_t;
        }

        for(i=0; i<n1; i++) SA[i]=s1[SA[i]];

        // init SA[n1..n-1]
        for(i=n1; i<n; i++)SA[i] = EMPTY;
    }



    template<typename string_type, typename sarray_type, typename index_type, typename level_type>
    int
    osaca(string_type s, sarray_type SA, index_type n, index_type m,
            level_type level, index_type k =256)
    {
        //cout << "osaca "<<endl;
        typedef typeof(*s) char_type;
        typedef typeof(*SA) savalue_type;
        typedef level_type op_type;

        savalue_type EMPTY = ((savalue_type)1) << (sizeof(savalue_type)*8-1);
	savalue_type MASK0 = ~EMPTY;
        index_type i;
        index_type *bkt = NULL;

        // stage 1: reduce the problem by at least 1/2.
        if(level==0)
        {
            bkt = (index_type *)malloc(sizeof(op_type)*k);
            putSubstr0(SA, s, bkt, n, k);
            induceSAl0(SA, s, bkt, n, k, false);
            induceSAs0(SA, s, bkt, n, k, false);
        }
        else
        {

            putSubstr1((op_type *)SA, (op_type *)s, (op_type)n, (savalue_type)level);
            induceSAl1((op_type *)SA, (op_type *)s, n ,(savalue_type)level, false);
            induceSAs1((op_type *)SA, (op_type *)s, n, (savalue_type)level, false);

        }
        index_type n1=0;
        for(i=0; i<n; i++)
            if(((op_type *)SA)[i]>0) SA[n1++]=SA[i];
        sarray_type SA1 = SA;
        sarray_type s1 = SA + m -n1;
        index_type name_ctr;
        name_ctr = nameSubstr(SA, s, s1, n, m, n1, level);
        // stage 2: solve the reduced problem.
        if(name_ctr<n1)
            osaca((string_type)s1, SA1, n1, m-n1, level+1, (index_type)0);
        else // get the suffix array of s1 directly.
            for(i=0; i<n1; i++)
            {
//		cout<<"sa size: " << n <<" index: "<<(int)(s1[i]&MASK0)<<endl;
		SA1[(op_type)(s1[i]&MASK0)] = i;
            }

        // stage 3: induce SA(S) from SA(S1).
        getSAlms(SA, s, s1, n, n1, level);
        if(level==0)
        {
            putSuffix0(SA, s, bkt, n, k, n1, level);
            induceSAl0(SA, s, bkt, n, k, true);
            induceSAs0(SA, s, bkt, n, k, true);
            free(bkt);
        }
        else
        {

            putSuffix1((op_type *)SA, (op_type *)s, n1, (savalue_type)level);
            induceSAl1((op_type *)SA, (op_type *)s, n, (savalue_type)level, true);
            induceSAs1((op_type *)SA, (op_type *)s, n, (savalue_type)level, true);

        }
        return 0;
    }

} /*namespace osaca_private*/

template<typename string_type, typename sarray_type, typename index_type>
int
osacaxx(string_type s, sarray_type SA, index_type n, index_type k)
{
    //cout <<"osacaxx" << endl;

    //uint16_t 
    typedef typename std::iterator_traits<string_type>::value_type char_type;
    //int32_t, int40_t or int64_t
    typedef typename std::iterator_traits<sarray_type>::value_type savalue_type;

    if(sizeof(char_type) == 1)
    {
        uint8_t *S = (uint8_t *)&s[0];
        if(sizeof(savalue_type) == 4)
        {
            uint32_t *sa = (uint32_t *)&SA[0];
            return osaca_private::osaca(S, sa, (uint32_t)n, (uint32_t)n, (int32_t)0, (uint32_t)k);
        }
        else if(sizeof(savalue_type) == 5)
        {
            //TODO
            uint40_t *sa = (uint40_t *)&SA[0];
            return osaca_private::osaca(S, sa, (uint40_t)n, (uint40_t)n, (int40_t)0, (uint40_t)k);
        }
        else if(sizeof(savalue_type) == 8)
        {
            uint64_t *sa = (uint64_t *)&SA[0];
            return osaca_private::osaca(S, sa, (uint64_t)n, (uint64_t)n, (int64_t)0, (uint64_t)k);
        }
    }
    else if(sizeof(char_type) == 2)
    {
        uint16_t *S = (uint16_t *)&s[0];
        if(sizeof(savalue_type) == 4)
        {
            uint32_t *sa = (uint32_t *)&SA[0];
            return osaca_private::osaca(S, sa, (uint32_t)n, (uint32_t)n, (int32_t)0, (uint32_t)k);
        }
        else if(sizeof(savalue_type) == 5)
        {
            //TODO
//            uint40_t *sa = (uint40_t *)&SA[0];
//            return osaca_private::osaca(S, sa, (uint40_t)n, (uint40_t)n, (int40_t)0, (uint40_t)k);
        }
        else if(sizeof(savalue_type) == 8)
        {
            uint64_t *sa = (uint64_t *)&SA[0];
            return osaca_private::osaca(S, sa, (uint64_t)n, (uint64_t)n, (int64_t)0, (uint64_t)k);
        }

    }

    return -1;
}

} /*namespace osaca*/
} /*namespace succinct*/

}}

#endif //_OSACA_H_
