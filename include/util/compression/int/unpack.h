#ifndef UNPACK_H_
#define UNPACK_H_

namespace izenelib{namespace util{namespace compression{

void unpack0(unsigned int* p, unsigned int* w, int BS);
void unpack1(unsigned int* p, unsigned int* w, int BS);
void unpack2(unsigned int* p, unsigned int* w, int BS);
void unpack3(unsigned int* p, unsigned int* w, int BS);
void unpack4(unsigned int* p, unsigned int* w, int BS);
void unpack5(unsigned int* p, unsigned int* w, int BS);
void unpack6(unsigned int* p, unsigned int* w, int BS);
void unpack7(unsigned int* p, unsigned int* w, int BS);
void unpack8(unsigned int* p, unsigned int* w, int BS);
void unpack9(unsigned int* p, unsigned int* w, int BS);
void unpack10(unsigned int* p, unsigned int* w, int BS);
void unpack11(unsigned int* p, unsigned int* w, int BS);
void unpack12(unsigned int* p, unsigned int* w, int BS);
void unpack13(unsigned int* p, unsigned int* w, int BS);
void unpack16(unsigned int* p, unsigned int* w, int BS);
void unpack20(unsigned int* p, unsigned int* w, int BS);
void unpack32(unsigned int* p, unsigned int* w, int BS);

typedef void (*pf)(unsigned int* p, unsigned int* w, int BS);

}}}
#endif /* UNPACK_H_ */
