SRCDIR = .
CXXFLAGS += -I$(SRCDIR) 

MODULES = \
	open-layered-graph.o \
	open.o 


.PHONY : all
all : $(MODULES)
	@echo All done

%-gen.idx : %.idx
	java -jar $(INDEXICAL_JAR) -f $< -o $@ --genPropForce $(IDXFLAGS)

# open.cpp : open-gen.idx
# 	java -jar $(INDEXICAL_JAR) -f $< -t gecode -o $* $(IDXFLAGS)


# $@ for the pattern-matched target
# $< for the pattern-matched dependency
%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(MODULES:.o=.d)

.PHONY : clean
clean : 
	rm -f $(MODULES:.o=.d)
	rm -f $(MODULES:.o=.d.*)
	rm -f $(MODULES)
