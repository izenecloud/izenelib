#ifndef INVINDEX_H_
#define INVINDEX_H_

#include <types.h>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <utility>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <sstream>
#include "IDMapper.hpp"
#include "AVMapper.hpp"
#include "DNF.hpp"
#include "SimpleSerialization.hpp"
#include <3rdparty/json/json.h>

namespace izenelib { namespace ir { namespace be_index {

class ConjunctionInvIndex {
public:
    typedef std::map<uint32_t, boost::unordered_map<std::pair<uint32_t, uint32_t>, std::vector<std::pair<uint32_t, bool> > > > OuterT;
    typedef boost::unordered_map<std::pair<uint32_t, uint32_t>, std::vector<std::pair<uint32_t, bool> > > InnerT;

    // assumes number of attributes are not too large, use std::map to keep K ordered.
    std::map<uint32_t, boost::unordered_map<std::pair<uint32_t, uint32_t>, std::vector<std::pair<uint32_t, bool> > > > index;
    std::vector<std::pair<uint32_t, bool> > zeroConjunctionList;
    AVMapper avMapper;

    ConjunctionInvIndex()
    {
    }

    ~ConjunctionInvIndex()
    {
    }

    void addConjunction(uint32_t conjunctionID, const Conjunction & conjunction)
    {
        int K = conjunction.getK();

        if (K == 0) {
            zeroConjunctionList.push_back(std::make_pair(conjunctionID, true));
        }

        for (std::size_t i = 0; i != conjunction.assignments.size(); ++i) {
            for(std::size_t j = 0; j != conjunction.assignments[i].values.size(); ++j) {
                std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.insert(conjunction.assignments[i].attribute, conjunction.assignments[i].values[j]);
                index[K][std::make_pair(avResult.first.first, avResult.second.first)].push_back(std::make_pair(conjunctionID, conjunction.assignments[i].belongsTo));
            }
        }
    }

    // special case: assignment has some nonexistent a v pair.
    void retrieve(const std::vector<std::pair<std::string, std::string> > & assignment, std::vector<uint32_t> & conjunctionIDs)
    {
        for (OuterT::iterator i = index.begin(); i != index.end() && i->first <= assignment.size(); ++i) {
            std::list<std::vector<std::pair<uint32_t, bool> > *> lists;
            std::list<std::size_t> positions;
            if (i->first == 0) {
                lists.push_back(&zeroConjunctionList);
                positions.push_back(0);
            }
            initPostingLists(lists, positions, assignment, i->second);

            uint32_t K = i->first;
            if (K == 0) {
                K = 1;
            }

            while (lists.size() >= K) {
                std::priority_queue<uint32_t> pq;
                uint32_t min = (*lists.front())[positions.front()].first;
                uint32_t minCnt = 0;
                bool invalid = false;

                std::list<std::vector<std::pair<uint32_t, bool> > *>::iterator j = lists.begin();
                std::list<std::size_t>::iterator k = positions.begin();
                while (j != lists.end()) {
                    uint32_t cid = (*(*j))[*k].first;
                    bool belongsTo = (*(*j))[*k].second;

                    if (pq.size() < K) {
                        pq.push(cid);
                    } else if (cid < pq.top()) {
                        pq.pop();
                        pq.push(cid);
                    }

                    if (cid < min) {
                        min = cid;
                        minCnt = 1;
                        invalid = !belongsTo;
                    } else if (cid == min) {
                        ++minCnt;
                        if (!belongsTo) {
                            invalid = true;
                        }
                    }

                    ++j;
                    ++k;
                }

                if (minCnt == K && !invalid) {
                    conjunctionIDs.push_back(min);
                }

                if (min == pq.top()) {
                    skipTo(lists, positions, min + 1);
                } else {
                    skipTo(lists, positions, pq.top());
                }
            }
        }
    }

    void skipTo(std::list<std::vector<std::pair<uint32_t, bool> > *> & lists, std::list<std::size_t> & positions, uint32_t cid)
    {
        std::list<std::vector<std::pair<uint32_t, bool> > *>::iterator i = lists.begin();
        std::list<std::size_t>::iterator j = positions.begin();
        while (i != lists.end()) {
            while (*j != (*(*i)).size() && (*(*i))[*j].first < cid) {
                ++(*j);
            }

            if (*j == (*(*i)).size()) {
                i = lists.erase(i);
                j = positions.erase(j);
            } else {
                ++i;
                ++j;
            }
        }
    }

