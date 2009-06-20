
LIBDIR = ../../generator
LIBS   = $(LIBDIR)/protogen $(LIBDIR)/mplex.rb $(LIBDIR)/phraser.rb $(LIBDIR)/ccfproto.rb
RUN    = $(RUBY) -I$(LIBDIR) $(LIBDIR)/protogen

PROTO_TMPL = $(LIBDIR)/proto.mpl
STUB_TMPL  = $(LIBDIR)/stub.mpl

../server/proto.h: server.msg $(PROTO_TMPL) $(LIBS)
	$(RUN) $(PROTO_TMPL) $< $@

../server/stub.h: server.msg $(STUB_TMPL) $(LIBS)
	$(RUN) $(STUB_TMPL) $< $@

../client/proto.h: client.msg $(PROTO_TMPL) $(LIBS)
	$(RUN) $(PROTO_TMPL) $< $@

../client/stub.h: client.msg $(STUB_TMPL) $(LIBS)
	$(RUN) $(STUB_TMPL) $< $@


nodist_noinst_HEADERS = \
	../client/proto.h \
	../client/stub.h \
	../server/proto.h \
	../server/stub.h

EXTRA_DIST = \
		protogen \
		client.msg \
		server.msg

MOSTLYCLEANFILES = \
		$(nodist_noinst_HEADERS)

