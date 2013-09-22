SHARED=
INCPATH=-I../include -I../include/thrift -I../kba-common
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


