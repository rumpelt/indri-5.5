
SHARED=
INCPATH=-I../include -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal -I../kba-common/
LIBPATH=-L../obj -L../kba-common -L.
LIBS=-lrdf -lraptor2 -lrasqal  -lboost_program_options -lkbacommon
APP=RDFParser
CXX = g++ -std=c++0x
CXXFLAGS = -Wall -g 
RUNDIRFLAGS = -Wl,-R,'$$ORIGIN/:$$ORIGIN/../kba-common'
all:
	$(CXX) $(CXXFLAGS) $(INCPATH) $(APP).cpp  -o $(APP) $(LIBPATH) $(LIBS) $(RUNDIRFLAGS)

install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


