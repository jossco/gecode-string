SRCDIR = ../src
CXXFLAGS += -I$(SRCDIR) 

MODULES = \
	flatzinc-binding.o

.PHONY : all
all : $(MODULES)
	@echo All done

# open.cpp : open-gen.idx
# 	java -jar $(INDEXICAL_JAR) -f $< -t gecode -o $* $(IDXFLAGS)

#flatzinc-binding.cpp : %-gen.idx
#	java -jar $(INDEXICAL_JAR) -f $< -o $@ -t gecode/fzn $(IDXFLAGS)

flatzinc-binding.o: flatzinc-binding.cpp $(SRCDIR)/open.hh $(SRCDIR)/open-layered-graph.hh
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -c


.PHONY : clean
clean : 
	rm -f $(MODULES)
