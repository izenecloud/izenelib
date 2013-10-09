#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace izenelib{ namespace ir{ namespace be{

using namespace boost::assign;

class Assignment
{
public:
    int attribute;
    bool belongsTo;
    std::vector<int> values;

    Assignment(int attribute, bool belongsTo, const std::vector<int> & values): attribute(attribute), belongsTo(belongsTo), values(values)
    {
    }
};

class Conjunction
{
public:
    int conjunctionID;
    std::vector<Assignment> assignments;

    Conjunction(int conjunctionID, const std::vector<Assignment> & assignments): conjunctionID(conjunctionID), assignments(assignments)
    {
    }

    int getK() const
    {
        int ret = 0;
        for (std::size_t i = 0; i != assignments.size(); ++i)
        {
            if (assignments[i].belongsTo == true)
            {
                ++ret;
            }
        }
        return ret;
    }
};

class DNF
{
public:
    int dnfID;
    std::vector<Conjunction> conjunctions;

    DNF(int dnfID, const std::vector<Conjunction> & conjunctions): dnfID(dnfID), conjunctions(conjunctions)
    {
    }
};

class ConjunctionInvIndex
{
public:
    typedef boost::unordered_map<int, boost::unordered_map<std::pair<int, int>, std::vector<std::pair<int, bool> > > > TindexOuter;
    typedef boost::unordered_map<std::pair<int, int>, std::vector<std::pair<int, bool> > > TindexInner;
    typedef std::map<int, boost::unordered_map<std::pair<int, int>, std::vector<std::pair<int, bool> > > *> TKorderedIndex;

    boost::unordered_map<int, boost::unordered_map<std::pair<int, int>, std::vector<std::pair<int, bool> > > > index;
    std::vector<std::pair<int, bool> > zeroConjunctionList;
    std::map<int, boost::unordered_map<std::pair<int, int>, std::vector<std::pair<int, bool> > > *> KorderedIndex;

    ConjunctionInvIndex()
    {
    }

    void addConjunction(const Conjunction & conjunction)
    {
        int K = conjunction.getK();

        if (K == 0)
        {
            zeroConjunctionList.push_back(std::make_pair(conjunction.conjunctionID, true));
        }

        for (std::size_t i = 0; i != conjunction.assignments.size(); ++i)
        {
            for (std::size_t j = 0; j != conjunction.assignments[i].values.size(); ++j)
            {
                index[K][std::make_pair(conjunction.assignments[i].attribute, conjunction.assignments[i].values[j])].push_back(std::make_pair(conjunction.conjunctionID, conjunction.assignments[i].belongsTo));
            }
        }

        if (KorderedIndex.find(K) == KorderedIndex.end())
        {
            KorderedIndex[K] = &(index[K]);
        }
    }

    void retrieve(const std::vector<std::pair<int, int> > & assignment, std::vector<int> & conjunctionIDs)
    {
        for (TKorderedIndex::iterator i = KorderedIndex.begin(); i != KorderedIndex.end() && i->first <= (int)assignment.size(); ++i)
        {
            std::list<std::vector<std::pair<int, bool> > *> lists;
            std::list<std::size_t> positions;
            if (i->first == 0)
            {
                lists.push_back(&zeroConjunctionList);
                positions.push_back(0);
            }
            initPostingLists(lists, positions, assignment, i->second);

            int K = i->first;
            if (K == 0)
            {
                K = 1;
            }

            while ((int)lists.size() >= K)
            {
                // std::cout << lists.size() << std::endl;
                // std::list<std::vector<std::pair<int, bool> > *>::iterator m = lists.begin();
                // std::list<std::size_t>::iterator p = positions.begin();
                // for (; m != lists.end(); ++m, ++p) {
                //     std::cout << *p << std::endl;
                //     for (auto n: (*(*m))) {
                //         std::cout << n.first << '\t' << n.second << std::endl;
                //     }
                // }

                std::priority_queue<int> pq;
                int min = (*lists.front())[positions.front()].first;
                int minCnt = 0;
                bool invalid = false;

                std::list<std::vector<std::pair<int, bool> > *>::iterator j = lists.begin();
                std::list<std::size_t>::iterator k = positions.begin();
                while (j != lists.end())
                {
                    int cid = (*(*j))[*k].first;
                    bool belongsTo = (*(*j))[*k].second;

                    if ((int)pq.size() < K)
                    {
                        pq.push(cid);
                    }
                    else if (cid < pq.top())
                    {
                        pq.pop();
                        pq.push(cid);
                    }

                    if (cid < min)
                    {
                        min = cid;
                        minCnt = 1;
                        invalid = !belongsTo;
                    }
                    else if (cid == min)
                    {
                        ++minCnt;
                        if (!belongsTo)
                        {
                            invalid = true;
                        }
                    }

                    ++j;
                    ++k;
                }

                if (minCnt == K && !invalid)
                {
                    conjunctionIDs.push_back(min);
                }

                if (min == pq.top())
                {
                    skipTo(lists, positions, min + 1);
                }
                else
                {
                    skipTo(lists, positions, pq.top());
                }
            }
        }
    }