    void initPostingLists(std::list<std::vector<std::pair<uint32_t, bool> > *> & lists, std::list<std::size_t> & positions, const std::vector<std::pair<std::string, std::string> > & assignment, InnerT & indexInner)
    {
        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.insert(assignment[i].first, assignment[i].second);
            InnerT::iterator p = indexInner.find(std::make_pair(avResult.first.first, avResult.second.first));
            if (p != indexInner.end()) {
                lists.push_back(&(p->second));
                positions.push_back(0);
            }
        }
    }

    void show()
    {
        for (OuterT::iterator i = index.begin(); i != index.end(); ++i) {
            std::cout << "K = " << i->first << std::endl;
            for (InnerT::iterator j = i->second.begin(); j != i->second.end(); ++j) {
                std::cout << "attribute = " << j->first.first << ", value = " << j->first.second << ":" << std::endl;
                for (std::size_t k = 0; k != j->second.size(); ++k) {
                    std::cout << "conjunction id = " << j->second[k].first << ", belongsTo = " << (j->second[k].second ? "true" : "false") << std::endl;
                }
            }
        }

        std::cout << "zeroConjunctionList:" << std::endl;
        for (std::size_t i = 0; i != zeroConjunctionList.size(); ++i) {
            std::cout << "conjunction id = " << zeroConjunctionList[i].first << ", belongsTo = " << (zeroConjunctionList[i].second ? "true" : "false") << std::endl;
        }
    }

    void toJson(Json::Value & root)
    {
        indexToJson(root["index"]);
        zeroConjunctionListToJson(root["zeroConjunctionList"]);
        avMapper.toJson(root["avMapper"]);
    }

    void indexToJson(Json::Value & root)
    {
        for (OuterT::iterator i = index.begin(); i != index.end(); ++i) {
            std::ostringstream out;
            out << i->first;
            innerToJson(root[out.str()], i->second);
        }
    }

    void innerToJson(Json::Value & root, InnerT & inner)
    {
        for (InnerT::iterator i = inner.begin(); i != inner.end(); ++i) {
            std::ostringstream out;
            out << i->first.first << ' ' << i->first.second;
            postingListToJson(root[out.str()], i->second);
        }
    }

    void postingListToJson(Json::Value & root, std::vector<std::pair<uint32_t, bool> > & postingList)
    {
        for (std::size_t i = 0; i != postingList.size(); ++i) {
            conjunctionPairToJson(root[i], postingList[i]);
        }
    }

    void conjunctionPairToJson(Json::Value & root, std::pair<uint32_t, bool> & conjunctionPair)
    {
        root[Json::Value::UInt(0U)] = Json::Value::UInt(conjunctionPair.first);
        root[Json::Value::UInt(1U)] = conjunctionPair.second;
    }

    void zeroConjunctionListToJson(Json::Value & root)
    {
        for (std::size_t i = 0; i != zeroConjunctionList.size(); ++i) {
            conjunctionPairToJson(root[i], zeroConjunctionList[i]);
        }
    }

    void fromJson(Json::Value & root)
    {
        indexFromJson(root["index"]);
        zeroConjunctionListFromJson(root["zeroConjunctionList"]);
        avMapper.fromJson(root["avMapper"]);
    }

    void indexFromJson(Json::Value & root)
    {
        for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
            std::istringstream in(i.key().asString());
            uint32_t K;
            in >> K;
            innerFromJson(*i, index[K]);
        }
    }

    void innerFromJson(Json::Value & root, InnerT & inner)
    {
        for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
            std::istringstream in(i.key().asString());
            uint32_t attrID, valueID;
            in >> attrID >> valueID;
            postingListFromJson(*i, inner[std::make_pair(attrID, valueID)]);
        }
    }

    void postingListFromJson(Json::Value & root, std::vector<std::pair<uint32_t, bool> > & postingList)
    {
        postingList.resize(root.size());
        for (std::size_t i = 0; i != root.size(); ++i) {
            conjunctionPairFromJson(root[i], postingList[i]);
        }
    }

    void conjunctionPairFromJson(Json::Value & root, std::pair<uint32_t, bool> & conjunctionPair)
    {
        conjunctionPair.first = root[Json::Value::UInt(0U)].asUInt();
        conjunctionPair.second = root[Json::Value::UInt(1U)].asBool();
    }

