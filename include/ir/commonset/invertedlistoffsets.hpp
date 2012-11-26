#ifndef _IZENELIB_IR_COMMONSET_INVERTEDLIST_OFFSETS_HPP_
#define _IZENELIB_IR_COMMONSET_INVERTEDLIST_OFFSETS_HPP_

#include <fstream>
#include <map>

#include "query.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class InvertedListOffsets
{
public:

    InvertedListOffsets( unsigned int ntokens, unsigned int nqueries, unsigned long ndocids_max ) :
        ntokens_(ntokens),
        nqueries_(nqueries),
        ninvertedlists_(ntokens+nqueries),
        ndata_( 2 * ninvertedlists_ ),
        nunique_docids_(ndocids_max),
        offsets_(NULL),
        offset_ptrs_(NULL)
    {
        offsets_ = new unsigned int[ndata_];
    }

    InvertedListOffsets( const std::string& filename ) :
        ntokens_(0),
        nqueries_(0),
        ninvertedlists_(0),
        ndata_(0),
        nunique_docids_(0),
        offsets_(NULL),
        offset_ptrs_(NULL)
    {
        readOffsets( filename );
    }

    InvertedListOffsets( const std::string& filename, DocID* inverted_list_data ) :
        ntokens_(0),
        nqueries_(0),
        ninvertedlists_(0),
        ndata_(0),
        nunique_docids_(0),
        offsets_(NULL),
        offset_ptrs_(NULL)
    {
        readOffsets( filename );
        //computeOffsetPointers( inverted_list_data );
    }

    virtual ~InvertedListOffsets()
    {
        delete[] offsets_;
        delete[] offset_ptrs_;
    }

    bool readOffsets( const std::string& filename )
    {
        std::fstream file( filename.c_str(), std::ios::binary | std::ios::in );
        if( !file.is_open() ) return false;
        file.read( reinterpret_cast<char*>(&ntokens_), sizeof(unsigned int) );
        file.read( reinterpret_cast<char*>(&nqueries_), sizeof(unsigned int) );
        ninvertedlists_ = ntokens_ + nqueries_;
        file.read( reinterpret_cast<char*>(&nunique_docids_), sizeof(unsigned long) );
        ndata_ = 2 * ninvertedlists_ ;
        offsets_ = new unsigned int[ndata_];
        file.read( reinterpret_cast<char*>(&offsets_), ndata_ * sizeof(unsigned int) );
        file.close();
        return true;
    }

    bool writeOffsets( const std::string& filename )
    {
        std::fstream file( filename.c_str(), std::ios::binary | std::ios::out );
        if( !file.is_open() ) return false;
        file.write( reinterpret_cast<char*>(&ntokens_), sizeof(unsigned int) );
        file.write( reinterpret_cast<char*>(&nqueries_), sizeof(unsigned int) );
        file.write( reinterpret_cast<char*>(&nunique_docids_), sizeof(unsigned long) );
        file.write( reinterpret_cast<char*>(&offsets_), 2 * ntokens_ * sizeof(unsigned int) );
        file.close();
        return true;
    }

    bool addInvertedList( const TokenID& tokenid, DocID* docids, unsigned int ndocids )
    {
        unsigned int invertedlistid = addInvertedListID( tokenid );

        return addInvertedList( invertedlistid, docids, ndocids );
    }

    bool addInvertedList( const Query<TokenID>& query, DocID* docids, unsigned int ndocids )
    {
        unsigned int invertedlistid = addInvertedListID( query );

        return addInvertedList( invertedlistid, docids, ndocids );
    }

    bool addInvertedList( unsigned int invertedlistid, DocID* docids, unsigned int ndocids )
    {
        if( offsets_ == NULL ) return false;

        for( unsigned int idocid = 0 ; idocid < ndocids ; ++idocid )
        {
            if( docids[idocid] < nunique_docids_ )
            {
                offsets_[ invertedlistid ] = 0;
            }
        }

        offsets_[invertedlistid] = ndocids;

        return true;
    }

    bool convertOffsetsToPointers( DocID* inverted_list_data_ )
    {
        delete[] offset_ptrs_;
        offset_ptrs_ = new DocID*[ ndata_ ];
        DocID* ptr = *offset_ptrs_;
        unsigned int* ndocids_ptr = offsets_;

        for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken, ++ptr, ++ndocids_ptr )
        {
            *ptr = *ndocids_ptr;
        }

        delete[] offsets_;
        return true;
    }

    unsigned int getNDocIDs( unsigned int invertedlistid ) const
    {
        if( invertedlistid >= ninvertedlists_ ) return 0;
        unsigned int ipos = 2 * invertedlistid ;
        if( offset_ptrs_ == NULL ) return offsets_[ ipos ];
        return (unsigned int) ( offset_ptrs_[ ipos + 2 ] - offset_ptrs_[ ipos ] );
    }

    bool getDocIDs( unsigned int invertedlistid, DocID*& docidptr, DocID*& docidptr_end ) const
    {
        if( invertedlistid >= ninvertedlists_ || offset_ptrs_ == NULL ) return false;

        for( unsigned int i = 0, ipos = 2 * invertedlistid ; i < 1 ; ++i, ++ipos )
        {
            docidptr[i] = *offset_ptrs_[ ipos ];
            docidptr_end[i] = *offset_ptrs_[ ipos + 1 ];
        }

        return true;
    }

    bool getInvertedListID( const TokenID& tokenid, unsigned int& id ) const
    {
        typename std::map<TokenID,unsigned int>::const_iterator id_it = token_id_map_.find( tokenid );
        if( id_it == token_id_map_.end() ) return false;
        id = id_it->second;
        return true;
    }

    bool getInvertedListID( const Query<TokenID>& query, unsigned int& id ) const
    {
        typename std::map<Query<TokenID>,unsigned int>::const_iterator id_it;// = query_id_map_.find( query );
        if( id_it == query_id_map_.end() ) return false;
        id = id_it->second;
        return true;
    }

    unsigned int getNTokens() const
    {
        return ntokens_;
    }

    unsigned int getNQueries() const
    {
        return nqueries_;
    }

    unsigned int getNData() const
    {
        return ndata_;
    }

    unsigned int getNUniqueDocIDs() const
    {
        return nunique_docids_;
    }
private:

    unsigned int ntokens_;

    unsigned int nqueries_;

    unsigned int ninvertedlists_;

    unsigned int ndata_;

    unsigned int nunique_docids_;

    unsigned int* offsets_;

    DocID** offset_ptrs_;

    std::map<TokenID,unsigned int> token_id_map_;

    std::map<Query<TokenID>,unsigned int> query_id_map_;

    unsigned int addInvertedListID( const TokenID& tokenid )
    {
        std::map<TokenID,unsigned int> id = token_id_map_.find( tokenid );
        if( id != token_id_map_.end() ) return id->second;
        unsigned int invertedlistid = token_id_map_.size();
        token_id_map_.insert( std::pair<TokenID,unsigned int>(tokenid,invertedlistid) );
        return invertedlistid;
    }

    unsigned int addInvertedListID( const Query<TokenID>& query )
    {
        std::map<Query<TokenID>,unsigned int> id = query_id_map_.find( query );
        if( id != query_id_map_.end() ) return id->second;
        unsigned int invertedlistid = query_id_map_.size();
        query_id_map_.insert( std::pair<Query<TokenID>,unsigned int>(query,invertedlistid) );
        return invertedlistid;
    }
};

}
}
}

#endif
