//
// ccf::session_request - Cluster Communication Framework
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
#ifndef CCF_SESSION_REQUEST_H__
#define CCF_SESSION_REQUEST_H__

#include "ccf/types.h"
#include "ccf/address.h"
#include "ccf/retry.h"
#include "ccf/session_responder.h"

namespace ccf {


template <typename Parameter, typename Identifier = address>
struct session_request {
	typedef ccf::retry<Parameter> retry;

	session_request(const Identifier& from_, msgobj param_,
			session_responder response) :
		from(from_), m_response(response)
	{
		param_.convert(&param);
	}

	Parameter param;
	const Identifier from;

	template <typename Result>
	void result(Result res)
		{ m_response.result<Result>(res); }

	template <typename Result>
	void result(Result res, auto_zone& z)
		{ m_response.result<Result>(res, z); }

	template <typename Result>
	void result(Result res, shared_zone& life)
		{ m_response.result<Result>(res, life); }

	void result_null()
		{ m_response.null(); }

	template <typename Error>
	void result_error(Error err)
		{ m_response.error<Error>(err); }

	template <typename Error>
	void result_error(Error err, auto_zone& z)
		{ m_response.error<Error>(err, z); }

	template <typename Error>
	void result_error(Error err, shared_zone& life)
		{ m_response.error<Error>(err, life); }

private:
	session_responder m_response;
	session_request();
};


}  // namespace ccf

#endif /* ccf/session_request.h */

