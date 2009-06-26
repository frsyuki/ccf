//
// ccf::rpc_connection - Cluster Communication Framework
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
#ifndef CCF_RPC_CONNECTION_H__
#define CCF_RPC_CONNECTION_H__

#include "ccf/connection.h"

#include "cclog/cclog.h"
#include "ccf/types.h"
#include "ccf/protocol.h"
#include "ccf/responder.h"
#include "ccf/service.h"
// FIXME #include "rpc/stream.h"
#include <msgpack.hpp>
#include <stdexcept>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace ccf {


template <typename IMPL>
class rpc_connection : public connection<IMPL> {
public:
	rpc_connection(int fd);
	~rpc_connection();

public:
	// from connection<IMPL>
	void process_message(msgobj msg, auto_zone z);

	void process_request(method_t method, msgobj param, msgid_t msgid, auto_zone z);

	void dispatch_request(method_t method, msgobj param, responder& response, auto_zone z);

	void process_response(msgobj result, msgobj error, msgid_t msgid, auto_zone z);

	void process_stream(msgobj method, msgobj param, msgid_t msgid, auto_zone z);

protected:
	msgpack::unpacker m_pac;
	bool m_stream_mode;

private:
	rpc_connection();
	rpc_connection(const rpc_connection&);
};


template <typename IMPL>
rpc_connection<IMPL>::rpc_connection(int fd) :
	connection<IMPL>(fd), m_stream_mode(false) { }

template <typename IMPL>
rpc_connection<IMPL>::~rpc_connection() { }


template <typename IMPL>
inline void rpc_connection<IMPL>::dispatch_request(method_t method, msgobj param,
		responder& response, auto_zone z)
{
	throw msgpack::type_error();
}

template <typename IMPL>
inline void rpc_connection<IMPL>::process_request(method_t method, msgobj param,
		msgid_t msgid, auto_zone z)
{
	responder response(connection<IMPL>::fd(), msgid);
	static_cast<IMPL*>(this)->dispatch_request(method, param, response, z);
}

template <typename IMPL>
inline void rpc_connection<IMPL>::process_response(msgobj result, msgobj error,
		msgid_t msgid, auto_zone z)
{
	throw msgpack::type_error();
}

template <typename IMPL>
void rpc_connection<IMPL>::process_message(msgobj msg, auto_zone z)
{
	rpc_message rpc; msg.convert(&rpc);

	switch(rpc.type()) {
	case message_code::REQUEST: {
			message_request<msgobj> req; msg.convert(&req);
			static_cast<IMPL*>(this)->process_request(
					req.method(), req.param(), req.msgid(), z);
		}
		break;

	case message_code::RESPONSE: {
			message_response<msgobj, msgobj> res; msg.convert(&res);
			static_cast<IMPL*>(this)->process_response(
					res.result(), res.error(), res.msgid(), z);
		}
		break;

	default:
		throw msgpack::type_error();
	}
}


}  // namespace ccf

#endif /* ccf/rpc_connection.h */

