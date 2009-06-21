#
# ccf protogen - Cluster Communication Framework
#
# Copyright (C) 2008-2009 FURUHASHI Sadayuki
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
require 'phraser'

module CCF
module Protogen

extend Phraser

s = Phraser::Scanner.new

s.token :@message
s.token :@end
s.token :optkey,    /\:[a-zA-Z0-9_]+/
s.token :num,       /[0-9]+/
s.token :sym,       /[a-zA-Z_][a-zA-Z_0-9\:\<\>]*/
s.token :qqstr,     /\"[^\"]*\"/
s.token :qstr,      /\'[^\']*\'/
s.token "//"
s.token "="
s.token ";"
s.token ","
s.token :blank,   /\#[^\r\n]*|[ \t\r\n]+/

Scanner = s

class Member
	def initialize(type, name, default = nil)
		@type = type
		@name = name
		@default = default
	end
	attr_reader :type, :name, :default
end

class Message
	def initialize(id, name, body, attr = nil)
		@id   = id
		@name = name
		@body = body
		@attr = attr
	end
	attr_reader :id, :name, :body, :attr

	def required
		@body.select {|b| b.default == nil }
	end

	def optional
		@body.select {|b| b.default != nil }
	end

	def method_missing(*args, &block)
		@body.__send__(*args, &block)
	end
end

RLiteral = rule do
	token(:sym) / token(:num) / token(:qqstr) / token(:qstr)
end

RMemberRequired = rule do
	(token(:sym)[:type] ^ token(:sym)[:name] ^ token(";")).action {|x,e|
		Member.new(e[:type], e[:name])
	}
end

RMemberOptional = rule do
	(token(:sym)[:type] ^ token(:sym)[:name] ^
			token("=") ^ RLiteral[:default] ^
			token(";")).action {|x,e|
		Member.new(e[:type], e[:name], e[:default])
	}
end

RBody = rule do
	(RMemberRequired.*[:x] ^ RMemberOptional.*[:xs]).action {|x,e|
		e[:x] + e[:xs]
	}
end

RAttr = rule do
	(token(:optkey)[:key] ^
			((token("=") ^ (token(:sym) / token(:num))[:value])).opt
	).action {|x,e|
		[e[:key], e[:value]]
	}
end

RAttributes = rule do
	(RAttr.*).action {|x,e|
		kv = {}
		x.each {|k,v| kv[k] = v }
		kv
	}
end


$ids = {}

RMessage = rule do
	(token(:@message) ^ token(:sym)[:name] ^ RAttributes.opt[:attr] ^
			RBody[:body] ^ token(:@end) ^
			(token("=") ^ token(:num)[:id]).opt
	).action {|x,e|
		eid = e[:id]
		ename = e[:name]
		if eid
			eid = eid.to_i
			if id = $ids[ename]
				if eid != id
					raise "id for #{ename} not match"
				end
			else
				$ids.each {|n, i|
					if i == eid
						raise "id for #{ename} is already used for #{n}"
					end
				}
			end
		end
		$ids[ename] = eid
		Message.new(eid, ename, e[:body], e[:attr])
	}
end

RMessageID = rule do
	(token(:@message) ^ token(:@sym)[:name] ^
			token("=") ^ token(:num)[:id] ^ token(";")).action {|x,e|
		eid = e[:id].to_i
		ename = e[:name]
		if id = $ids[ename]
			if eid != id
				raise "id for #{ename} not match"
			end
		else
			$ids.each {|n, i|
				if i == eid
					raise "id for #{ename} is already used for #{n}"
				end
			}
		end
		$ids[ename] = eid
	}
end

RProtocol = rule do
	(RMessageID / RMessage[:a]).action {|x,e| e[:a] }.*
end

Rule = rule do
	(RProtocol[:a] ^ eof).action {|x,e| e[:a] }
end


def self.parse(src)
	lx = Scanner.scan(src).delete_if {|r| r.token == :blank }
	Phraser::parse(Rule, lx)
end


end  # module Protogen
end  # module CCF

