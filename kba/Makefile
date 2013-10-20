SHARED=
INCPATH=-I. -I../kba-common -I../include -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal -I/usa/arao/projects/bdb-lib/include

LIBPATH=-L../obj  -L../kba-common
LIBS= -lkbacommon -lboost_thread-mt -lboost_program_options -lindri -llzma -lthrift -lrdf -lraptor2 -lrasqal -ldb_cxx
APP=KbaProcessing
CXX = g++
CXXFLAGS = -Wall -g -c -std=c++0x
OBJECTS = $(patsubst %.cc,%.o,$(wildcard *.cc))

RUNDIRFLAGS = -Wl,-R,'$$ORIGIN/../kba-common'

%.o : %.cc
	$(CXX) $(CXXFLAGS) $? -o $@  $(INCPATH) 

all:  $(OBJECTS)
	$(CXX) -Wall -g  -o $(APP) $(OBJECTS)    $(LIBPATH) $(LIBS) $(RUNDIRFLAGS)


install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


