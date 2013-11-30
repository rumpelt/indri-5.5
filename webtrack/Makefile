SHARED=
INCPATH=-I. -I../include -I/usa/arao/gcctools/include/c++/4.8.2 -I/usa/arao/gcctools/include -I/usa/arao/projects/libxml2
LIBPATH=-L../obj  -L/usa/arao/gcctools/lib64
LIBS= -lboost_program_options -lindri   
APP=clue
CXX = /usa/arao/gcctools/bin/g++ 
CXXFLAGS = -Wall -g -c -std=c++11x
#CXXFLAGS = -Wall -g -c -std=c++0x 
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

