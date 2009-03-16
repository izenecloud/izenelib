/**
 * @file sdb_btree_header.h
 * @brief The header file of CbFileHeader.
 * This file defines class CbFileHeader.
 * 
 */

#ifndef SDB_BTREE_HEADER_H_
#define SDB_BTREE_HEADER_H_

#include <iostream>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 *  brief cc-b*-tree file header
 * 
 * 
    @FileHeader
 * 
 *     |----------------|
 *     |   magic        |  
 *     |----------------| 
 * 	   |   maxKeys      | /then the order of the btree is 2*minDegree-1.
 *     |----------------|
 *     |   pageSize     |
 *	   |----------------| 
 *     |   cacheSize    |
 *     |----------------|
 *     |    .....       |
 *     |----------------|
 *     |   numItem      |   
 *     |----------------|      
 *     |   rootPos      |  
 *     |----------------|
 *     |   nPages       |
 *     |----------------| 
 */
struct CbFileHeader {
		int magic;  //set it as 0x061561, check consistence.			
		size_t maxKeys; //a node at mostly hold maxKeys items. 
		size_t pageSize;	
		size_t cacheSize;	
		size_t numItems;		
		long rootPos;
		
		//totally allocated pages.
		//fileSize = nPages * pageSize
		static size_t nPages; 
		
		CbFileHeader()
		{
			magic = 0x061561;
			maxKeys = 32;
			pageSize = 1024;	
			cacheSize = 100000;			
			numItems = 0;
			rootPos = sizeof(CbFileHeader)+sizeof(size_t);
			
			CbFileHeader::nPages = 0;				
		}

		void display(std::ostream& os = std::cout) {
			os<<"magic: "<<magic<<endl;	
			os<<"maxKeys: "<<maxKeys<<endl;
			os<<"pageSize: "<<pageSize<<endl;			
			os<<"cacheSize: "<<cacheSize<<endl;
			os<<"numItem: "<<numItems<<endl;				
			os<<"rootPos: "<<rootPos<<endl;
			
			os<<"nPages: "<<nPages<<endl;	
		}

		bool toFile(FILE* f)
		{
			if ( 0 != fseek(f, 0, SEEK_SET) )
				return false;
			fwrite(this, sizeof(CbFileHeader), 1, f);		
			fwrite(&nPages, sizeof(size_t), 1, f);	
			return true;
		}

		bool fromFile(FILE* f)
		{
			if ( 0 != fseek(f, 0, SEEK_SET) )
				return false;
			fread(this, sizeof(CbFileHeader), 1, f);
			fread(&nPages, sizeof(size_t), 1, f);				
			return true;
		}
	};

size_t CbFileHeader::nPages=0;

NS_IZENELIB_AM_END

#endif


