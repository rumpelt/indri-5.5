
SHARED=
INCPATH=-I../include -I/usa/arao/projects/redlandlib/include -I/usa/arao/projects/redlandlib/include/raptor2 -I/usa/arao/projects/redlandlib/include/rasqal
LIBPATH=-L../obj -L.
LIBS=-lrdf -lraptor2 -lrasqal 
APP=RDFParser
CXX = g++
CXXFLAGS = -Wall -g 
RUNDIRFLAGS = -Wl,-R,'$$ORIGIN/'
all:
	$(CXX) $(CXXFLAGS) $(INCPATH) $(APP).cpp -o $(APP) $(LIBPATH) $(LIBS) $(RUNDIRFLAGS)

install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


