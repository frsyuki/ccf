//
// ccf::responder - Cluster Communication Framework
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
#ifndef CCF_RESPONDER_H__
#define CCF_RESPONDER_H__

#include "ccf/types.h"

namespace ccf {


class responder {
public:
	responder(int fd, msgid_t msgid);

	~responder();

	template <typename Result>
	void result(Result res);

	template <typename Result>
	void result(Result res, auto_zone z);

	template <typename Error>
	void error(Error err);

	template <typename Error>
	void error(Error err, auto_zone z);

	void null();

private:
	template <typename Result, typename Error>
	void call(Result& res, Error& err);

	template <typename Result, typename Error>
	void call(Result& res, Error& err, auto_zone z);

private:
	int m_fd;
	msgid_t m_msgid;

private:
	responder();
};


}  // namespace ccf

#include "ccf/responder_impl.h"

#endif /* ccf/responder.h */

