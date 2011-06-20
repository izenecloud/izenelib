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
	template <typename A1>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	future call(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	future call(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1>
	future call(const std::string& name,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2>
	future call(const std::string& name,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	future call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_request(
				name, params, slife);
	}
	template <typename ArgArray>
	future call_apply(const std::string& name,
			auto_zone msglife,
			const ArgArray& params)
	{
		shared_zone  slife(msglife.release());
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
	template <typename A1>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	void notify(const std::string& name,
			auto_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone  slife(msglife.release());
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	void notify(const std::string& name,
			shared_zone msglife,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone& slife = msglife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1>
	void notify(const std::string& name,
			const A1& a1)
	{
		type::tuple<const A1&> params(a1);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2>
	void notify(const std::string& name,
			const A1& a1, const A2& a2)
	{
		type::tuple<const A1&, const A2&> params(a1, a2);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3)
	{
		type::tuple<const A1&, const A2&, const A3&> params(a1, a2, a3);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&> params(a1, a2, a3, a4);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&> params(a1, a2, a3, a4, a5);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&> params(a1, a2, a3, a4, a5, a6);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&> params(a1, a2, a3, a4, a5, a6, a7);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&> params(a1, a2, a3, a4, a5, a6, a7, a8);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13, const A14& a14, const A15& a15, const A16& a16)
	{
		type::tuple<const A1&, const A2&, const A3&, const A4&, const A5&, const A6&, const A7&, const A8&, const A9&, const A10&, const A11&, const A12&, const A13&, const A14&, const A15&, const A16&> params(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
		shared_zone  slife;
		return static_cast<IMPL*>(this)->send_notify(
				name, params, slife);
	}
	template <typename ArgArray>
	void notify_apply(const std::string& name,
			auto_zone msglife,
			const ArgArray& params)
	{
		shared_zone  slife(msglife.release());
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

