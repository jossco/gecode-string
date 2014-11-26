export GECODE = /home/stringy/software/gecode-4.3.2/build
export GECODE_LIB = $(GECODE)/lib
export GECODE_INCL = $(GECODE)/include
export GECODE_SRC = /home/stringy/software/gecode-4.3.2
export GIST = false
export ARCH = linux

SRCDIR = .

override CXXFLAGS += -std=c++11 -I$(SRCDIR) -I$(GECODE_INCL)
STDLIB = libstdc++
FSTLIB = /usr/local/lib
ifeq "$(strip $(ARCH))" "mac"
export CXX = g++-4.8
endif
ifneq (,$(findstring clang,$(CXX)))
override LDLIBS += -stdlib=$(STDLIB)
override CXXFLAGS += -stdlib=$(STDLIB)
else
override CXXFLAGS += -fimplement-inlines -fno-inline-functions
endif
override CXXFLAGS += -ggdb -pipe -Wall  -fPIC -pthread -pg 
export CXXFLAGS

export INDEXICAL_JAR = /Users/joe/src/indexicals-old/indexicals.jar
IDXFLAGS = 
override IDXFLAGS += -v4
export IDXFLAGS

export LDFLAGS = -L$(GECODE_LIB) -L$(FSTLIB)
export LDLIBS += -lgecodedriver
ifneq "$(strip $(GIST))" "false"
LDLIBS += -lgecodegist
endif
LDLIBS += -lgecodesearch -lgecodeminimodel -lgecodeint -lgecodekernel -lgecodesupport -lfst
#ifeq "$(strip $(ARCH))" "linux"
LDLIBS += -ldl
#endif

BINDIR = bin
SUBDIRS = src mzndir benchmark test
.PHONY : all
all : src mzn test kaluza
	@echo All done

.PHONY : mzn
mzn : mzn-gecode-string fzn-gecode-string

.PHONY : mzn-gecode-string
mzn-gecode-string : $(BINDIR)/mzn-gecode-string
$(BINDIR)/mzn-gecode-string: $(GECODE_SRC)/tools/flatzinc/mzn-gecode
#$(BINDIR)/mzn-gecode-string: $(GECODE)/bin/mzn-gecode
	cp $< mzn-gecode
	sed -e 's/fzn-gecode/fzn-gecode-string/' \
	    -e 's#minizinc -I#minizinc -I'$(CURDIR)'/minizinc/mznlib -I#' \
	    < mzn-gecode > $@
	rm mzn-gecode
	chmod +x $@

.PHONY : mzndir
mzndir: src
	$(MAKE) -C minizinc

.PHONY : fzn-gecode-string src
fzn-gecode-string : $(BINDIR)/fzn-gecode-string
$(BINDIR)/fzn-gecode-string: src/open.o src/open-layered-graph.o mzndir
	$(CXX) -o $@ $(CXXFLAGS) \
			src/open.o \
			src/open-layered-graph.o \
			$(GECODE_SRC)/tools/flatzinc/fzn-gecode.o \
			$(GECODE_SRC)/gecode/flatzinc/flatzinc.o \
			minizinc/flatzinc-binding.o \
			$(GECODE_SRC)/gecode/flatzinc/registry.o \
			$(GECODE_SRC)/gecode/flatzinc/parser.tab.o \
			$(GECODE_SRC)/gecode/flatzinc/lexer.yy.o \
			$(LDFLAGS) \
			-lgecodedriver -lgecodesearch -lgecodeminimodel -lgecodeset -lgecodeint -lgecodekernel -lgecodesupport -lfst -ldl
.PHONY : src
src :
	$(MAKE) -C src

.PHONY : benchmark
examples : src
	$(MAKE) -C benchmark

.PHONY : test
test : src
	$(MAKE) -C test

.PHONY : debug
debug : CXXFLAGS += -g -g3
debug : all

.PHONY : opt
opt : CXXFLAGS += -O3
opt : all

.PHONY : clean
clean :
	rm -f bin/mzn-gecode-string
	rm -f bin/fzn-gecode-string
	for d in $(SUBDIRS); \
	do \
		$(MAKE) --directory=$$d clean; \
	done


