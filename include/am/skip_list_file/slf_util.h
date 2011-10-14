#ifndef SLF_UTIL_H_
#define SLF_UTIL_H_

NS_IZENELIB_AM_BEGIN

inline unsigned int Log2(unsigned int num)
{
    unsigned int i;
    unsigned int limit;

    limit = 1;
    for (i = 0; limit < num; limit = limit << 1, i++)
        ;

    cout<<"log2\n";
    cout<<"input= "<<num<<endl;
    cout<<"outPut= "<<i<<endl;
    return (i);
}

const int MAX_BAND = 7;

long vBandNum[] = {0, 2, 4, 16, 64, 65536, 4294967296};

long vBandSum[] = {0, 2, 6, 22, 86, 65622, 4295032918};

int vBandHeight[] = {0, 1, 2, 4, 8, 16, 32 };

int vBandHeightAcc[] = {0, 1, 3, 7, 15, 31, 63 };


inline int totalBand(int num)
{
    int i =0;
    for (; vBandSum[i]<num; i++);
    return i;
}

inline int whichBand(int height)
{
    int i=0;
    for (; vBandHeightAcc[i]<height; i++);
    return i;

}

NS_IZENELIB_AM_END

#endif /*SLF_UTIL_H_*/
