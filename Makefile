GECODE_BASE = /Users/joe/local/gecode-4.2.0
GECODE_INCL = $(GECODE_BASE)/include
SRCDIR = src
CXX = g++-4.8
DEBUGFLAGS = 
OPTFLAGS = 
CPPFLAGS = $(OPTFLAGS)
override CPPFLAGS += -I$(SRCDIR)
GIST = true
LDFLAGS = -L$(GECODE_BASE)/lib
LIBS = -lgecodedriver
ifneq "$(strip $(GIST))" "false"
LIBS += -lgecodegist
endif
LIBS += \
	-lgecodesearch \
	-lgecodeminimodel\
	-lgecodeint \
	-lgecodekernel \
	-lgecodesupport \
	-lfst
ifneq (,$(findstring clang,$(CXX)))
override LIBS += -stdlib=libstdc++
endif

LINK_TARGET = pentominoes

ALT_TARGETS =

MODULES = open-layered-graph.o

OBJS = $(patsubst %,%.o,$(LINK_TARGET)) $(patsubst %,%.o,$(ALT_TARGETS))

REBUILDABLES = $(OBJS) $(LINK_TARGET) $(MODULES) $(ALT_TARGETS)

build : all
.PHONY : build
all : $(LINK_TARGET)
	@echo All done
.PHONY : all

.PHONY : clean
clean :
	rm -f $(REBUILDABLES)
	rm -f $(OBJS:.o=.d)
	rm -f $(OBJS:.o=.d.*)
	rm -f $(MODULES:.o=.d)
	rm -f $(MODULES:.o=.d.*)
	@echo Clean done
	
$(LINK_TARGET) : % : %.o $(MODULES)
	$(CXX) $(DEBUGFLAGS) -o $@ $(LDFLAGS) $^ $(LIBS)
$(ALT_TARGETS) : % : %.o $(MODULES)
	$(CXX) $(DEBUGFLAGS) -o $@ $(LDFLAGS) $^ $(LIBS)

# $@ for the pattern-matched target
# $< for the pattern-matched dependency
%.o : $(SRCDIR)/%.cpp
	$(CXX) $(DEBUGFLAGS) $(CPPFLAGS) -I$(GECODE_INCL) -o $@ -c $<
	
%.d: $(SRCDIR)/%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) -I$(GECODE_INCL) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(OBJS:.o=.d)
include $(MODULES:.o=.d)