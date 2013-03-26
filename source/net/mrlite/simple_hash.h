#ifndef HASH_SIMPLE_HASH_H_
#define HASH_SIMPLE_HASH_H_

#include <string>

typedef unsigned int (*HashFunction)(const std::string&);

unsigned int RSHash(const std::string& str);
unsigned int JSHash(const std::string& str);
unsigned int PJWHash(const std::string& str);
unsigned int ELFHash(const std::string& str);
unsigned int BKDRHash(const std::string& str);
unsigned int SDBMHash(const std::string& str);
unsigned int DJBHash(const std::string& str);
unsigned int DEKHash(const std::string& str);
unsigned int BPHash(const std::string& str);
unsigned int FNVHash(const std::string& str);
unsigned int APHash(const std::string& str);

#endif  // HASH_SIMPLE_HASH_H_
