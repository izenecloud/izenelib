#ifndef DNF_H_
#define DNF_H_

#include <vector>
#include <string>
#include <types.h>

namespace izenelib { namespace ir { namespace be_index {

class Assignment {
public:
    Assignment()
    {
    }

    std::string attribute;
    bool belongsTo;
    std::vector<std::string> values;

    Assignment(const std::string & attribute, bool belongsTo, const std::vector<std::string> & values): attribute(attribute), belongsTo(belongsTo), values(values)
    {
    }

    // assumes values.size() != 0
    std::string toString() const
    {
        std::string ret;
        ret += attribute;
        ret += " ";
        ret += (belongsTo ? "t" : "f");
        ret += " ";
        for (std::size_t i = 0; i != values.size() - 1; ++i) {
            ret += values[i];
            ret += " ";
        }
        ret += values.back();

        return ret;
    }
};

class Conjunction {
public:
    Conjunction()
    {
    }

    std::vector<Assignment> assignments;

    Conjunction(const std::vector<Assignment> & assignments): assignments(assignments)
    {
    }

    uint32_t getK() const
    {
        uint32_t ret = 0;
        for (std::size_t i = 0; i != assignments.size(); ++i) {
            if (assignments[i].belongsTo == true) {
                ++ret;
            }
        }
        return ret;
    }

    // assumes assignments.size() != 0
    std::string toString() const
    {
        std::string ret;
        for (std::size_t i = 0; i != assignments.size() - 1; ++i) {
            ret += assignments[i].toString();
            ret += " & ";
        }
        ret += assignments.back().toString();

        return ret;
    }
};

class DNF {
public:
    DNF()
    {
    }

    std::vector<Conjunction> conjunctions;

    DNF(const std::vector<Conjunction> & conjunctions): conjunctions(conjunctions)
    {
    }

    // assumes conjunctions.size() != 0
    std::string toString() const
    {
        std::string ret;
        for (std::size_t i = 0; i != conjunctions.size() - 1; ++i) {
            ret += conjunctions[i].toString();
            ret += " | ";
        }
        ret += conjunctions.back().toString();

        return ret;
    }
};

}}}

#endif
