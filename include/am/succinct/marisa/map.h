#ifndef IZENELIB_AM_SUCCINCT_MARISA_MAP_H
#define IZENELIB_AM_SUCCINCT_MARISA_MAP_H

#include "marisa.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace marisa
{

/**
 * Succict Map using UX
 */
template <class V>
class Map
{
public:
    /**
     * Constructor
     */
    Map() : size_(0) {}

    /**
     * Destructor
     */
    ~Map() {}

    /**
     * Build a map from std::map
     * @param m A std::map as an input
     */
    bool build(const std::map<std::string, V>& m)
    {
        marisa::Keyset keyset;
        for (typename std::map<std::string, V>::const_iterator it =
                    m.begin(); it != m.end(); ++it)
        {
            keyset.push_back(it->first.c_str(), it->first.length(), 1.0F);
        }
        try
        {
            trie_.build(keyset, MARISA_DEFAULT_NUM_TRIES | MARISA_DEFAULT_TAIL | MARISA_DEFAULT_ORDER |MARISA_DEFAULT_CACHE);
        }
        catch (const marisa::Exception &ex)
        {
            std::cerr << ex.what() << ": failed to build a dictionary" << std::endl;
            return false;
        }
        vs_.resize(keyset.size());
        for (typename std::map<std::string, V>::const_iterator it =
                    m.begin(); it != m.end(); ++it)
        {
            const std::string key = it->first;
            if (set(key.c_str(), key.size(), it->second) != 0)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Get a value for a given key
     * @param str the key
     * @param len the length of str
     * @param v An associated value for a key
     * @return 0 on success and -1 if not found
     */
    int get(const char* str, size_t len, V& v) const
    {
        marisa::Agent agent;
        try
        {
            agent.set_query(str, len);
            if (trie_.lookup(agent))
            {
                std::cout << agent.key().id() << '\t' << str << '\n';
                v = vs_[agent.key().id()];
                return 0;
            }
            else
            {
                std::cout << "-1\t" << str << '\n';
                return -1;
            }
        }
        catch (const marisa::Exception &ex)
        {
            std::cerr << ex.what() << ": lookup() failed: " << str << std::endl;
            return -1;
        }
    }

    /**
     * Set a value for a given key
     * @param str the key
     * @param len the length of str
     * @param v  A value to be associated for a key
     * @return 0 on success and -1 if not found
     */
    int set(const char* str, size_t len, const V& v)
    {
        marisa::Agent agent;
        try
        {
            agent.set_query(str, len);
            if (trie_.lookup(agent))
            {
                //std::cout << agent.key().id() << '\t' << str << '\n';
                vs_[agent.key().id()] = v;
                return 0;
            }
            else
            {
                std::cout << "-1\t" << str << '\n';
                return -1;
            }
        }
        catch (const marisa::Exception &ex)
        {
            std::cerr << ex.what() << ": lookup() failed: " << str << std::endl;
            return -1;
        }
    }

    /**
     * Return the longest key that matches the prefix of the query in the dictionary
     * @param str the query
     * @param len the length of the query
     * @param retLen The length of the matched key in the dictionary
     * @param v The associated value for the key
     * @return 0 if found and -1 if not found
     */
    int prefixSearch(const char* str, size_t len, V& v) const
    {
        marisa::Agent agent;
        try
        {
            agent.set_query(str, len);
            if (trie_.predictive_search(agent))
            {
                std::cout << agent.key().id() << '\t' << str << '\n';
                v = vs_[agent.key().id()];
                return 0;
            }
            else
            {
                std::cout << "-1\t" << str << '\n';
                return -1;
            }
        }
        catch (const marisa::Exception &ex)
        {
            std::cerr << ex.what() << ": lookup() failed: " << str << std::endl;
            return -1;
        }
    }

    /**
     * Get the number of keys
     * @return the number of keys
     */
    size_t size() const
    {
        return trie_.size();
    }

private:
    Trie trie_;
    std::vector<V> vs_;
    size_t size_;
};


}

#endif
