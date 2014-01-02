INCPATH=-I. -I../library/include  -I../kba-common
#INCPATH=-I. -I../library/include   -I/usa/arao/gcctools/include/c++/4.8.2 -I/usa/arao/gcctools/include -I/usa/arao/projects/libxml2/include

#LIBPATH=-L../library/lib  -L/usa/arao/gcctools/lib64 -L/usa/arao/gcctools/lib
LIBPATH=-L../library/lib -L../kba-common

LIBS= -lindri  -lz -liberty -lpthread -lkbacommon -lboost_program_options  -llzma -lthrift   -lboost_thread-mt

APP=IndriBuildIndex
#CXX = /usa/arao/gcctools/bin/g++ 
CXX=g++

CXXFLAGS = -g -c -std=c++0x -DPACKAGE_NAME=\"Indri\" -DPACKAGE_TARNAME=\"indri\" -DPACKAGE_VERSION=\"5.5\" -DPACKAGE_STRING=\"Indri\ 5.5\" -DPACKAGE_BUGREPORT=\"project@lemurproject.org\" -DYYTEXT_POINTER=1 -DINDRI_STANDALONE=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DHAVE_LIBZ=1 -DHAVE_LIBIBERTY=1 -DHAVE_NAMESPACES=/\*\*/ -DISNAN_IN_NAMESPACE_STD=/\*\*/ -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FSEEKO=1 -DHAVE_EXT_ATOMICITY_H=1 -DP_NEEDS_GNU_CXX_NAMESPACE=1 -DHAVE_MKSTEMP=1 -DHAVE_MKSTEMPS=1 -DNDEBUG=1  $(INCPATH)

#CXXFLAGS = -g -c -std=c++11 -DPACKAGE_NAME=\"Indri\" -DPACKAGE_TARNAME=\"indri\" -DPACKAGE_VERSION=\"5.5\" -DPACKAGE_STRING=\"Indri\ 5.5\" -DPACKAGE_BUGREPORT=\"project@lemurproject.org\" -DYYTEXT_POINTER=1 -DINDRI_STANDALONE=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DHAVE_LIBZ=1 -DHAVE_LIBIBERTY=1 -DHAVE_NAMESPACES=/\*\*/ -DISNAN_IN_NAMESPACE_STD=/\*\*/ -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FSEEKO=1 -DHAVE_EXT_ATOMICITY_H=1 -DP_NEEDS_GNU_CXX_NAMESPACE=1 -DHAVE_MKSTEMP=1 -DHAVE_MKSTEMPS=1 -DNDEBUG=1  $(INCPATH) 

OBJECTS = $(patsubst %.cc,%.o,$(wildcard *.cc))

#RUNDIRFLAGS = -Wl,-R,'../library/lib:/usa/arao/gcctools/lib64:/usa/arao/gcctools/lib:.' 
RUNDIRFLAGS = -Wl,-R,'../library/lib:.' 

%.o : %.cc
	$(CXX) $(CXXFLAGS) $? -o $@  $(INCPATH)

all:  $(OBJECTS)
	$(CXX) -Wall -g  -o $(APP) $(OBJECTS) $(LIBPATH) $(LIBS) $(RUNDIRFLAGS) 


install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)

