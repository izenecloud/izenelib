// Copyright 2010, Takuya Akiba
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Takuya Akiba nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef IZENELIB_AM_SUCCINCT_MINIMAL_PERFECT_HASH_HPP
#define IZENELIB_AM_SUCCINCT_MINIMAL_PERFECT_HASH_HPP

#include <types.h>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace boost
{
namespace serialization
{
class access;
}
}

NS_IZENELIB_AM_BEGIN

namespace succinct{

template<typename Key> class DefaultSeedHash;
template<typename Key, typename SeedHash> class MinimalPerfectHash;

template<typename Key, typename SeedHash = DefaultSeedHash<Key> >
class PerfectHash
{
public:
    PerfectHash() : num_v_(0) {}

    int Build(const std::vector<Key> &keys);

    inline int GetHash(const Key &key) const
    {
        int v[3];
        v[0] = SeedHash::GetHash(seeds_[0], num_v_, key) % num_v_;
        v[1] = SeedHash::GetHash(seeds_[1], num_v_, key) % num_v_ + num_v_;
        v[2] = SeedHash::GetHash(seeds_[2], num_v_, key) % num_v_ + num_v_ * 2;
        return v[(GetG(v[0]) + GetG(v[1]) + GetG(v[2])) % 3];
    }

    inline int GetRange() const
    {
        return num_v_ * 3;
    }
private:
    friend class MinimalPerfectHash<Key, SeedHash>;
    friend class boost::serialization::access;

    static constexpr int kNumTrial = 100;
    static constexpr double kScale = 1.3;

    // Number of the vertices of each block in the hyper graph
    int num_v_;

    // Generated set of the seeds
    uint32_t seeds_[3];

    // G value of each vertices
    std::vector<uint8_t> g_;

    inline int GetG(int i) const
    {
        return 3 & (g_[i / 4] >> ((i % 4) * 2));
    }

    template<class Archive>
    void serialize(Archive& ar, unsigned int ver __attribute__((unused)))
    {
        ar & num_v_;
        for (int j = 0; j < 3; ++j)
        {
            ar & seeds_[j];
        }
        ar & g_;
    }
};

template<typename Key, typename SeedHash = DefaultSeedHash<Key> >
class MinimalPerfectHash
{
public:
    int Build(const std::vector<Key> &keys);

    inline int GetHash(const Key &key) const
    {
        int i = phf_.GetHash(key);
        return exists_acm256_[i / 256] + exists_acm32_[i / 32]
               + __builtin_popcount(exists_[i / 32] & ((uint32_t(1) << (i % 32)) - 1));
    }
    // |__builtin_popcount| may cause problems
    // if |unsigned int| has less than 32 bits (lol)

    inline int GetRange() const
    {
        return range_;
    }
private:
    friend class boost::serialization::access;

    PerfectHash<Key, SeedHash> phf_;
    int range_;

    std::vector<uint32_t> exists_;
    std::vector<uint32_t> exists_acm256_;
    std::vector<uint8_t> exists_acm32_;

