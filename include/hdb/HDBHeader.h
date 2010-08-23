/**
 * @file HDBHeader.h
 * @brief Implementation of hdb header.
 * @author Wei Cao
 * @date 2009-09-11
 */

#ifndef _HDBHEADER_H_
#define _HDBHEADER_H_

#include <string>
#include <vector>

#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>

namespace izenelib {

namespace hdb {

/**
 * @brief Hdb header
 */
struct HugeDBHeader {

    /**
     * @brief Number of sdbs.
     */
    size_t slicesNum;

    /**
     * @brief Stamp for last modification.
     */
    size_t lastModificationStamp;

    /**
     * @brief Level of each sdb.
     */
    std::vector<int> slicesLevel;

    /**
     * @brief Size of each sdb.
     */
    std::vector<size_t> deletions;

    /**
     * @brief Deletion in memory partition.
     */
    size_t memoryPartitionDeletion;

    /**
     * @param path - path of header file.
     */
    HugeDBHeader(const std::string& path)
        : path_(path)
	{
	    ifstream ifs(path_.c_str());
        if( !ifs ) {
            slicesNum = 0;
            lastModificationStamp = 0;
            memoryPartitionDeletion = 0;
            return;
        }

        ifs.seekg(0, ifstream::end);
        if( 0 == ifs.tellg()) {
            slicesNum = 0;
            lastModificationStamp = 0;
            memoryPartitionDeletion = 0;
        } else {
            ifs.seekg(0, fstream::beg);
            boost::archive::xml_iarchive xml(ifs);
            xml >> boost::serialization::make_nvp("PartitionNumber", slicesNum);
            xml >> boost::serialization::make_nvp("LastModificationStamp", lastModificationStamp);
            xml >> boost::serialization::make_nvp("PartitionLevel", slicesLevel);
            xml >> boost::serialization::make_nvp("Deletions", deletions);
            xml >> boost::serialization::make_nvp("MemoryPartitionDeletion", memoryPartitionDeletion);
        }
        ifs.close();
    }

    ~HugeDBHeader()
    {
    }

    /**
     * @brief Flush header out to hdbname.hdb.header.xml
     */
    void flush()
    {
        ofstream ofs(path_.c_str());
        boost::archive::xml_oarchive xml(ofs);
        xml << boost::serialization::make_nvp("PartitionNumber", slicesNum);
        xml << boost::serialization::make_nvp("LastModificationStamp", lastModificationStamp);
        xml << boost::serialization::make_nvp("PartitionLevel", slicesLevel);
        xml << boost::serialization::make_nvp("Deletions", deletions);
        xml << boost::serialization::make_nvp("MemoryPartitionDeletion", memoryPartitionDeletion);
        ofs.flush();
    }

	void display(std::ostream& os = std::cout)
	{
		os << "Number of partitions " << slicesNum << std::endl;
		os << "Stamp of last modification " << lastModificationStamp << std::endl;
		if(slicesNum != 0) {
            os << "Partitions:";
            for(size_t i= 0; i<slicesNum; i++)
                os << " " << "[L" << slicesLevel[i] << "]"
                    << deletions[i] << "deletions";
            os << std::endl;
		}
	}

private:

    std::string path_;

};

}

}

#endif
