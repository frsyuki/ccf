#
# Phraser: Simple lexer and parser combinator
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

module Phraser


class Scanner
	class Token
		def initialize(name, expr)
			@name = name
			@expr = expr
		end
		attr_reader :name, :expr
	end

	def initialize
		@tokens = []
	end

	def token(name, expr = nil)
		name = name.to_sym
		expr = /#{Regexp.escape(name.to_s)}/ unless expr
		@tokens.push Token.new(name, expr)
	end

	def scan(src)
		require 'strscan'
	
		result = []
	
		s = StringScanner.new(src)
		until s.empty?
			err = @tokens.each {|t|
				if m = s.scan(t.expr)
					(class<<m; self; end).instance_eval {
						define_method(:token) { t.name }
						define_method(:inspect) { "\"#{m}\":#{t.name}" }
					}
					result.push(m)
					break nil
				end
			}
			raise "error '#{s.peek(10)}'" if err
		end
	
		result
	end
end


class Parser
	class ParseError < StandardError; end

	def initialize(&block)
		@block = block
		@action = Proc.new {|x,e| x }
		@name = nil
	end

	def action(&block)
		@action = block
		self
	end

	def [](name)
		@name = name
		self
	end

	def parse(i, e = {})
		r = @block[i, e]
		r[0] = @action.call(r[0], e)
		e[@name] = r[0] if @name
		r
	end

	def /(o)
		parser {|i,e|
			begin
				parse(i,e)
			rescue ParseError
				o.parse(i,e)
			end
		}
	end

	def ^(o)
		parser {|i,e|
			r1 = parse(i,e)
			r2 = o.parse(r1[1],e)
			[[r1[0], r2[0]], r2[1]]
		}
	end

	def apply(&rule)
		parser {|i,e|
			r = parse(i,e)
			[rule.call(r[0]), r[1]]
		}
	end

	def opt
		self / parser {|i,e| [nil, i] }
	end

	def *
		parser {|i,e|
			rs = []
			while true
				begin
					r = parse(i,e)
				rescue ParseError
					break
				end
				rs << r[0]
				i = r[1]
			end
			[rs, i]
		}
	end

	def +
		(self ^ self.*).apply {|i| i[1].unshift(i[0]) }
	end

	def not
		parser {|i,e|
			begin
				r = parse(i,e)
			rescue ParseError
			end
			raise ParseError, "not error: #{i.inpsect}" if r
			[nil, i]
		}
	end

	def and
		self.not.not
	end

	class Context
		def token(t)
			parser {|i,e|
				unless i.first && i.first.token == t.to_sym
					raise ParseError, "token error: #{i.inspect}"
				end
				[i.first, i[1..-1]]
			}
		end
	
		def any
			parser {|i,e|
				unless i.length > 0
					raise ParseError, "any error: #{i.inspect}"
				end
				[i.first, i[1..-1]]
			}
		end

		def eof
			parser {|i,e|
				unless i.empty?
					raise ParseError, "EOF error: #{i.inspect}"
				end
				[nil, i]
			}
		end

		def parser(&block)
			Parser.new(&block)
		end
	end

	def parser(&block)
		Parser.new(&block)
	end
end

def rule(&block)
	Parser.new {|i,e|
		Parser::Context.new.instance_eval(&block).parse(i)
	}
end


def self.parse(rule, src)
	rule.parse(src)[0]
end


end  # module Phraser

