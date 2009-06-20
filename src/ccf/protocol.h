//
// ccf::protocol - Cluster Communication Framework
//
// Copyright (C) 2009 FURUHASHI Sadayuki
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
#ifndef CCF_PROTOCOL_H__
#define CCF_PROTOCOL_H__

#include <msgpack.hpp>
#include "ccf/types.h"

namespace ccf {


typedef uint8_t rpc_type_t;

namespace message_code {
	static const rpc_type_t REQUEST  = 0;
	static const rpc_type_t RESPONSE = 1;
	static const rpc_type_t INIT     = 2;
}  // namespace message_code

namespace error_code {
	// FIXME HTTP-like error code?
	//static const int INTERNAL_SERVER_ERROR = 501;
	//static const int TYPE_ERROR            = 502;
	//static const int TIMEOUT_ERROR         = 504;

	static const int TRANSPORT_LOST_ERROR = 1;
	static const int NODE_LOST_ERROR      = 2;
	static const int TIMEOUT_ERROR        = 3;
	static const int UNKNOWN_ERROR        = 4;
	static const int PROTOCOL_ERROR       = 5;
	static const int SERVER_ERROR         = 6;
}


struct rpc_message : msgpack::define< msgpack::type::tuple<rpc_type_t> > {
	rpc_message() { }
	rpc_type_t type()  const { return get<0>(); }
	bool is_request()  const { return type() == message_code::REQUEST;  }
	bool is_response() const { return type() == message_code::RESPONSE; }
	bool is_init()     const { return type() == message_code::INIT;     }
};


template <typename Parameter>
struct message_request : msgpack::define<
			msgpack::type::tuple<rpc_type_t, msgid_t, method_t, Parameter> > {

	typedef message_request<Parameter> this_t;

	message_request() { }

	message_request(
			method_t method,
			typename msgpack::type::tuple_type<Parameter>::transparent_reference param,
			msgid_t msgid) :
		this_t::define_type(typename this_t::msgpack_type(
					message_code::REQUEST,
					msgid,
					method,
					param
					)) { }

	msgid_t  msgid()  const { return this_t::msgpack_type::template get<1>(); }

	method_t method() const { return this_t::msgpack_type::template get<2>(); }

	typename msgpack::type::tuple_type<Parameter>::const_reference
	param() const { return this_t::msgpack_type::template get<3>(); }
};


template <typename Result, typename Error>
struct message_response : msgpack::define<
			msgpack::type::tuple<rpc_type_t, msgid_t, Error, Result> > {

	typedef message_response<Result, Error> this_t;

	message_response() { }

	message_response(
			typename msgpack::type::tuple_type<Result>::transparent_reference res,
			typename msgpack::type::tuple_type<Error >::transparent_reference err,
			msgid_t msgid) :
		this_t::define_type(typename this_t::msgpack_type(
					message_code::RESPONSE,
					msgid,
					err,
					res
					)) { }

	msgid_t msgid()  const { return this_t::msgpack_type::template get<1>(); }

	typename msgpack::type::tuple_type<Error>::const_reference
	error()  const { return this_t::msgpack_type::template get<2>(); }

	typename msgpack::type::tuple_type<Result>::const_reference
	result() const { return this_t::msgpack_type::template get<3>(); }
};


}  // namespace ccf

#endif /* ccf/protocol.h */

