SHARED=
INCPATH=-I../include -I../include/thrift
LIBPATH=-L../obj -L.
LIBS=-lindri -llzma -lthrift
APP=KbaProcessing
CXX = g++
CXXFLAGS = -Wall -g 
RUNDIRFLAGS = -Wl,-R,'$$ORIGIN/'
all:
	$(CXX) $(CXXFLAGS) $(INCPATH) $(APP).cc -o $(APP) $(LIBPATH) $(LIBS) $(RUNDIRFLAGS)

install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


