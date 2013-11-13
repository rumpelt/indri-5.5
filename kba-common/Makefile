SHARED=
INCPATH=-I.   -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal

LIBPATH=-L../obj -Lobj -Llib
LIBS= -lboost_program_options -lindri -llzma -lthrift -lrdf -lraptor2 -lrasqal
CXX = g++
CXXFLAGS = -std=c++0x -Wall -g  -c -O3 -ffast-math
OBJECTS = $(patsubst %.cc,%.o,$(wildcard *.cc))
OBJECTLIB = libkbacommon.a
%.o : %.cc
	$(CXX) $(CXXFLAGS) $? -o $@  $(INCPATH)

all: $(OBJECTS)
	$(AR) -rs $(OBJECTLIB) $(OBJECTS)
install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


