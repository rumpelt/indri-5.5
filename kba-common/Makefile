SHARED=
INCPATH=-I../include -I../include/thrift -Iinclude/
LIBPATH=-L../obj -Lobj -Llib
LIBS= -lboost_program_options -lindri -llzma -lthrift
APP=KbaProcessing
CXX = g++
CXXFLAGS = -Wall -g  -c
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


