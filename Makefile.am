export ERB
export RUBY
export RAGEL

MPLEX = $(RUBY) -I$(PWD)/src/generator $(PWD)/src/generator/mplex
export MPLEX

SUFFIXES = .mpl
%.cc: %.mpl
	$(MPLEX) -o $@ $<

SUBDIRS = src

