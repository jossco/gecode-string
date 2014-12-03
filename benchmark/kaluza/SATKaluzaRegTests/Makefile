GSDIR = ../../../src
override CXXFLAGS += -I$(GSDIR)

TARGETS = \
	concat \
	indexof \
	bettermatch1 \
	bettermatch2 \
	replace \
	streq \
	search

GSINCLUDES = \
	open-layered-graph.o \
	open.o

MODULES = $(patsubst %,$(GSDIR)/%,$(GSINCLUDES))

OBJS = $(patsubst %,%.o,$(TARGETS)) 

REBUILDABLES = $(OBJS) $(TARGETS)

all : $(TARGETS)
	@echo All done
.PHONY : all


.PHONY : clean
clean :
	rm -f $(REBUILDABLES)
	rm -f $(OBJS:.o=.d)
	rm -f $(OBJS:.o=.d.*)
	@echo Clean done

$(TARGETS) : % : %.o $(MODULES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ \
		$(LDLIBS)

# $@ for the pattern-matched target
# $< for the pattern-matched dependency
%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<