#include "BitPattern.h"

namespace izenelib {
namespace util{
namespace compression {

void BitPattern::fromFile(FILE *fp) {

	if ( !fp) {
		throw CompressorException("BitPattern:file not exists\n");
	}
	int magic;
	if ( 1 != fread(&magic, sizeof(int), 1, fp) ) {
		throw CompressorException("BitPattern:write file error. \n");
	}
	if(magic !=  0x13579acf){
		throw CompressorException("File formart error.. \n");
				
	}
	
	size_t sz;
	if (1 != fread(&sz, sizeof(size_t), 1, fp)) {
		throw CompressorException("BitPattern:read file error. \n");
	}
	for (size_t i=0; i<sz; i++) {
		char ch;
		if ( 1 != fread(&ch, sizeof(char), 1, fp) ) {
			return;
		}
		Pattern pt;
		if ( 1 != fread(&pt.nbits, sizeof(ub1), 1, fp) ) {
			return;
		}
		if (1 != fread(&pt.bits, sizeof(ub4), 1, fp)) {
			return;
		}
		bitPattern_[ch] = pt;
	}

}

void BitPattern::toFile(FILE *fp) {

	if ( !fp) {
		throw CompressorException("BitPattern:file not exists\n");
	}
	int magic = 0x13579acf;
	if ( 1 != fwrite(&magic, sizeof(int), 1, fp) ) {
		throw CompressorException("BitPattern:write file error. \n");
	}

	map<char, Pattern>::iterator it;

	size_t sz = bitPattern_.size();
	if ( 1 != fwrite(&sz, sizeof(size_t), 1, fp) ) {
		throw CompressorException("BitPattern:write file error. \n");
	}
	for (it=bitPattern_.begin(); it != bitPattern_.end(); it++) {

		char ch = it->first;
		if ( 1 != fwrite(&ch, sizeof(char), 1, fp) ) {
			return;
		}
		Pattern pt = it->second;
		if ( 1 != fwrite(&pt.nbits, sizeof(ub1), 1, fp) ) {
			return;
		}
		if (1 != fwrite(&pt.bits, sizeof(ub4), 1, fp) ) {
			return;
		}
	}

}

}}}

