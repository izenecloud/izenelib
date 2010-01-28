#ifndef SDB_STORAGE_HEADER_H_
#define SDB_STORAGE_HEADER_H_

#include <iostream>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 * \brief FileHeader of sdb_hash
 *  
 */
struct SsHeader {
	int magic; //set it as 0x061561, check consistence.			
	size_t pageSize;
	size_t cacheSize;
	size_t nPage;
	size_t dPage; //deleted pages
	
	SsHeader()
	{
		magic = 0x061561;
		pageSize = 8;		
		cacheSize = 100000;		
		nPage = 0;
		dPage = 0;
	}

	void display(std::ostream& os = std::cout) {
		os<<"magic: "<<magic<<endl;
		os<<"pageSize: "<<pageSize<<endl;		
		os<<"cacheSize: "<<cacheSize<<endl;
		os<<"nPage: "<<nPage<<endl;	
		
		os<<endl;
		os<<"Data file size: "<<nPage*pageSize + sizeof(SsHeader)<<" bytes"<<endl;
		if(nPage != 0) {
			os<<"density: "<<double(dPage)/double(nPage)<<endl;	
			
		}
	}

	bool toFile(FILE* f)
	{
		if ( 0 != fseek(f, 0, SEEK_SET) )
		return false;

		fwrite(this, sizeof(SsHeader), 1, f);
		return true;

	}

	bool fromFile(FILE* f)
	{
		if ( 0 != fseek(f, 0, SEEK_SET) )
		return false;
		fread(this, sizeof(SsHeader), 1, f);
		return true;
	}
};

NS_IZENELIB_AM_END

#endif /*SDB_HASH_HEADER_H_*/
