GSDIR = ../src
override CXXFLAGS += -I$(GSDIR)

TARGETS = 	\
	test_basic \
	test_equal \
	test_concat

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

$(TARGETS) : % : %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ \
		$(MODULES) \
		$(LDLIBS)

# $@ for the pattern-matched target
# $< for the pattern-matched dependency
%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

#%.d: %.cpp
#	@set -e; rm -f $@; \
#	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$

#include $(OBJS:.o=.d)
