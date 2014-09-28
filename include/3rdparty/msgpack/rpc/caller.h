//
// msgpack::rpc::caller - MessagePack-RPC for C++
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
#ifndef MSGPACK_RPC_CALLER_H__
#define MSGPACK_RPC_CALLER_H__

#include "types.h"
#include "future.h"
#include <string>

namespace msgpack {
namespace rpc {


class session;


template <typename IMPL>
class caller {
public:
    caller() { }
    ~caller() { }



    future call(const std::string& name)
    {
        type::tuple<> params;
        return static_cast<IMPL*>(this)->send_request(
                name, params, shared_zone());
    }

    template <typename... Types>
    future call(const std::string& name, auto_zone msglife, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone slife;
        shared_zone_helper::reset_shared_zone(slife, msglife.release());
        return static_cast<IMPL*>(this)->send_request(name, params, slife);
    }

    template <typename... Types>
    future call(const std::string& name, shared_zone msglife, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone& slife = msglife;
        return static_cast<IMPL*>(this)->send_request(name, params, slife);
    }

    template <typename... Types>
    future call(const std::string& name, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone slife;
        return static_cast<IMPL*>(this)->send_request(name, params, slife);
    }

    template <typename ArgArray>
    future call_apply(const std::string& name,
            auto_zone msglife,
            const ArgArray& params)
    {
        shared_zone  slife;
        shared_zone_helper::reset_shared_zone(slife, msglife.release());
        return static_cast<IMPL*>(this)->send_request(
                name, params, slife);
    }
    template <typename ArgArray>
    future call_apply(const std::string& name,
            shared_zone msglife,
            const ArgArray& params)
    {
        shared_zone& slife = msglife;
        return static_cast<IMPL*>(this)->send_request(
                name, params, slife);
    }
    template <typename ArgArray>
    future call_apply(const std::string& name,
            const ArgArray& params)
    {
        shared_zone  slife;
        return static_cast<IMPL*>(this)->send_request(
                name, params, slife);
    }

    void notify(const std::string& name)
    {
        type::tuple<> params;
        return static_cast<IMPL*>(this)->send_notify(
                name, params, shared_zone());
    }

    template <typename... Types>
    void notify(const std::string& name, auto_zone msglife, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone slife;
        shared_zone_helper::reset_shared_zone(slife, msglife.release());
        return static_cast<IMPL*>(this)->send_notify(name, params, slife);
    }

    template <typename... Types>
    void notify(const std::string& name, shared_zone msglife, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone& slife = msglife;
        return static_cast<IMPL*>(this)->send_notify(name, params, slife);
    }

    template <typename... Types>
    void notify(const std::string& name, const Types&... args)
    {
        type::tuple<const Types&...> params(args...);
        shared_zone slife;
        return static_cast<IMPL*>(this)->send_notify(name, params, slife);
    }

    template <typename ArgArray>
    void notify_apply(const std::string& name,
            auto_zone msglife,
            const ArgArray& params)
    {
        shared_zone  slife;
        shared_zone_helper::reset_shared_zone(slife, msglife.release());
        return static_cast<IMPL*>(this)->send_notify(
                name, params, slife);
    }
    template <typename ArgArray>
    void notify_apply(const std::string& name,
            shared_zone msglife,
            const ArgArray& params)
    {
        shared_zone& slife = msglife;
        return static_cast<IMPL*>(this)->send_notify(
                name, params, slife);
    }
    template <typename ArgArray>
    void notify_apply(const std::string& name,
            const ArgArray& params)
    {
        shared_zone  slife;
        return static_cast<IMPL*>(this)->send_notify(
                name, params, slife);
    }
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/caller.h */
