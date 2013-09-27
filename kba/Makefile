SHARED=
INCPATH=-I../include -I../include/thrift -I../kba-common -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal

LIBPATH=-L../obj  -L../kba-common
LIBS= -lkbacommon -lboost_program_options -lindri -llzma -lthrift -lrdf -lraptor2 -lrasqal
APP=KbaProcessing
CXX = g++
CXXFLAGS = -Wall -g -c
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


