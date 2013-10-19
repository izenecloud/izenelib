//
// msgpack::rpc::types - Cluster Communication Framework
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_RPC_TYPES_H__
#define MSGPACK_RPC_TYPES_H__

#include "../msgpack.hpp"
#include "../mp/memory.h"
#include "../msgpack/zone.hpp"

namespace msgpack {
namespace rpc {


typedef mp::shared_ptr<msgpack_zone> shared_zone;

class shared_zone_helper
{
public:
    static shared_zone create_shared_zone()
    {
        shared_zone p(msgpack_zone_new(MSGPACK_ZONE_CHUNK_SIZE), &msgpack_zone_free);
        return p;
    }
    static void reset_shared_zone(shared_zone& orig, msgpack_zone* data)
    {
        orig.reset(data, &msgpack_zone_free);
    }
    static void reset_shared_zone(shared_zone& orig, zone* data)
    {
        orig.reset(data, &msgpack_zone_free);
    }
};

template <typename T>
class with_shared_zone : public T {
public:
	with_shared_zone(shared_zone life) : m_life(life) { }
	with_shared_zone(const T& c, shared_zone life) : T(c), m_life(life) { }
	~with_shared_zone() { }
private:
	shared_zone m_life;
private:
	with_shared_zone();
	with_shared_zone(const with_shared_zone&);
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/types.h */

