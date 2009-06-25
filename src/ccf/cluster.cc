//
// ccf::cluster - Cluster Communication Framework
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
#include "ccf/cluster.h"

namespace ccf {


cluster::connection::connection(int fd, cluster* manager) :
	managed_state_connection<connection>(fd, manager)
{ }

cluster::connection::connection(int fd, cluster* manager, shared_session session) :
	managed_state_connection<connection>(fd, manager, session)
{
	set_process(&cluster::connection::cluster_state);
}

cluster* cluster::connection::get_manager()
{
	// managed_connection::m_manager
	return static_cast<cluster*>(m_manager);
}

//void cluster::connection::process_init(msgobj msg, auto_zone& z)
//{
//	// FIXME
//  get_manager()->bind_session(id, addr, this);
//	set_process(&cluster::connection::cluster_state);
//  get_manager()->subsystem().bind_session(id, addr, this);
//	set_process(&cluster::connection::subsys_state);
//}

void cluster::connection::cluster_state(msgobj msg, auto_zone z)
{
//	LOG_TRACE("receive rpc message: ",msg);
	rpc_message rpc(msg.convert());

	switch(rpc.type()) {
	case message_code::REQUEST: {
			message_request<msgobj> msgreq(msg.convert());
			responder response(fd(), msgreq.msgid());
			get_manager()->cluster_dispatch_request(
					mp::static_pointer_cast<node>(m_session),
					msgreq.method(), msgreq.param(), response, z);
		}
		break;

	case message_code::RESPONSE: {
			message_response<msgobj, msgobj> msgres(msg.convert());
			process_response(  // managed_connection::process_response
					msgres.result(), msgres.error(), msgres.msgid(), z);
		}
		break;

	default:
		throw msgpack::type_error();
	}
}

void cluster::connection::subsys_state(msgobj msg, auto_zone z)
{
//	LOG_TRACE("receive rpc message: ",msg);
	rpc_message rpc(msg.convert());

	switch(rpc.type()) {
	case message_code::REQUEST: {
			message_request<msgobj> msgreq(msg.convert());
			responder response(fd(), msgreq.msgid());
			get_manager()->subsys_dispatch_request(
					mp::static_pointer_cast<peer>(m_session),
					msgreq.method(), msgreq.param(), response, z);
		}
		break;

	case message_code::RESPONSE: {
			message_response<msgobj, msgobj> msgres(msg.convert());
			process_response(  // managed_connection::process_response
					msgres.result(), msgres.error(), msgres.msgid(), z);
		}
		break;

	default:
		throw msgpack::type_error();
	}
}


}  // namespace ccf

