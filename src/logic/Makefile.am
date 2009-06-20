AM_CPPFLAGS   = -I..
AM_C_CPPFLAGS = -I..

SUBDIRS = proto

bin_PROGRAMS = mpecho-server mpecho-client

mpecho_server_SOURCES = \
		server/main.cc

mpecho_server_LDADD = \
		../ccf/libccf.a \
		../mpsrc/libmpio.a \
		../cclog/libcclog.a

mpecho_client_SOURCES = \
		client/main.cc

mpecho_client_LDADD = \
		../ccf/libccf.a \
		../mpsrc/libmpio.a \
		../cclog/libcclog.a

# work around for duplicated file name
mpecho_server_CFLAGS = $(AM_CFLAGS)
mpecho_server_CXXFLAGS = $(AM_CXXFLAGS)
mpecho_client_CFLAGS = $(AM_CFLAGS)
mpecho_client_CXXFLAGS = $(AM_CXXFLAGS)

