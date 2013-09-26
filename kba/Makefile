SHARED=
INCPATH=-I../include -I../include/thrift -I../kba-common -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal

LIBPATH=-L../obj  -L../kba-common
LIBS= -lboost_program_options -lindri -llzma -lthrift
APP=KbaProcessing
CXX = g++
CXXFLAGS = -Wall -g 


RUNDIRFLAGS = -Wl,-R,'$$ORIGIN/../kba-common'

 
all:
	$(CXX) $(CXXFLAGS)  $(APP).cc  -o $(APP) $(INCPATH) $(LIBPATH) $(LIBS) $(RUNDIRFLAGS)


install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


