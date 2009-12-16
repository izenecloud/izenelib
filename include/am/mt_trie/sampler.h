/**
 * @brief Collect given number of samples from a stream.
 * @author Wei Cao
 * @date 2009-12-11
 */

#ifndef _STREAM_SAMPLER_H_
#define _STREAM_SAMPLER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>

NS_IZENELIB_AM_BEGIN

template <typename StringType>
class StreamSampler
{

public:

    StreamSampler(const std::string& path)
    :   path_(path), step_(1), skip_(0),
        currentPosition_(0), lastSampledPosition_(0)
    {
        load();
    }

    void flush()
    {
        sync();
    }

    void collect(const StringType& term)
    {
        skip_ ++;
        if(skip_ == step_) {
            skip_ = 0;
            size_t measure = currentPosition_-lastSampledPosition_;

            // havn't collect enough samples from sequence
            if(samples_.size() < maximumSampleNum_) {
                samples_.insert(std::make_pair(currentPosition_ , term));
                associations_.insert(std::make_pair(currentPosition_ ,  measures_.insert(
                    std::make_pair(measure, currentPosition_) )));
                lastSampledPosition_ = currentPosition_;
            } else {
                //  replace existing samples with new one
                size_t existingLeastMeasure = measures_.begin()->first;

                if( existingLeastMeasure < measure ) {
                    // step 1. erase replaced sample from samples_
                    size_t replacedLine = measures_.begin()->second;
                    samples_.erase(replacedLine);
                    measures_.erase(measures_.begin());
                    associations_.erase(replacedLine);

                    // step 2. add measure of replaced line to the next line
                    size_t updatedLine = samples_.upper_bound(replacedLine)->first;
                    size_t updatedMeasure = existingLeastMeasure +
                        associations_[updatedLine]->first;
                    measures_.erase(associations_[updatedLine]);
                    associations_.erase(updatedLine);
                    associations_.insert(std::make_pair( updatedLine, measures_.insert(
                        std::make_pair(updatedMeasure, updatedLine) )));

                    // step 3. insert new line
                    samples_.insert(std::make_pair(currentPosition_, term));
                    associations_.insert(std::make_pair( currentPosition_, measures_.insert(
                        std::make_pair(measure, currentPosition_))));
                    lastSampledPosition_ = currentPosition_;
                } else {
                    step_ *= 2;
                }
            }
        }
        currentPosition_ ++;
    }

    void getSamples(std::set<StringType>& container) {
        for( typename std::map<size_t, StringType>::iterator it =
            samples_.begin(); it != samples_.end(); it++ ) {
            container.insert(it->second);
        }
    }

protected:

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::make_nvp("Step", step_);
        ar & boost::serialization::make_nvp("Skip", skip_);
        ar & boost::serialization::make_nvp("CurrentPosition", currentPosition_);
        ar & boost::serialization::make_nvp("LastSampledPosition", lastSampledPosition_);

        std::map<size_t, std::string> buffer;
        for( typename std::map<size_t, StringType>::const_iterator it = samples_.begin();
            it != samples_.end(); it++ ) {
            // convert StringType to string
            size_t p = it->first;
            std::string s((char*)it->second.c_str(), it->second.size());
            buffer.insert(std::make_pair(p,s));
        }

        ar & boost::serialization::make_nvp("Samples", buffer);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::make_nvp("Step", step_);
        ar & boost::serialization::make_nvp("Skip", skip_);
        ar & boost::serialization::make_nvp("CurrentPosition", currentPosition_);
        ar & boost::serialization::make_nvp("LastSampledPosition", lastSampledPosition_);

        std::map<size_t, std::string> buffer;
        ar & boost::serialization::make_nvp("Samples", buffer);

        size_t last = 0;
        for( std::map<size_t,std::string>::iterator it = buffer.begin();
            it != buffer.end(); it++ ) {
            // convert std::string to StringType
            size_t p = it->first;
            StringType s(it->second);
            samples_.insert(std::make_pair(p,s));
            associations_.insert( std::make_pair( p ,
                measures_.insert(std::make_pair(p-last,p))
            ));
            last = p;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    bool load()
    {
	    ifstream ifs(path_.c_str());
        if( ifs ) {
            try {
                boost::archive::xml_iarchive xml(ifs);
                xml >> boost::serialization::make_nvp("StreamSampler", *this);
            } catch (...) {
                throw std::runtime_error("StreamSampler config file corrputed");
            }
            ifs.close();
            return true;
        }
        return false;
    }

    void sync()
    {
            ofstream ofs(path_.c_str());
            boost::archive::xml_oarchive xml(ofs);
            xml << boost::serialization::make_nvp("StreamSampler", *this);
            ofs.flush();
    }

private:

    std::string path_;

    const static size_t maximumSampleNum_ = 65556;

    size_t step_;

    size_t skip_;

    size_t currentPosition_;

    size_t lastSampledPosition_;

    std::map<size_t, StringType> samples_;

    std::multimap<size_t, size_t> measures_;

    std::map<size_t, std::multimap<size_t, size_t>::iterator> associations_;

};

NS_IZENELIB_AM_END

#endif
