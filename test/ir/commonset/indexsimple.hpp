#ifndef _SF1R_NEW_INDEX_SIMPLE_HPP_
#define _SF1R_NEW_INDEX_SIMPLE_HPP_

#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <cstring>

#include <ir/commonset/index.hpp>
#include <ir/commonset/query.hpp>
#include <ir/commonset/commonset.hpp>

namespace izenelib
{
namespace ir
{
namespace commonset
{

class IndexSimple : public Index<>
{
public:

    IndexSimple( unsigned int ndocids_total, unsigned int ntokens_cached, unsigned int ntokens_disk, unsigned int ndocuments_common, const std::vector<unsigned int>& ndocids, const std::string& directory_name = "./", bool verbose = false );

    ~IndexSimple()
    {
        delete[] correct_docids_common_set_;
    }

    bool checkResult( CommonSet<>& common_set );

    virtual bool getDocIDs( const unsigned int& tokenid, unsigned int* buffer, unsigned int nbuffer, unsigned int*& docid_ptr, unsigned int*& docid_ptr_end ) const;

protected:

    unsigned int* correct_docids_common_set_;

    unsigned int ncorrect_docids_common_set_;

    unsigned int** docids_;

    unsigned int* ndocids_;

    unsigned int nunique_docids_;

    unsigned int ntokens_;

};

IndexSimple::IndexSimple( unsigned int ndocids_total, unsigned int ntokens_cached, unsigned int ntokens_disk, unsigned int ndocuments_common, const std::vector<unsigned int>& ndocids, const std::string& directory, bool verbose ) :
    Index<>(),
    correct_docids_common_set_( NULL ),
    ncorrect_docids_common_set_( ndocuments_common ),
    docids_( NULL ),
    ndocids_( NULL ),
    nunique_docids_ (ndocids_total),
    ntokens_( ntokens_cached > ntokens_disk ? ntokens_cached : ntokens_disk )
{
    std::cout << "IndexSimple::constructor : creating common doc ids..." << std::flush;

    /*
     *  randomly select document ids, until ndocuments_common doc ids have been selected.
     *  record the doc ids selected with booleans.
     */

    unsigned int ndocs = 0;
    unsigned int idoc_rand;
    boost::random_device rd;
    boost::mt19937 generator(rd());
    boost::random::uniform_real_distribution<> random(0,ndocids_total);

    // use_common stores which documents are going to be the common documents

    bool* use_common = new bool[ndocids_total];
    memset( use_common, 0, sizeof(bool) * ndocids_total );

    while( ndocs < ndocuments_common )
    {
        idoc_rand = boost::math::round(random(generator));
        if( !use_common[idoc_rand] )
        {
            ++ndocs;
            use_common[idoc_rand] = true;
        }
    }

    correct_docids_common_set_ = new unsigned int[ ndocuments_common ];

    for( unsigned int idoc = 0, iuse = 0 ; iuse < ndocids_total ; ++iuse )
    {
        if( use_common[iuse] ) correct_docids_common_set_[idoc++] = iuse;
    }

    std::cout << "done.\n\n";

    if(verbose)
    {
        std::cout << "IndexSimple::constructor : common doc ids [" << ndocuments_common << "] : ";
        for( unsigned int idoc = 0 ; idoc < ndocuments_common ; ++idoc ) std::cout << correct_docids_common_set_[idoc] << " " ;
        std::cout << "\n\n";
    }

    std::cout << "IndexSimple::constructor : creating remaining doc ids..." << std::flush;

    /*
     *  for the remaining doc ids, select randomly in a similar fashion,
     *  but also check that the doc id is not in the common set.
     */

    bool** use = new bool*[ntokens_];
    unsigned int jtoken;

    for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
    {
        use[itoken] = new bool[ndocids_total];
        memcpy( use[itoken], use_common, sizeof(bool)*ndocids_total );
        ndocs = ndocuments_common;

        while( ndocs < ndocids[itoken] )
        {
            idoc_rand = boost::math::round(random(generator));
            if( !use[itoken][idoc_rand] )
            {
                jtoken = 0;
                while( jtoken < itoken && !use[jtoken][idoc_rand] )
                {
                    ++jtoken;
                }
                if( jtoken == itoken )
                {
                    ++ndocs;
                    use[itoken][idoc_rand] = true;
                }
            }
        }

    }

    // convert boolean arrays to doc id arrays

    docids_ = new unsigned int*[ntokens_];
    ndocids_ = new unsigned int[ntokens_];

    for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
    {
        docids_[itoken] = new unsigned int[ ndocids[itoken] ];
        ndocids_[itoken] = 0;

        for( unsigned int iuse = 0 ; iuse < ndocids_total ; ++iuse )
        {
            if( use[itoken][iuse] )
            {
                docids_[itoken][ ndocids_[itoken]++ ] = iuse; /*std::cout<< itoken << " : " << ndocids_[itoken]-1 << " = " << iuse << "\n";*/
            }
        }
    }

    std::cout << "done.\n\n";

    if(verbose)
    {
        for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken )
        {
            std::cout << "IndexSimple::constructor : token " << itoken << " [" << ndocids_[itoken] << "] : ";
            for( unsigned int idoc = 0 ; idoc < ndocids_[itoken] ; ++idoc ) std::cout << docids_[itoken][idoc] << " ";
            std::cout << "\n\n";
        }
    }

    for( unsigned int itoken = 0 ; itoken < ntokens_ ; ++itoken ) delete[] use[itoken];
    delete[] use;
    delete[] use_common;
}

bool IndexSimple::checkResult( CommonSet<>& common_set )
{
    unsigned int ndocids = common_set.getNResults();

    unsigned int* docids = new unsigned int[ ndocids ];

    common_set.getDocIDs( docids, ndocids );

    if( ndocids == 0 && ncorrect_docids_common_set_ != 0 ) return false;

    for( unsigned int idoc = 0 ; idoc < ndocids ; ++idoc ) if( docids[idoc] != correct_docids_common_set_[idoc] ) return false;

    return true;
}

bool IndexSimple::getDocIDs( const unsigned int& tokenid, unsigned int* buffer, unsigned int nbuffer, unsigned int*& docid_ptr, unsigned int*& docid_ptr_end ) const
{
    if( tokenid >= ntokens_ ) return false;
    docid_ptr = docids_[tokenid];
    docid_ptr_end = docid_ptr + ndocids_[tokenid];
    return true;
}

}
}
}

#endif
