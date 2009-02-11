#ifndef SDB_HASH_HEADER_H_
#define SDB_HASH_HEADER_H_

#include <iostream>

using namespace std;

NS_IZENELIB_AM_BEGIN


struct ShFileHeader {
		int magic; //set it as 0x061561, check consistence.			
		size_t blockSize;
		size_t cacheSize;	
		size_t numItems;
		size_t nBlock;
		
		ShFileHeader()
		{
			magic = 0x061561;
			blockSize = 1024*128 - sizeof(long)-sizeof(int);		
			cacheSize = 100000;
			numItems = 0;
			nBlock = 0;		
			
		}

		void display(std::ostream& os = std::cout) {
			os<<"magic: "<<magic<<endl;	
			os<<"blockSize: "<<blockSize<<endl;			
			os<<"numItem: "<<numItems<<endl;
			os<<"nBlock: "<<nBlock<<endl;
			
		}

		bool toFile(FILE* f)
		{
			if ( 0 != fseek(f, 0, SEEK_SET) )
				return false;		
				
			fwrite(this, sizeof(ShFileHeader), 1, f);		
			return true;

		}

		bool fromFile(FILE* f)
		{
			if ( 0 != fseek(f, 0, SEEK_SET) )
				return false;
			fread(this, sizeof(ShFileHeader), 1, f);
			return true;
		}
	};

NS_IZENELIB_AM_END

#endif /*SDB_HASH_HEADER_H_*/
