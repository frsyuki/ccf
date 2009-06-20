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

	def result(context = nil)
		context.instance_eval(&@proc)
	end

	def self.result(src, context = nil, fname = "(mplex)")
		context.instance_eval(compile(src), fname)
	end

	def self.script(src)
		compile(src)
	end

	private
	def self.compile(src)
		# MPLEX_COMPILE_BEGIN
		o = "_o='';"
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
				(o << "_o.concat #{m}.to_s;"; next) if i % 2 == 1
				o << "_o.concat #{m.dump};" unless m.empty?
			}
			o << "\n"
		}
		o << "_o"
		# MPLEX_COMPILE_END
	end
end


if $0 == __FILE__

require 'optparse'

op = OptionParser.new
op.banner += " [input=stdin]"

input_file = "-"
output_file = "-"
ctx_file = nil
load_libs = []
script_mode = false

op.on("-c file",    "context ruby script") {|v| ctx_file    = v    }
op.on("-o file",    "output file")         {|v| output_file = v    }
op.on("-r library", "load library")        {|v| load_libs  << v    }
op.on("-x",         "print ruby script")   {|v| script_mode = true }

begin
	op.parse!(ARGV)

	input_file = ARGV.shift unless ARGV.empty?

	raise "unexpected argument #{ARGV.first.inspect}" unless ARGV.empty?
	raise "both input and context are stdin" if input_file == "-" && ctx_file == "-"

rescue
	puts op.to_s
	puts $!
	exit 1
end

load_libs.each {|lib| require lib }

if input_file == "-"
	input = $stdin.read
else
	input = File.read(input_file)
	input_file = "(stdin)"
end

b = nil; Object.instance_eval { b = binding }
if ctx_file == "-"
	ctx = eval($stdin.read, b, "(stdin)")
elsif ctx_file
	#ctx = load File.read(ctx_file)
	ctx = eval(File.read(ctx_file), b, ctx_file)
end

if output_file == "-"
	output = $stdout
else
	output = File.open(output_file, "w")
end

if script_mode
	output.write Mplex.script(input)
else
	ctx ||= Object.new
	output.write Mplex.result(input, ctx, input_file)
end

end  # $0 == __FILE__

