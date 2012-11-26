#ifndef _IZENELIB_IR_COMMONSET_INDEX_DISK_CACHED_HPP_
#define _IZENELIB_IR_COMMONSET_INDEX_DISK_CACHED_HPP_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include "indexbase.hpp"
#include "invertedlistoffsets.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

// needs complete rewrite / debug

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class IndexDiskCached : virtual public IndexBase<DocID,TokenID>, private InvertedListOffsets<DocID,TokenID>
{
public:

    IndexDiskCached
    (
        std::string directory = "./",
        std::string filename_offsets = "docids_disk_cached_ndocids.bin"
    ) :
        IndexBase<DocID,TokenID>( directory ),
        InvertedListOffsets<DocID,TokenID>( directory + filename_offsets )
    {}

    virtual ~IndexDiskCached() {}

    bool getDocIDs( const TokenID& tokenid, DocID* buffer, unsigned int nbuffer_values, DocID*& docid_ptrs, DocID*& docid_ptrs_end )
    {
        unsigned int id = 0;

        bool ok = InvertedListOffsets<DocID,TokenID>::getInvertedListID( tokenid, id );

        return !ok ? false : getDocIDsFromInvertedListID( id, buffer, nbuffer_values, docid_ptrs, docid_ptrs_end );
    }

    bool getDocIDs( const Query<TokenID>& query, DocID* buffer, unsigned int nbuffer_values, DocID*& docid_ptrs, DocID*& docid_ptrs_end )
    {
        unsigned int id = 0;

        bool ok = InvertedListOffsets<DocID,TokenID>::getInvertedListID( query, id );

        return !ok ? false : getDocIDsFromInvertedListID( id, buffer, nbuffer_values, docid_ptrs, docid_ptrs_end );
    }

    bool getDocIDsFromInvertedListID( unsigned int invertedlistid, DocID* buffer, unsigned int nbuffer_values, DocID*& docid_ptrs, DocID*& docid_ptrs_end )
    {
        unsigned int ndocids = InvertedListOffsets<DocID,TokenID>::getNDocIDs( invertedlistid );

        if( nbuffer_values < ndocids ) return false;
        std::string filename = getInvertedListFilename( invertedlistid );
        std::fstream file( filename.c_str(), std::ios::binary | std::ios::in );
        if( !file.is_open() ) return false;
        file.read( reinterpret_cast<char*>(buffer), sizeof(DocID) * ndocids );
        file.close();
        //todo
        return false;//InvertedListOffsets<DocID,TokenID>::convertOffsetsToPointers( invertedlistid, buffer, docid_ptrs, docid_ptrs_end );
    }

    static bool mapDocIDs( const std::map<DocID,DocID>& docids_map, DocID* docids, unsigned int ndocids )
    {
        typename std::map<DocID,DocID>::iterator newdocid;

        for( DocID* docidptr = docids ; docidptr != docids + ndocids ; ++docidptr )
        {
            newdocid = docids_map.find( *docidptr );
            if( newdocid == docids_map.end() ) return false;
            *docidptr = newdocid->second;
        }
        return true;
    }

    bool writeInvertedListToDisk( const TokenID& tokenid, DocID* docids, unsigned int ndocids )
    {
        unsigned int invertedlistid = InvertedListOffsets<DocID,TokenID>::addInvertedList( tokenid, docids, ndocids );

        return writeInvertedListToDisk( invertedlistid, docids, ndocids );
    }

    bool writeInvertedListToDisk( const Query<TokenID>& query, DocID* docids, unsigned int ndocids )
    {
        unsigned int invertedlistid = InvertedListOffsets<DocID,TokenID>::addInvertedList( query, docids, ndocids );

        return writeInvertedListToDisk( invertedlistid, docids, ndocids );
    }

    std::string getInvertedListFilename( unsigned int invertedlistid )
    {
        std::stringstream ss;
        unsigned int millions = (unsigned int) ( invertedlistid / 1000000 );
        unsigned int thousands = (unsigned int) ( (invertedlistid%1000000) / 1000 );
        unsigned int filename = (unsigned int) ( invertedlistid%1000 );
        //ss << directory_ << "/" << std::setw(3) << millions << "/" << std::setw(3) << thousands << "/" << filename << ".bin";
        ss << IndexBase<DocID,TokenID>::getDirectory() << "/" << std::setw(3) << millions << "-" << std::setw(3) << thousands << "-" << std::setw(3) << filename << ".bin";
        return ss.str();
    }

    bool writeInvertedListToDisk( unsigned int invertedlistid, DocID* buffer, unsigned int nvalues_to_write )
    {
        std::string filename = getInvertedListFilename( invertedlistid );
        std::fstream file( filename.c_str(), std::ios::binary | std::ios::out );
        if( !file.is_open() ) return false;
        file.write( reinterpret_cast<char*>(buffer), sizeof(DocID) * nvalues_to_write );
        file.close();
        return true;
    }

};

}
}
}

#endif
