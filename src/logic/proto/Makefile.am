
LIBDIR = ../../generator
LIBS   = $(LIBDIR)/protogen $(LIBDIR)/proto.mpl $(LIBDIR)/stub.mpl \
		 $(LIBDIR)/mplex.rb $(LIBDIR)/phraser.rb $(LIBDIR)/ccfproto.rb

RUN    = $(RUBY) -I$(LIBDIR) $(LIBDIR)/protogen

../server/proto.h: ../server/proto.mpl ../server/message.ccf $(LIBS)
	$(RUN) ../server/proto.mpl ../server/message.ccf $@

../server/stub.h: ../server/stub.mpl ../server/message.ccf $(LIBS)
	$(RUN) ../server/stub.mpl ../server/message.ccf $@

nodist_noinst_HEADERS = \
	../server/proto.h \
	../server/stub.h

EXTRA_DIST = \
		protogen \
		../server/message.ccf \
		../server/proto.mpl \
		../server/stub.mpl

MOSTLYCLEANFILES = \
		$(nodist_noinst_HEADERS)