    template<class Archive>
    void serialize(Archive& ar, unsigned int ver __attribute__((unused)))
    {
        ar & phf_;
        ar & range_;
        ar & exists_;
        ar & exists_acm256_;
        ar & exists_acm32_;
    }
};

template<typename Key, typename SeedHash>
int PerfectHash<Key, SeedHash>::Build(const std::vector<Key> &keys)
{
    num_v_ = std::max(5, (int)ceil(keys.size() * kScale / 3));
    int num_3v = num_v_ * 3;

    for (int t = 0; t < kNumTrial; ++t)
    {
        // Generate a candidate set of the seeds
        for (int i = 0; i < 3; ++i)
        {
            seeds_[i] = rand();
        }

        // Generate the edges
        std::vector<std::vector<int> > edges(keys.size());
        for (size_t i = 0; i < keys.size(); ++i)
        {
            std::vector<int> &e = edges[i];
            e.resize(3);
            for (int j = 0; j < 3; ++j)
            {
                uint32_t h = SeedHash::GetHash(seeds_[j], num_v_, keys[i]);
                e[j] = h % num_v_ + num_v_ * j;
            }
        }

        // Construct the adjacency list
        std::vector<std::vector<int> > adj(num_3v);
        for (size_t i = 0; i < edges.size(); ++i)
        {
            const std::vector<int> &e = edges[i];
            for (int j = 0; j < 3; ++j)
            {
                adj[e[j]].push_back(i);
            }
        }

        // Prepare for BFS
        std::vector<int> deg(num_3v);
        std::queue<int> que;
        for (int i = 0; i < num_3v; ++i)
        {
            deg[i] = adj[i].size();
            if (deg[i] == 1)
            {
                que.push(i);
            }
        }

        // BFS
        std::vector<bool> edge_del(edges.size());
        std::vector<int> edge_ord;
        std::vector<int> edge_to_vertex(edges.size());
        while (!que.empty())
        {
            int v = que.front();
            que.pop();

            // Find the last remaining edge connected to |v|
            int eid = -1;
            for (size_t i = 0; i < adj[v].size(); ++i)
            {
                if (!edge_del[adj[v][i]])
                {
                    eid = adj[v][i];
                }
            }
            if (eid == -1)
            {
                continue;
            }
            edge_del[eid] = true;
            edge_ord.push_back(eid);
            edge_to_vertex[eid] = v;

            // Decrease |deg| and enque vertices
            for (int j = 0; j < 3; ++j)
            {
                int w = edges[eid][j];
                if (--deg[w] == 1)
                {
                    que.push(w);
                }
            }
        }

        // Failed, i.e. the graph has cycles
        if (edge_ord.size() != edges.size())
        {
            continue;
        }

        // Compute |g|
        reverse(edge_ord.begin(), edge_ord.end());
        std::vector<bool> vertex_vis(num_3v);
        std::vector<int> tg(num_3v, 3);
        for (size_t i = 0; i < edge_ord.size(); ++i)
        {
            int eid = edge_ord[i];
            const std::vector<int> &e = edges[eid];
            int v = edge_to_vertex[eid];

            tg[v] = 0;
            for (int j = 0; j < 3; ++j)
            {
                if (e[j] == v)
                {
                    tg[v] += j;
                }
                else if (vertex_vis[e[j]])
                {
                    tg[v] += 3 - tg[e[j]];
                }
            }
            tg[v] %= 3;
            vertex_vis[v] = true;
        }

        g_.resize((num_3v + 3) / 4);
        fill(g_.begin(), g_.end(), 0);
        for (int i = 0; i < num_3v; ++i)
        {
            g_[i / 4] |= tg[i] << ((i % 4) * 2);
        }
        return 0;
    }

    // Failed
    num_v_ = 0;
    return -1;
}

template<typename Key, typename SeedHash>
int MinimalPerfectHash<Key, SeedHash>::Build(const std::vector<Key> &keys)
{
    if (phf_.Build(keys) != 0)
    {
        range_ = 0;
        return -1;
    }

    int num_3v = phf_.num_v_ * 3;
    exists_.resize((num_3v + 32 - 1) / 32);
    exists_acm256_.resize((num_3v + 256 - 1) / 256);
    exists_acm32_.resize((num_3v + 32 - 1) / 32);
    fill(exists_.begin(), exists_.end(), 0);
    fill(exists_acm256_.begin(), exists_acm256_.end(), 0);
    fill(exists_acm32_.begin(), exists_acm32_.end(), 0);

    int rnk = 0;
    for (int i = 0; i < num_3v; ++i)
    {
        if (i % 256 == 0)
        {
            exists_acm256_[i / 256] = rnk;
        }
        if (i % 32 == 0)
        {
            exists_acm32_[i / 32] = rnk - exists_acm256_[i / 256];
        }
        int g = phf_.GetG(i);
        if (g != 3)
        {
            exists_[i / 32] |= uint32_t(1) << (i % 32);
            ++rnk;
        }
    }
    range_ = rnk;
    return 0;
}

// |PerfectHash| and |MinimalPerfectHash| require hash functions which receives
// seeds and behaves differently depending on the seeds.
//
// The following classes are such hash functions and
// |PerfectHash| and |MinimalPerfectHash| use them by default.
//
// I wrote them just to pass the tests
// and don't know these hash functions are good or bad.

template<typename T> class DefaultSeedHash
{
public:
    inline static uint32_t GetHash(uint32_t seed, uint32_t lim, const T &key)
    {
        return (uint64_t(key) * 1350490027 + 123456789012345ULL) % seed % lim;
    }
};

template<typename T, typename U> class DefaultSeedHash<std::pair<T, U> >
{
public:
    inline static uint32_t GetHash(uint32_t seed, uint32_t lim,
                                   const std::pair<T, U> &key)
    {
        uint32_t f = DefaultSeedHash<T>::GetHash(seed * 2044897763 + 12345, lim, key.first);
        uint32_t s = DefaultSeedHash<U>::GetHash(seed * 1120048829 + 67890, lim, key.second);
        return (f * uint64_t(101) + s) % seed % lim;
    }
};

template<> class DefaultSeedHash<std::string>
{
public:
    inline static uint32_t GetHash(uint32_t seed, uint32_t lim,
                                   const std::string &key)
    {
        uint32_t h = 1111111111, s = seed;
        for (size_t i = 0; i < key.length(); ++i)
        {
            s = s * 1504569917 + 987987987;
            h = (h * uint64_t(103) % seed)
                + DefaultSeedHash<char>::GetHash(s, lim, key[i]);
        }
        return h % seed % lim;
    }
};

template<typename T> class DefaultSeedHash<std::vector<T> >
{
public:
    inline static uint32_t GetHash(uint32_t seed, uint32_t lim,
                                   const std::vector<T> &key)
    {
        uint32_t h = 1010101010, s = seed;
        for (size_t i = 0; i < key.size(); ++i)
        {
            s = s * 1717226057 + 123123123;
            h = (h * uint64_t(107) % seed)
                + DefaultSeedHash<T>::GetHash(s, lim, key[i]);
        }
        return h % seed % lim;
    }
};

}
NS_IZENELIB_AM_END

#endif  // IZENELIB_AM_SUCCINCT_MINIMAL_PERFECT_HASH_HPP
