%def genstub
%self.each do |msg|
%next unless msg.id
void svr_[%msg.name%]([%msg.name%] param, ccf::session_responder response,
		ccf::shared_session from, ccf::auto_zone& z);

%end

void dispatch(ccf::shared_session from,
		ccf::method_t method, ccf::msgobj param,
		ccf::session_responder response, ccf::auto_zone& z)
{
	switch(method) {
	%self.each do |msg|
	%next unless msg.id
	case [%msg.name%]::method:
		svr_[%msg.name%](param.as<[%msg.name%]>(), response, from, z);
		break;
	%end
	default:
		throw std::runtime_error("unknown method");  // FIXME
	}
}
%end
%# vim: syntax=mplex