    void zeroConjunctionListFromJson(Json::Value & root)
    {
        zeroConjunctionList.resize(root.size());
        for (std::size_t i = 0; i != root.size(); ++i) {
            conjunctionPairFromJson(root[i], zeroConjunctionList[i]);
        }
    }

    void save_binary(std::ostream & os)
    {
        serialize(index.size(), os);
        for (OuterT::iterator i = index.begin(); i != index.end(); ++i) {
            serialize(i->first, os);

            serialize(i->second.size(), os);
            std::vector<std::pair<std::pair<uint32_t, uint32_t>, std::vector<std::pair<uint32_t, bool> > > > temp(i->second.begin(), i->second.end());
            std::sort(temp.begin(), temp.end());
            for (std::size_t j = 0; j != temp.size(); ++j) {
                serialize(temp[j].first.first, os);
                serialize(temp[j].first.second, os);

                serialize(temp[j].second.size(), os);
                for (std::size_t k = 0; k != temp[j].second.size(); ++k) {
                    serialize(temp[j].second[k].first, os);
                    serialize(temp[j].second[k].second, os);
                }
            }
        }

        serialize(zeroConjunctionList.size(), os);
        for (std::size_t i = 0; i != zeroConjunctionList.size(); ++i) {
            serialize(zeroConjunctionList[i].first, os);
            serialize(zeroConjunctionList[i].second, os);
        }

        avMapper.save_binary(os);
    }

    void load_binary(std::istream & is)
    {
        index.clear();
        std::size_t OuterSize;
        deserialize(is, OuterSize);
        for (std::size_t i = 0; i != OuterSize; ++i) {
            uint32_t K;
            deserialize(is, K);

            std::size_t InnerSize;
            deserialize(is, InnerSize);
            InnerT innerIndex;
            for (std::size_t j = 0; j != InnerSize; ++j) {
                uint32_t attrID;
                deserialize(is, attrID);
                uint32_t valueID;
                deserialize(is, valueID);

                std::size_t conjunctionListSize;
                deserialize(is, conjunctionListSize);
                std::vector<std::pair<uint32_t, bool> > conjunctionList(conjunctionListSize);
                for (std::size_t k = 0; k != conjunctionListSize; ++k) {
                    deserialize(is, conjunctionList[k].first);
                    deserialize(is, conjunctionList[k].second);
                }

                innerIndex[std::make_pair(attrID, valueID)] = conjunctionList;
            }

            index[K] = innerIndex;
        }

        std::size_t zeroConjunctionListSize;
        deserialize(is, zeroConjunctionListSize);
        zeroConjunctionList.resize(zeroConjunctionListSize);
        for (std::size_t i = 0; i != zeroConjunctionListSize; ++i) {
            deserialize(is, zeroConjunctionList[i].first);
            deserialize(is, zeroConjunctionList[i].second);
        }

        avMapper.load_binary(is);
    }
};

class DNFInvIndex {
public:
    ConjunctionInvIndex conjIndex;
    boost::unordered_map<uint32_t, std::vector<uint32_t> > index;
    IDMapper conjunctionMapper;
    uint32_t numDNF;

    DNFInvIndex(): numDNF(0)
    {
    }

    ~DNFInvIndex()
    {
    }

    // interface addDNF:
    // add a dnf which stands for an advertiser's targeting requirement.
    void addDNF(uint32_t dnfID, const DNF & dnf)
    {
        for (std::size_t i = 0; i != dnf.conjunctions.size(); ++i) {
            std::pair<uint32_t, bool> conjunctionResult = conjunctionMapper.insert(dnf.conjunctions[i].toString());
            if (conjunctionResult.second == true) {
                conjIndex.addConjunction(conjunctionResult.first, dnf.conjunctions[i]);
            }
            index[conjunctionResult.first].push_back(dnfID);
        }

        ++numDNF;
    }

    // interface retrieve:
    // read in a given assignment, and returns the valid dnf's.
    void retrieve(const std::vector<std::pair<std::string, std::string> > & assignment, boost::unordered_set<uint32_t> & dnfIDs)
    {
        std::vector<uint32_t> conjunctionIDs;
        conjIndex.retrieve(assignment, conjunctionIDs);
        for (std::size_t i = 0; i != conjunctionIDs.size(); ++i) {
            for (std::size_t j = 0; j != index[conjunctionIDs[i]].size(); ++j) {
                dnfIDs.insert(index[conjunctionIDs[i]][j]);
            }
        }
    }