    void skipTo(std::list<std::vector<std::pair<int, bool> > *> & lists, std::list<std::size_t> & positions, int cid)
    {
        std::list<std::vector<std::pair<int, bool> > *>::iterator i = lists.begin();
        std::list<std::size_t>::iterator j = positions.begin();
        while (i != lists.end())
        {
            while (*j != (*(*i)).size() && (*(*i))[*j].first < cid)
            {
                ++(*j);
            }

            if (*j == (*(*i)).size())
            {
                i = lists.erase(i);
                j = positions.erase(j);
            }
            else
            {
                ++i;
                ++j;
            }
        }
    }

    void initPostingLists(std::list<std::vector<std::pair<int, bool> > *> & lists, std::list<std::size_t> & positions, const std::vector<std::pair<int, int> > & assignment, TindexInner * indexInner)
    {
        for (std::size_t i = 0; i != assignment.size(); ++i)
        {
            TindexInner::iterator p = indexInner->find(assignment[i]);
            if (p != indexInner->end())
            {
                lists.push_back(&(p->second));
                positions.push_back(0);
            }
        }
    }

    void show()
    {
        for (TindexOuter::iterator i = index.begin(); i != index.end(); ++i)
        {
            std::cout << "K = " << i->first << std::endl;
            for (TindexInner::iterator j = i->second.begin(); j != i->second.end(); ++j)
            {
                std::cout << "attribute = " << j->first.first << ", value = " << j->first.second << ":" << std::endl;
                for (std::size_t k = 0; k != j->second.size(); ++k)
                {
                    std::cout << "conjunction id = " << j->second[k].first << ", belongsTo = " << (j->second[k].second ? "true" : "false") << std::endl;
                }
            }
        }

        std::cout << "zeroConjunctionList:" << std::endl;
        for (std::size_t i = 0; i != zeroConjunctionList.size(); ++i)
        {
            std::cout << "conjunction id = " << zeroConjunctionList[i].first << ", belongsTo = " << (zeroConjunctionList[i].second ? "true" : "false") << std::endl;
        }
    }
};

class DNFInvIndex
{
public:
    ConjunctionInvIndex conjIndex;
    boost::unordered_map<int, std::vector<int> > dnfIndex;

    DNFInvIndex()
    {
    }

    void addDNF(const DNF & dnf)
    {
        for (std::size_t i = 0; i != dnf.conjunctions.size(); ++i)
        {
            boost::unordered_map<int, std::vector<int> >::iterator j = dnfIndex.find(dnf.conjunctions[i].conjunctionID);
            if (j != dnfIndex.end())
            {
                j->second.push_back(dnf.dnfID);
            }
            else
            {
                dnfIndex.insert(std::make_pair(dnf.conjunctions[i].conjunctionID, list_of(dnf.dnfID).convert_to_container<std::vector<int> >()));
                conjIndex.addConjunction(dnf.conjunctions[i]);
            }
        }
    }

    // may do merge.
    void retrieve(const std::vector<std::pair<int, int> > & assignment, boost::unordered_set<int> & dnfIDs)
    {
        std::vector<int> conjunctionIDs;
        conjIndex.retrieve(assignment, conjunctionIDs);
        for (std::size_t i = 0; i != conjunctionIDs.size(); ++i)
        {
            for (std::size_t j = 0; j != dnfIndex[conjunctionIDs[i]].size(); ++j)
            {
                dnfIDs.insert(dnfIndex[conjunctionIDs[i]][j]);
            }
        }
    }
};

}}}

