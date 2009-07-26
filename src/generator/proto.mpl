__BEGIN__

def xjoin(tgt, between, after, join)
	tgt.map {|a, b| "#{a}#{between}#{b}#{after}" }.join(join)
end

def sep(i, c, between = ",")
	between if c.length != i+1
end

__END__
%def genproto
%self.each do |msg|
struct [%msg.name%] {
	static const int method = [%msg.id%];  %>if msg.id

	%# constructor
	%unless msg.body.empty?
	[%msg.name%](
		const [%b.type%]& [%b.name%]_[%sep(i, msg.body)%]  %|b,i| msg.body.each_with_index
	) :
		[%b.name%]([%b.name%]_)[%sep(i, msg.body)%]  %|b,i| msg.body.each_with_index
	{ }
	%end

	%# default constructor
	[%msg.name%]() { }

	%# unpack
	void msgpack_unpack(msgpack::object o)
	{
		if(o.type != msgpack::type::ARRAY ||
				o.via.array.size < [%msg.required.size%] ||
				o.via.array.size > [%msg.body.size%]) {
			throw msgpack::type_error();
		}
		[%b.name%] = o.via.array.ptr[[%i%]].as<[%b.type%] >();  %|b,i| msg.required.each_with_index

		%unless msg.optional.empty?
		switch(o.via.array.size) {
		%msg.optional.length.times do |i|
		case [%msg.required.size + i%]:
			[%x.name%] = o.via.array.ptr[[%i%]].as<[%x.type%] >(); %|x| msg.optional[0..i].each
			[%x.name%] = [%x.default%];  %|x| msg.optional[i+1..-1].each
			break;
		%end
		}
		%end
	}

	%# pack
	template <typename Packer>
	void msgpack_pack(Packer& pk) const
	{
		pk.pack_array([%msg.body.size%]);
		pk.pack([%b.name%]); %|b| msg.body.each
	}

	%# members
	[%b.type%] [%b.name%];  %|b| msg.body.each
};

%end
%end  # def genproto
%# vim: syntax=mplex
