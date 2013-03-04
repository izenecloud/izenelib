/**
 * @file sdb_btree_header.h
 * @brief The header file of CbFileHeader.
 * This file defines class CbFileHeader.
 *
 */

#ifndef SDB_BTREE_HEADER_H_
#define SDB_BTREE_HEADER_H_

#include "../../types.h"

#include <stdio.h>
#include <iostream>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 *	brief cc-b*-tree file header
 *
 *
 @FileHeader
 *
 *	   |----------------|
 *	   |   magic		|
 *	   |----------------|
 *	   |   maxKeys		| /then the order of the btree is 2*minDegree-1.
 *	   |----------------|
 *	   |   pageSize		|
 *	   |----------------|
 *	   |   cacheSize	|
 *	   |----------------|
 *	   |	.....		|
 *	   |----------------|
 *	   |   numItem		|
 *	   |----------------|
 *	   |   rootPos		|
 *	   |----------------|
 *	   |   nPages		|
 *	   |----------------|
 *	   |   oPages		|
 *	   |----------------|
 */
struct CbFileHeader
{
    int magic; //set it as 0x061561, check consistence.
    size_t maxKeys; //a node at mostly hold maxKeys items.
    size_t pageSize;
    size_t cacheSize;
    size_t numItems;
    long rootPos;

    size_t nPages;
    size_t oPages;

    CbFileHeader()
    {
        ::memset(&magic, 0x00, sizeof(CbFileHeader));
        magic = 0x061561;
        maxKeys = 24;
        pageSize = 1024;
        cacheSize = 100*1024;
        initialize();
    }

    void display(std::ostream& os = std::cout)
    {
        os<<"magic: "<<magic<<endl;
        os<<"maxKeys: "<<maxKeys<<endl;
        os<<"pageSize: "<<pageSize<<endl;
        os<<"cacheSize: "<<cacheSize<<endl;
        os<<"numItem: "<<numItems<<endl;
        os<<"rootPos: "<<rootPos<<endl;

        os<<"node Pages: "<<nPages<<endl;
        os<<"overflow Pages: "<<oPages<<endl;
        os<<endl;
        os<<"file size: "<<pageSize*(nPages+oPages)+sizeof(CbFileHeader)<<"bytes"<<endl;
        if (nPages != 0)
        {
            os<<"average items number in a btree node: "<<double(numItems)/double(nPages)<<endl;
            os<<"average overflow page for a node: "<<double(oPages)/double(nPages)<<endl;
        }

    }

    bool toFile(FILE* f)
    {
        if ( 0 != fseek(f, 0, SEEK_SET) )
            return false;

        if ( 1 != fwrite(this, sizeof(CbFileHeader), 1, f) )
            return false;

        return true;
    }

    bool fromFile(FILE* f)
    {
        if ( 0 != fseek(f, 0, SEEK_SET) )
            return false;

        if ( 1 != fread(this, sizeof(CbFileHeader), 1, f) )
            return false;

        return true;
    }

    void initialize()
    {
        numItems = 0;
        rootPos = sizeof(CbFileHeader);
        nPages = 0;
        oPages = 0;
    }

};

const size_t SDB_FILE_HEAD_SIZE = 1024;
NS_IZENELIB_AM_END

#endif