    // interface totalNumDNF:
    // returns the total number of indexed dnf's.
    uint32_t totalNumDNF() const
    {
        return numDNF;
    }

    // interface save:
    // serialize the index in json text format.
    void save(std::ostream & os)
    {
        Json::FastWriter writer;
        Json::Value root;
        this->toJson(root);
        os << writer.write(root);
    }

    // interface load:
    // deserialize the index in json text format.
    void load(std::istream & is)
    {
        Json::Reader reader;
        Json::Value root;
        bool successful = reader.parse(is, root);
        if (!successful) {
            std::cout << "json parse error" << std::endl;
            return;
        }
        fromJson(root);
    }

    // interface save_binary:
    // serialize the index in binary format(for size consideration).
    void save_binary(std::ostream & os)
    {
        conjIndex.save_binary(os);

        serialize(index.size(), os);
        std::vector<std::pair<uint32_t, std::vector<uint32_t> > > temp(index.begin(), index.end());
        std::sort(temp.begin(), temp.end());
        for (std::size_t i = 0; i != temp.size(); ++i) {
            serialize(temp[i].first, os);

            serialize(temp[i].second.size(), os);
            for (std::size_t j = 0; j != temp[i].second.size(); ++j) {
                serialize(temp[i].second[j], os);
            }
        }

        conjunctionMapper.save_binary(os);

        serialize(numDNF, os);
    }

    // interface load_binary:
    // deserialize the index in binary format(for size consideration).
    void load_binary(std::istream & is)
    {
        conjIndex.load_binary(is);

        index.clear();
        std::size_t indexSize;
        deserialize(is, indexSize);
        for (std::size_t i = 0; i != indexSize; ++i) {
            uint32_t conjunctionID;
            deserialize(is, conjunctionID);

            std::size_t dnfListSize;
            deserialize(is, dnfListSize);
            std::vector<uint32_t> dnfList(dnfListSize);
            for (std::size_t j = 0; j != dnfListSize; ++j) {
                deserialize(is, dnfList[j]);
            }

            index[conjunctionID] = dnfList;
        }

        conjunctionMapper.load_binary(is);

        deserialize(is, numDNF);
    }

    // interface show:
    // show the index in human readable format(mainly for debug purpose).
    void show()
    {
        conjIndex.show();

        std::cout << std::endl;

        for (boost::unordered_map<uint32_t, std::vector<uint32_t> >::iterator i = index.begin(); i != index.end(); ++i) {
            std::cout << "conjunction id = " << i->first << ":" << std::endl;
            for (std::size_t j = 0; j != i->second.size(); ++j) {
                std::cout << "DNF id = " << i->second[j] << std::endl;
            }
        }
    }

    void toJson(Json::Value & root)
    {
        conjIndex.toJson(root["conjIndex"]);
        indexToJson(root["index"]);
        conjunctionMapper.toJson(root["conjunctionMapper"]);
        root["numDNF"] = Json::Value::UInt(numDNF);
    }

    void indexToJson(Json::Value & root)
    {
        for (boost::unordered_map<uint32_t, std::vector<uint32_t> >::iterator i = index.begin(); i != index.end(); ++i) {
            std::ostringstream out;
            out << i->first;
            postingListToJson(root[out.str()], i->second);
        }
    }

    void postingListToJson(Json::Value & root, std::vector<uint32_t> & postingList)
    {
        for (std::size_t i = 0; i != postingList.size(); ++i) {
            root[i] = Json::Value::UInt(postingList[i]);
        }
    }

    void fromJson(Json::Value & root)
    {
        conjIndex.fromJson(root["conjIndex"]);
        indexFromJson(root["index"]);
        conjunctionMapper.fromJson(root["conjunctionMapper"]);
        numDNF = root["numDNF"].asUInt();
    }

    void indexFromJson(Json::Value & root)
    {
        for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
            std::istringstream in(i.key().asString());
            uint32_t conjunctionID;
            in >> conjunctionID;
            postingListFromJson(*i, index[conjunctionID]);
        }
    }

    void postingListFromJson(Json::Value & root, std::vector<uint32_t> & postingList)
    {
        for (std::size_t i = 0; i != root.size(); ++i) {
            postingList.push_back(root[i].asUInt());
        }
    }
};

}}}

#endif
