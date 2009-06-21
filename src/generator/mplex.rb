#!/usr/bin/env ruby
#
# Mplex: Extended Metaprogramming Library
#
# Copyright (C) 2009 FURUHASHI Sadayuki
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

class Mplex
	def initialize(src, fname = "(mplex)")
		@script = self.class.compile(src)
		b = nil; Object.new.instance_eval { b = binding }
		@proc   = eval("Proc.new{#{@script}}", b, fname)
	end
	attr_reader :script

	def result(context = nil, output = "")
		self.class.with_context(context, output) {|ctx|
			ctx.instance_eval(&@proc)
		}
	end

	def self.result(src, context = nil, fname = "(mplex)", output = "")
		with_context(context, output) {|ctx|
			ctx.instance_eval(compile(src), fname)
		}
	end

	def self.script(src)
		compile(src)
	end

	private
	def self.with_context(context, output, &block)
		context ||= Object.new
		save = context.instance_variable_get(:@_mplexout)
		context.instance_variable_set(:@_mplexout, output)
		block.call(context)
		output = context.instance_variable_get(:@_mplexout)
		context.instance_variable_set(:@_mplexout, save)
		output
	end

	def self.compile(src)
		# MPLEX_COMPILE_BEGIN
		o = ""
		k = false
		src.each_line {|t|
			(k = false; o << "\n"; next) if k && t == "__END__\n"
			(o << t; next) if k
			(k = true;  o << "\n"; next) if t == "__BEGIN__\n"

			c, l = t.split(/^[ \t]*%/,2)
			(o << l; next) if l

			c, a, b = t.split(/[ \t]*\%\|([a-z0-9\,]*)\|/,2)
			t = "[%:#{b.strip} do |#{a}|%]#{c+"\n"}[%:end%]" if b

			c, q = t.split(/[ \t]*\%\>/,2)
			t = "[%:(%]#{c+"\n"}[%:)#{q.strip}%]" if q

			t.split(/\[\%(\:?.*?)\%\]/m).each_with_index {|m,i|
				(o << "#{m[1..-1]};"; next) if m[0] == ?: && i % 2 == 1
				(o << "@_mplexout.concat #{m}.to_s;"; next) if i % 2 == 1
				o << "@_mplexout.concat #{m.dump};" unless m.empty?
			}
			o << "\n"
		}
		o << "@_mplexout"
		# MPLEX_COMPILE_END
	end
end

