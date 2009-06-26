#ifndef SERVER_STUB_H__
#define SERVER_STUB_H__

#include <ccf/session_request.h>
#include <ccf/server.h>
#include <string>
#include "server/proto.h"

#define SVR_IMPL(NAME, req, zone) \
	void svr_##NAME(ccf::session_request<NAME/*, ccf::address*/> req, \
			ccf::auto_zone& zone)

namespace server {

%self.each do |msg|
%next unless msg.id
SVR_IMPL([%msg.name%], req, zone);
%end

static inline void dispatch(ccf::method_t method, ccf::msgobj param,
		ccf::session_responder response,
		/*const ccf::address& from, */ccf::auto_zone& z)
{
	switch(method) {
	%self.each do |msg|
	%next unless msg.id
	case [%msg.name%]::method:
		svr_[%msg.name%](ccf::session_request<[%msg.name%]/*, ccf::address*/>(
				param, response/*, from*/), z);
		break;
	%end
	default:
		throw std::runtime_error("unknown method");  // FIXME
	}
}


}  // namespace server

#endif /* server/stub.h */
%# vim: syntax=mplex
