GECODE_BASE = /Users/joe/local/gecode-4.2.0
GECODE_INCL = $(GECODE_BASE)/include
SRCDIR = src
CXX = g++-4.8
override CXXFLAGS += -I$(SRCDIR) -I$(GECODE_INCL)
GIST = true
LDFLAGS = -L$(GECODE_BASE)/lib
LDLIBS = -lgecodedriver
ifneq "$(strip $(GIST))" "false"
LDLIBS += -lgecodegist
endif
LDLIBS += \
	-lgecodesearch \
	-lgecodeminimodel\
	-lgecodeint \
	-lgecodekernel \
	-lgecodesupport \
	-lfst
ifneq (,$(findstring clang,$(CXX)))
override LDLIBS += -stdlib=libstdc++
endif

LINK_TARGET = 	\
	revenant

ALT_TARGETS = 	dfa2fst

MODULES = \
	open-layered-graph.o \
	bounded-none.o

OBJS = $(patsubst %,%.o,$(LINK_TARGET)) $(patsubst %,%.o,$(ALT_TARGETS))

REBUILDABLES = $(OBJS) $(LINK_TARGET) $(MODULES) $(ALT_TARGETS)

build : all
.PHONY : build
all : $(LINK_TARGET)
	@echo All done
.PHONY : all

.PHONY : debug
debug : CXXFLAGS += -g -g3
debug : all

.PHONY : clean
clean :
	rm -f $(REBUILDABLES)
	rm -f $(OBJS:.o=.d)
	rm -f $(OBJS:.o=.d.*)
	rm -f $(MODULES:.o=.d)
	rm -f $(MODULES:.o=.d.*)
	@echo Clean done
	
$(LINK_TARGET) : % : %.o $(MODULES)
	$(CXX) -o $@ $(LDFLAGS) $^ $(LDLIBS)
$(ALT_TARGETS) : % : %.o $(MODULES)
	$(CXX) -o $@ $(LDFLAGS) $^ $(LDLIBS)

# $@ for the pattern-matched target
# $< for the pattern-matched dependency
%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<
	
%.d: $(SRCDIR)/%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
%.pdf: %.ps
	ps2pdf $< $@
%.ps: %.dot
	dot -Tps $< > $@
%.dot: %.fst
	fstdraw --isymbols=isyms.txt $< $@
	
include $(OBJS:.o=.d)
include $(MODULES:.o=.d)