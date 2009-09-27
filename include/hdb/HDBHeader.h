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
     * Number of sdbs.
     */
    size_t slicesNum;

    /**
     * Level of each sdb.
     */
    std::vector<int> slicesLevel;

    /**
     * Size of each sdb.
     */
    std::vector<size_t> deletions;

    HugeDBHeader(const std::string& path)
        : path_(path)
	{
	    ifstream ifs(path_.c_str());
        if( !ifs ) {
            slicesNum = 0;
            return;
        }

        ifs.seekg(0, ifstream::end);
        if( 0 == ifs.tellg()) {
            slicesNum = 0;
        } else {
            ifs.seekg(0, fstream::beg);
            boost::archive::xml_iarchive xml(ifs);
            xml >> boost::serialization::make_nvp("PartitionNumber", slicesNum);
            xml >> boost::serialization::make_nvp("PartitionLevel", slicesLevel);
            xml >> boost::serialization::make_nvp("Deletions", deletions);
        }
        ifs.close();
    }

    ~HugeDBHeader()
    {
        flush();
    }

    void flush()
    {
        ofstream ofs(path_.c_str());
        boost::archive::xml_oarchive xml(ofs);
        xml << boost::serialization::make_nvp("PartitionNumber", slicesNum);
        xml << boost::serialization::make_nvp("PartitionLevel", slicesLevel);
        xml << boost::serialization::make_nvp("Deletions", deletions);
        ofs.flush();
    }

	void display(std::ostream& os = std::cout)
	{
		os << "Number of partitions " << slicesNum << std::endl;
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
