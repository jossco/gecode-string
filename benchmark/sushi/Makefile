GSDIR = ../../src
override CXXFLAGS += -I$(GSDIR)

TARGETS = 	\
	equation1 \
	equation2 \
	equation3 \
	equation4 \
	equation5

GSINCLUDES = \
	open-layered-graph.o \
	bounded-none.o \
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

#%.d: %.cpp
#	@set -e; rm -f $@; \
#	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$
%.pdf: %.ps
	ps2pdf $< $@
%.ps: %.dot
	dot -Tps $< > $@
%.dot: %.fst
	fstdraw --isymbols=isyms.txt $< $@

#include $(OBJS:.o=.d)
