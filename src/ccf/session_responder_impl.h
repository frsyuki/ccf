//
// ccf::session_responder - Cluster Communication Framework
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
#ifndef CCF_SESSION_RESPONDER_IMPL_H__
#define CCF_SESSION_RESPONDER_IMPL_H__

#include <mp/object_callback.h>
#include <cclog/cclog.h>
#include <msgpack/vrefbuffer.hpp>
#include "ccf/protocol.h"
#include "ccf/session.h"

namespace ccf {


inline session_responder::session_responder(weak_session s, msgid_t msgid) :
	m_session(s), m_msgid(msgid) { }

inline session_responder::~session_responder() { }


template <typename Result, typename Error, typename ZoneType>
void session_responder::call_impl(Result& res, Error& err, ZoneType& life)
{
	std::auto_ptr<msgpack::vrefbuffer> buf(new msgpack::vrefbuffer());

	message_response<Result&, Error> msg(res, err, m_msgid);
	msgpack::pack(*buf, msg);

	shared_session s(m_session.lock());
	if(!s) {
		//throw std::runtime_error("lost session");
		return;  // Note: ignore error
	}

	// FIXME auto_zone
	s->send_datav(buf, life);
}

template <typename Result, typename Error>
void session_responder::call(Result& res, Error& err)
{
	msgpack::sbuffer buf;
	message_response<Result, Error> msg(res, err, m_msgid);
	msgpack::pack(buf, msg);

	shared_session s(m_session.lock());
	if(!s) {
		//throw std::runtime_error("lost session");
		return;  // Note: ignore error
	}

	shared_zone nullz;
	s->send_data(buf, nullz);
}

template <typename Result, typename Error>
inline void session_responder::call(Result& res, Error& err, auto_zone& z)
{
	call_impl<Result, Error>(res, err, z);
}

template <typename Result, typename Error>
inline void session_responder::call(Result& res, Error& err, shared_zone& z)
{
	call_impl<Result, Error>(res, err, z);
}


template <typename Result>
inline void session_responder::result(Result res)
{
	LOG_TRACE("send response data with Success id=",m_msgid);
	msgpack::type::nil err;
	call(res, err);
}

template <typename Result>
inline void session_responder::result(Result res, auto_zone& z)
{
	LOG_TRACE("send response data with Success id=",m_msgid);
	msgpack::type::nil err;
#ifndef NO_RESPONSE_ZERO_COPY
	call(res, err, z);
#else
	call(res, err);
#endif
}

template <typename Result>
inline void session_responder::result(Result res, shared_zone& life)
{
	LOG_TRACE("send response data with Success id=",m_msgid);
	msgpack::type::nil err;
#ifndef NO_RESPONSE_ZERO_COPY
	call(res, err, life);
#else
	call(res, err);
#endif
}

template <typename Error>
inline void session_responder::error(Error err)
{
	LOG_TRACE("send response data with Error id=",m_msgid);
	msgpack::type::nil res;
	call(res, err);
}

template <typename Error>
inline void session_responder::error(Error err, auto_zone& z)
{
	LOG_TRACE("send response data with Error id=",m_msgid);
	msgpack::type::nil res;
	call(res, err, z);
}

template <typename Error>
inline void session_responder::error(Error err, shared_zone& life)
{
	LOG_TRACE("send response data with Error id=",m_msgid);
	msgpack::type::nil res;
#ifndef NO_RESPONSE_ZERO_COPY
	call(res, err, life);
#else
	call(res, err);
#endif
}

inline void session_responder::null()
{
	LOG_TRACE("send response data with null id=",m_msgid);
	msgpack::type::nil res;
	msgpack::type::nil err;
	call(res, err);
}


}  // namespace rpc

#endif /* ccf/session_responder_impl.h */

