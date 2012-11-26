#ifndef _IZENELIB_IR_COMMONSET_MEMORY_CACHED_HPP_
#define _IZENELIB_IR_COMMONSET_MEMORY_CACHED_HPP_

#include <fstream>
#include <vector>

#include "indexbase.hpp"
#include "invertedlistoffsets.hpp"

namespace izenelib
{
namespace ir
{
namespace commonset
{

// needs complete debug / rewrite

template< typename DocID = unsigned int, typename TokenID = unsigned int >
class IndexMemoryCached : virtual public IndexBase<DocID,TokenID>, private InvertedListOffsets<DocID,TokenID>
{
public:

    IndexMemoryCached
    (
        std::string directory = "./",
        std::string filename_offsets = "docids_memory_cached_ndocids.bin",
        std::string filename_docids = "docids_memory_cached_data.bin"
    ) :
        IndexBase<DocID,TokenID>( directory ),
        InvertedListOffsets<DocID,TokenID>( directory + filename_offsets ),
        docids_data_(NULL),
        docids_data_filename_(filename_docids),
        docids_data_size_bytes_(0)
    {
        readMemoryCache();
    }

    ~IndexMemoryCached()
    {
        delete[] docids_data_;
    }

    virtual bool getDocIDs( const TokenID& tokenid, DocID*& docid_ptr, DocID*& docid_ptr_end ) const
    {
        return true;//InvertedListOffsets<DocID,TokenID>::getDocIDs( tokenid, docid_ptr, docid_ptr_end );
    }

    virtual bool getDocIDs( const Query<TokenID>& query, DocID*& docid_ptr, DocID*& docid_ptr_end ) const
    {
        return true;//InvertedListOffsets<DocID,TokenID>::getDocIDs( query, docid_ptr, docid_ptr_end );
    }

    bool readMemoryCache()
    {
        if( docids_data_filename_ == "" ) return false;

        /*
        std::string filename = IndexBase<DocID,TokenID>::getDirectory() + filename_docids_;

        std::cout << "IndexMemoryCached::writeMemoryCache : opening inverted lists file <" << filename << ">..." << std::flush;

          std::fstream file( filename.c_str(), std::ios::binary | std::fstream::in );
          if( !file.is_open() ) return false;
          file.read( reinterpret_cast<char*>(&docids_data_size_bytes_), sizeof(unsigned long) );

        std::cout << "done.\nIndexMemoryCached::writeMemoryCache : allocating " << docids_data_size_bytes_ << " bytes..." << std::flush;

          delete[] docids_data_;

          docids_data_ = new DocID[ docids_data_size_bytes_ / sizeof(DocID) ];

        std::cout << "done.\nIndexMemoryCached::writeMemoryCache : reading inverted lists..." << std::flush;

          file.read( reinterpret_cast<char*>(docids_data_), docids_data_size_bytes_ );

          file.close();

        std::cout << "done.\nIndexMemoryCached::writeMemoryCache : computing inverted list pointers..." << std::flush;

          InvertedListOffsets<DocID,TokenID>::convertOffsetsToPointers( docids_data_ );

        std::cout << "done.\n";
        */

        return true;
    }

    bool writeMemoryCache( const std::vector<TokenID>& tokenids, const std::vector<Query<TokenID> >& queries )
    {
        /*
        std::cout << "IndexMemoryCached::writeMemoryCache : finding the size of the largest inverted list..." << std::flush;

          IndexDiskCached<DocID,TokenID> indexdiskcached( IndexBase<DocID,TokenID>::getDirectory() );

          unsigned int nbuffer_values = 0;

          for( typename std::vector<TokenID>::iterator tokenid = tokenids.begin() ; tokenid != tokenids.end() ; ++tokenid )
          {
            max( nbuffer_values, indexdiskcached.getNDocIDs( *tokenid ) );
          }

        std::cout << " longest has " << nbuffer_values << " values, taking up " << nbuffer_values * sizeof(DocID) << ".\nIndexMemoryCached::writeMemoryCache : allocating memory..." << std::flush;

          DocID* buffer = new DocID[ nbuffer_values ];

        std::cout << "done.\nIndexMemoryCached::writeMemoryCache : writing inverted lists to file, processing token id =" << std::flush;

          unsigned int ndocids;

          unsigned long docids_data_size_values = 0;

          DocID* docid_ptrs;

          DocID* docid_ptrs_end;

          std::string filename = IndexBase<DocID,TokenID>::getDirectory() + filename_docids_;
          std::fstream file( filename.c_str(), std::ios::binary | std::ios::out );
          if( !file.is_open() ) return false;

          for( typename std::vector<TokenID>::iterator tokenid = tokenids.begin() ; tokenid != tokenids.end() ; ++tokenid )
          {
            std::cout << " " << *tokenid << std::flush;

            if( !indexdiskcached.getDocIDs( *tokenid, buffer, nbuffer_values, docid_ptrs, docid_ptrs_end, ndocids ) ) return false;

            if( !filterMemoryCachedInvertedList( buffer, ndocids ) ) return false;

            docids_data_size_values += ndocids;

            file.write( reinterpret_cast<char*>(buffer), ndocids * sizeof(DocID) );
          }

          docids_data_size_bytes_ = docids_data_size_values * sizeof(DocID);
          file.seekg( 0, std::ios::beg );
          file.write( reinterpret_cast<char*>(&docids_data_size_bytes_), sizeof(unsigned long) );
          file.close();

        std::cout << "\nIndexMemoryCached::writeMemoryCache : written " << docids_data_size_bytes_ << " bytes (=" << (double)docids_data_size_bytes_/1024/1024/1024 << " Gb).\nIndexMemoryCached::writeMemoryCache : finished.";

        delete[] buffer;
        */

        return true;
    }

    unsigned long getDocIDsDataSizeBytes()
    {
        return docids_data_size_bytes_;
    }

    std::string getDocIDsDataFilename()
    {
        return docids_data_filename_;
    }

protected:

    DocID* getDocIDsDataPtr()
    {
        return docids_data_filename_;
    }

private:

    DocID* docids_data_;

    std::string docids_data_filename_;

    unsigned long docids_data_size_bytes_;

    virtual bool filterMemoryCachedInvertedList( DocID* docids, unsigned int& ndocids )
    {
        return true;
    }

};

}
}
}

#endif
