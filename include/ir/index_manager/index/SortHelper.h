/*
* Sort helper borrowed from sphinx search
*/

#ifndef SORTHELPER_H
#define SORTHELPER_H

#include <ir/index_manager/index/LAInput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template < typename T > class AutoArray
{
protected:
    T * pData_;

    int iSize_;
public:
    AutoArray ()
        :pData_(0)
        ,iSize_(0)
    {
    }
    ~AutoArray ()
    {
        reset();
    }

    void assign(int iCount)
    {
        pData_ = ( iCount>0 ) ? new T [ iCount ] : NULL;
        iSize_ = iCount;
    }

    void reset()
    {
        if (pData_) delete[] pData_;
        pData_ = NULL;
        iSize_ = 0;
    }

    const AutoArray & operator = ( const AutoArray & ) { assert(0); return *this; }

    operator T * ()
    {
        return pData_;
    }

    int size()
    {
        return iSize_;
    }
};

inline int Log2 ( uint64_t iValue )
{
    int iBits = 0;
    while ( iValue )
    {
        iValue >>= 1;
        iBits++;
    }
    return iBits;
}

/// swap
template < typename T > inline void Swap ( T & v1, T & v2 )
{
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

template < typename T >
struct Accessor_T
{
    typedef T MEDIAN_TYPE;

    MEDIAN_TYPE & Key ( T * a ) const
    {
        return *a;
    }

    void CopyKey ( MEDIAN_TYPE * pMed, T * pVal ) const
    {
        *pMed = Key(pVal);
    }

    void Swap ( T * a, T * b ) const
    {
        izenelib::ir::indexmanager::Swap ( *a, *b );
    }

    T * Add ( T * p, int i ) const
    {
        return p+i;
    }

    int Sub ( T * b, T * a ) const
    {
        return (int)(b-a);
    }
};


/// heap sort helper
template < typename T, typename U, typename V >
void siftDown ( T * pData, int iStart, int iEnd, U COMP, V ACC )
{
    for ( ;; )
    {
        int iChild = iStart*2+1;
        if ( iChild>iEnd )
            break;

        int iChild1 = iChild+1;
        if ( iChild1<=iEnd && COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iChild1 ) ) ) )
            iChild = iChild1;

        if ( COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iStart ) ) ) )
            return;
        ACC.Swap ( ACC.Add ( pData, iChild ), ACC.Add ( pData, iStart ) );
        iStart = iChild;
    }
}


/// heap sort
template < typename T, typename U, typename V >
void heapSort ( T * pData, int iCount, U COMP, V ACC )
{
    if ( !pData || iCount<=1 )
        return;

    // build a max-heap, so that the largest element is root
    for ( int iStart=( iCount-2 )>>1; iStart>=0; iStart-- )
        siftDown ( pData, iStart, iCount-1, COMP, ACC );

    // now keep popping root into the end of array
    for ( int iEnd=iCount-1; iEnd>0; )
    {
        ACC.Swap ( pData, ACC.Add ( pData, iEnd ) );
        siftDown ( pData, 0, --iEnd, COMP, ACC );
    }
}


/// generic sort
template < typename T, typename U, typename V >
void bufferSort ( T * pData, int iCount, U COMP, V ACC )
{
    typedef T * P;
    P st0[32], st1[32], a, b, i, j;
    typename V::MEDIAN_TYPE x;
    int k;

    const int SMALL_THRESH = 32;
    int iDepthLimit = Log2 ( iCount );
    iDepthLimit = ( ( iDepthLimit<<2 ) + iDepthLimit ) >> 1; // x2.5

    k = 1;
    st0[0] = pData;
    st1[0] = ACC.Add ( pData, iCount-1 );
    while ( k )
    {
        k--;
        i = a = st0[k];
        j = b = st1[k];

        // if quicksort fails on this data; switch to heapsort
        if ( !k )
        {
            if ( !--iDepthLimit )
            {
                heapSort ( a, ACC.Sub ( b, a )+1, COMP, ACC );
                return;
            }
        }

        // for tiny arrays, switch to insertion sort
        int iLen = ACC.Sub ( b, a );
        if ( iLen<=SMALL_THRESH )
        {
            for ( i=ACC.Add ( a, 1 ); i<=b; i=ACC.Add ( i, 1 ) )
            {
                for ( j=i; j>a; )
                {
                    P j1 = ACC.Add ( j, -1 );
                    if ( COMP.IsLess ( ACC.Key(j1), ACC.Key(j) ) )
                        break;
                    ACC.Swap ( j, j1 );
                    j = j1;
                }
            }
            continue;
        }

        ACC.CopyKey ( &x, ACC.Add ( a, iLen/2 ) );
        while ( a<b )
        {
            while ( i<=j )
            {
                while ( COMP.IsLess ( ACC.Key(i), x ) )
                    i = ACC.Add ( i, 1 );
                while ( COMP.IsLess ( x, ACC.Key(j) ) )
                    j = ACC.Add ( j, -1 );
                if ( i<=j )
                {
                    ACC.Swap ( i, j );
                    i = ACC.Add ( i, 1 );
                    j = ACC.Add ( j, -1 );
                }
            }

            if ( ACC.Sub ( j, a )>=ACC.Sub ( b, i ) )
            {
                if ( a<j )
                {
                    st0[k] = a;
                    st1[k] = j;
                    k++;
                }
                a = i;
            }
            else
            {
                if ( i<b )
                {
                    st0[k] = i;
                    st1[k] = b;
                    k++;
                }
                b = j;
            }
        }
    }
}

#define CMPTERMID_LESS(a,b) \
	( a.termid_<b.termid_ || \
	( a.termid_==b.termid_ && a.docId_<b.docId_ ) || \
	( a.termid_==b.termid_ && a.docId_==b.docId_ && a.wordOffset_<b.wordOffset_ ) )


struct CmpTermId_fn
{
    inline bool IsLess ( const TermId& a, const TermId & b ) const
    {
        return CMPTERMID_LESS ( a, b );
    }
};


template < typename T, typename U >
void bufferSort ( T * pData, int iCount, U COMP )
{
    bufferSort ( pData, iCount, COMP, Accessor_T<T>() );
}

}

NS_IZENELIB_IR_END

#endif
