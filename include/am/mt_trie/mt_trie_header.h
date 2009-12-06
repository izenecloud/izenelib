/**
 * @author Wei Cao
 * @date 2009-12-1
 */

#ifndef _MTTRIE_HEADER_H_
#define _MTTRIE_HEADER_H_

#include <string>
#include <vector>

#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

NS_IZENELIB_AM_BEGIN

/**
 * @brief Head of MtTrie
 */
template <typename StringType>
class MtTrieHeader {

public:

    /// @brief how many partitions do we divide all input terms into.
    int partitionNum;

//    /// @brief Number of terms in write cache.
//    size_t writeCacheLine;

    /// @brief boundaries between partitions, there are partitionNum_-1 elements.
    std::vector<StringType> boundaries;

    /**
     * @param path - path of header file.
     */
    MtTrieHeader(const std::string& path)
        : path_(path), exist_(false)
	{
	    ifstream ifs(path_.c_str());
        if( !ifs )
            return;

        ifs.seekg(0, ifstream::end);
        if( ifs.tellg()) {
            ifs.seekg(0, fstream::beg);
            boost::archive::xml_iarchive xml(ifs);
            xml >> boost::serialization::make_nvp("PartitionNumber", partitionNum);
//            xml >> boost::serialization::make_nvp("WriteCacheLine", writeCacheLine);
            std::vector<std::string> boundariesBuffer;
            for(size_t i =0; i< boundaries.size(); i++ ) {
                boundariesBuffer.push_back(std::string((char*)boundaries[i].c_str(),
                    boundaries[i].size()));
            }
            xml >> boost::serialization::make_nvp("Boundaries", boundariesBuffer);
            exist_ = true;
        }
        ifs.close();
    }

    ~MtTrieHeader()
    {
        flush();
    }

    /**
     * @brief Flush header to file
     */
    void flush()
    {
        ofstream ofs(path_.c_str());
        boost::archive::xml_oarchive xml(ofs);
        xml << boost::serialization::make_nvp("PartitionNumber", partitionNum);
//        xml << boost::serialization::make_nvp("WriteCacheLine", writeCacheLine);
        std::vector<std::string> boundariesBuffer;
        for(size_t i =0; i< boundaries.size(); i++ ) {
            boundariesBuffer.push_back( StringType(boundaries[i]) );
        }
        xml << boost::serialization::make_nvp("Boundaries", boundariesBuffer);
        ofs.flush();
    }

    bool exist()
    {
        return exist_;
    }

	void display(std::ostream& os = std::cout)
	{
		os << "Number of partitions " << partitionNum << std::endl;
	}

private:

    std::string path_;

    bool exist_;
};

NS_IZENELIB_AM_END

#endif
