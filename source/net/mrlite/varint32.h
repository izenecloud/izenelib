#ifndef BASE_VARINT32_H_
#define BASE_VARINT32_H_

bool ReadVarint32(FILE* input, uint32* value);
bool WriteVarint32(FILE* output, uint32 value);

#endif  // BASE_VARINT32_H_
