#include <string.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sstream>

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <net/mrlite/mrlite.h>
#include <net/mrlite/sorted_buffer_iterator.h>
#include <net/mrlite/util.h>

using namespace std;
using namespace net::mrlite;

class WordCountMapper : public Mapper
{
public:
    void Map(const std::string& key, const std::string& value)
    {
        std::vector<std::string> words;
        SplitStringUsing(value, " ", &words);
        for (unsigned i = 0; i < words.size(); ++i)
        {
            Output(words[i], "1");
        }
    }
};

class WordCountMapperWithCombiner : public Mapper
{
public:
    void Map(const std::string& key, const std::string& value)
    {
        std::vector<std::string> words;
        SplitStringUsing(value, " ", &words);
        for (unsigned i = 0; i < words.size(); ++i)
        {
            ++combined_results[words[i]];
        }
    }

    void Flush()
    {
        for (map<string, int>::const_iterator i = combined_results.begin();
                i != combined_results.end(); ++i)
        {
            ostringstream formater;
            formater << i->second;
            Output(i->first, formater.str());
        }
        // NOTE: for the correctness when we enable periodic-flush, we
        // must clear the intermediate data after flush it.
        combined_results.clear();
    }
private:
    map<string, int> combined_results;
};


class WordCountReducer : public IncrementalReducer
{
public:
    void Start()
    {
        Output("", "Following lines are reduce outputs:");
    }

    void* BeginReduce(const std::string& key, const std::string& value)
    {
        istringstream parser(value);
        int* r = new int;
        parser >> *r;
        return r;
    }

    void PartialReduce(const std::string& key,
                       const std::string& value,
                       void* partial_result)
    {
        istringstream parser(value);
        int count = 0;
        parser >> count;
        *static_cast<int*>(partial_result) += count;
    }

    void EndReduce(const string& key, void* partial_result)
    {
        int* p = static_cast<int*>(partial_result);
        ostringstream formater;
        formater << key << " " << *p; // If output format is Text, only value will
        Output(key, formater.str());  // be written.  So we write key into value.
        delete p;
    }

    void Flush()
    {
        Output("", "Above lines are reduce outputs.");
    }
};


class WordCountBatchReducer : public BatchReducer
{
public:
    void Start()
    {
        //Output("", "Following lines are MR reduce outputs:");
    }

    void Reduce(const string& key, ReduceInputIterator* values)
    {
        int sum = 0;
        LOG(INFO) << "key:[" << key << "]";
        for (; !values->Done(); values->Next())
        {
            //LOG(INFO) << "value:[" << values->value() << "]";
            istringstream parser(values->value());
            int count = 0;
            parser >> count;
            sum += count;
        }
        ostringstream formater;
        formater << key << " " << sum;
        Output(key, formater.str());

        // Example of writing logs, choices are LOG(INFO), LOG(WARNING), LOG(FATAL)
        LOG(INFO) << "Reduce for key : " << key;
    }

    void Flush()
    {
        //Output("", "Above lines are MR reduce outputs.");
    }
};

