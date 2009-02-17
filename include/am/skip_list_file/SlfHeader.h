#ifndef SLFHEADER_H_
#define SLFHEADER_H_


#include "slf_types.h"


NS_IZENELIB_AM_BEGIN


struct SlfHeader {
	int magic; //set it as 0x061561, check consistence.
	size_t degree;
	size_t pageSize;
	size_t cacheSize;
	size_t numItem;
	size_t nNode;
	int height;

	size_t numList;
	long headerPos;
	//long listPos[MAXLEVEL]; //streamoff of the root of btree.	

	SlfHeader() {
		magic = 0x061561;
		degree = 2;
		pageSize = 1024;	
		cacheSize = 1000000;
		numItem = 0;
		height = 0;
		nNode = 0;
		headerPos = sizeof( SlfHeader );
	}

	void display() {
		cout<<"displaying fileHeader ...\n";
		cout<<"magic: "<<magic<<endl;
		cout<<"degree: "<<degree<<endl;
		cout<<"numItem: "<<numItem<<endl;
		cout<<"pageSize: "<<pageSize<<endl;	
		cout<<"cacheSize: "<<cacheSize<<endl;
		cout<<"height: "<<height<<endl;
		cout<<"nNode: "<<nNode<<endl;
		cout<<"headerPos: "<<headerPos<<endl;

		/*for(size_t i=0; i<numList; i++) {
		 cout<<"list "<<i<<" offset : "<<listPos[i]<<endl;
		 }*/
	}

	bool toFile(FILE* f) {
		if ( 0 != fseek(f, 0, SEEK_SET) )
		return false;
		fwrite(this, sizeof(SlfHeader), 1, f);
		
		/*fwrite(&magic, sizeof(int), 1, f);
		fwrite(&degre, sizeof(size_t), 1, f);
		fwrite(&pageSize, sizeof(size_t), 1, f);
		fwrite(&overFlowSize, sizeof(size_t), 1, f);
		fwrite(&cacheSize, sizeof(size_t), 1, f);
		fwrite(&numItem, sizeof(size_t), 1, f);
		fwrite(&numList, sizeof(size_t), 1, f);
		fwrite(&headerPos, sizeof(long), 1, f);

		for(size_t i=0; i<MAX_LEVEL; i++) {
			fwrite( &listPos[i], sizeof(long), 1, f);
		}*/
		return true;

	}

	bool fromFile(FILE* f) {
		if ( 0 != fseek(f, 0, SEEK_SET) )
			return false;
		fread(this, sizeof(SlfHeader), 1, f);
		
		//display();

		/*fread(&magic, sizeof(int), 1, f);
		fread(&degree, sizeof(size_t), 1, f);
		fread(&pageSize, sizeof(size_t), 1, f);
		fread(&overFlowSize, sizeof(size_t), 1, f);
		fread(&cacheSize, sizeof(size_t), 1, f);
		fread(&numItem, sizeof(size_t), 1, f);
		fread(&numList, sizeof(size_t), 1, f);
		fread(&headerPos, sizeof(long), 1, f);

		for(size_t i=0; i<MAX_LEVEL; i++) {
			fwrite( &listPos[i], sizeof(long), 1, f);
		}*/
		return true;
	}	

};

NS_IZENELIB_AM_END

#endif /*SLF_HEAD_H_*/
