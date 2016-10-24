MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
CFLAGS        = -m64 -pipe -Wall -W -D_REENTRANT -fPIE $(DEFINES)
CXXFLAGS      = -m64 -pipe -fopenmp -std=c++0x -Wall -Wno-unused-parameter -W -D_REENTRANT -fPIE $(DEFINES)
LINK          = g++
LFLAGS        = -m64 -fopenmp
LIBS          = $(SUBLIBS) -lX11 -lpthread 
AR            = ar cqs
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p


####### Files

SOURCES       = src/main.cpp \
		src/GaussFilter.cpp \
		src/multimeter.cpp \
		src/LinearFilter.cpp \
		src/SingleCompartment.cpp \
		src/module.cpp \
		src/Retina.cpp \
		src/GratingGenerator.cpp \
		src/InterfaceNEST.cpp \
		src/StaticNonLinearity.cpp \
		src/DisplayManager.cpp \
		src/FileReader.cpp \
		src/whiteNoise.cpp \
		src/impulse.cpp \
		src/ShortTermPlasticity.cpp \
		src/fixationalMovGrating.cpp \
		src/SpikingOutput.cpp \
		src/SequenceOutput.cpp \
		src/constants.cpp 
OBJECTS       = build/main.o \
		build/GaussFilter.o \
		build/multimeter.o \
		build/LinearFilter.o \
		build/SingleCompartment.o \
		build/module.o \
		build/Retina.o \
		build/GratingGenerator.o \
		build/InterfaceNEST.o \
		build/StaticNonLinearity.o \
		build/DisplayManager.o \
		build/FileReader.o \
		build/whiteNoise.o \
		build/impulse.o \
		build/ShortTermPlasticity.o \
		build/fixationalMovGrating.o \
		build/SpikingOutput.o \
		build/SequenceOutput.o \
		build/constants.o

TARGET        = corem


first: all

debug: CXXFLAGS += -DDEBUG -g
debug: CFLAGS += -DDEBUG -g
debug: first

release: CXXFLAGS += -O2
release: CFLAGS += -O2
release: LFLAGS += -Wl,-O1
release: first

####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Install rules

PREFIX = /usr/local

.PHONY: install
install: corem
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/corem

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/corem

####### Build rules

files := $(shell mkdir build/)

all: Makefile $(TARGET)


$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

dist: 
	@test -d .tmp/corem1.0.0 || mkdir -p .tmp/corem1.0.0
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/corem1.0.0/ && $(COPY_FILE) --parents src/GaussFilter.h src/multimeter.h src/LinearFilter.h src/SingleCompartment.h src/module.h src/Retina.h src/GratingGenerator.h src/InterfaceNEST.h src/StaticNonLinearity.h src/DisplayManager.h src/FileReader.h src/whiteNoise.h src/impulse.h src/ShortTermPlasticity.h src/fixationalMovGrating.h src/constants.h .tmp/corem1.0.0/ && $(COPY_FILE) --parents src/main.cpp src/GaussFilter.cpp src/multimeter.cpp src/LinearFilter.cpp src/SingleCompartment.cpp src/module.cpp src/Retina.cpp src/GratingGenerator.cpp src/InterfaceNEST.cpp src/StaticNonLinearity.cpp src/DisplayManager.cpp src/FileReader.cpp src/whiteNoise.cpp src/impulse.cpp src/ShortTermPlasticity.cpp src/fixationalMovGrating.cpp src/constants.cpp .tmp/corem1.0.0/ && (cd `dirname .tmp/corem1.0.0` && $(TAR) corem1.0.0.tar corem1.0.0 && $(COMPRESS) corem1.0.0.tar) && $(MOVE) `dirname .tmp/corem1.0.0`/corem1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/corem1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

check: first

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_wayland-server-header_make_all:
compiler_wayland-server-header_clean:
compiler_wayland-client-header_make_all:
compiler_wayland-client-header_clean:
compiler_moc_header_make_all:
compiler_moc_header_clean:
compiler_wayland-code_make_all:
compiler_wayland-code_clean:
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: 

####### Compile

build/main.o: src/main.cpp CImg-1.6.0_rolling141127/CImg.h \
		src/InterfaceNEST.h \
		src/DisplayManager.h \
		src/multimeter.h \
		src/constants.h \
		src/Retina.h \
		src/module.h \
		src/LinearFilter.h \
		src/SingleCompartment.h \
		src/GaussFilter.h \
		src/GratingGenerator.h \
		src/fixationalMovGrating.h \
		src/SpikingOutput.h \
		src/SequenceOutput.h \
		src/whiteNoise.h \
		src/impulse.h \
		src/FileReader.h \
		src/StaticNonLinearity.h \
		src/ShortTermPlasticity.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/main.o src/main.cpp

build/GaussFilter.o: src/GaussFilter.cpp src/GaussFilter.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/GaussFilter.o src/GaussFilter.cpp

build/multimeter.o: src/multimeter.cpp src/multimeter.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/multimeter.o src/multimeter.cpp

build/LinearFilter.o: src/LinearFilter.cpp src/LinearFilter.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/LinearFilter.o src/LinearFilter.cpp

build/SingleCompartment.o: src/SingleCompartment.cpp src/SingleCompartment.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/SingleCompartment.o src/SingleCompartment.cpp

build/SpikingOutput.o: src/SpikingOutput.cpp src/SpikingOutput.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/SpikingOutput.o src/SpikingOutput.cpp

build/SequenceOutput.o: src/SequenceOutput.cpp src/SequenceOutput.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/SequenceOutput.o src/SequenceOutput.cpp

build/module.o: src/module.cpp src/module.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/module.o src/module.cpp

build/Retina.o: src/Retina.cpp src/Retina.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/LinearFilter.h \
		src/SingleCompartment.h \
		src/SpikingOutput.h \
		src/SequenceOutput.h \
		src/constants.h \
		src/GaussFilter.h \
		src/GratingGenerator.h \
		src/fixationalMovGrating.h \
		src/whiteNoise.h \
		src/impulse.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/Retina.o src/Retina.cpp

build/GratingGenerator.o: src/GratingGenerator.cpp src/GratingGenerator.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/GratingGenerator.o src/GratingGenerator.cpp

build/InterfaceNEST.o: src/InterfaceNEST.cpp src/InterfaceNEST.h \
		src/DisplayManager.h \
		src/multimeter.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h \
		src/Retina.h \
		src/module.h \
		src/LinearFilter.h \
		src/SingleCompartment.h \
		src/SpikingOutput.h \
		src/SequenceOutput.h \
		src/GaussFilter.h \
		src/GratingGenerator.h \
		src/fixationalMovGrating.h \
		src/whiteNoise.h \
		src/impulse.h \
		src/FileReader.h \
		src/StaticNonLinearity.h \
		src/ShortTermPlasticity.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/InterfaceNEST.o src/InterfaceNEST.cpp

build/StaticNonLinearity.o: src/StaticNonLinearity.cpp src/StaticNonLinearity.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/module.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/StaticNonLinearity.o src/StaticNonLinearity.cpp

build/DisplayManager.o: src/DisplayManager.cpp src/DisplayManager.h \
		src/multimeter.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h \
		src/Retina.h \
		src/module.h \
		src/LinearFilter.h \
		src/SingleCompartment.h \
		src/SpikingOutput.h \
		src/SequenceOutput.h \
		src/GaussFilter.h \
		src/GratingGenerator.h \
		src/fixationalMovGrating.h \
		src/whiteNoise.h \
		src/impulse.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/DisplayManager.o src/DisplayManager.cpp

build/FileReader.o: src/FileReader.cpp src/FileReader.h \
		src/DisplayManager.h \
		src/multimeter.h \
		CImg-1.6.0_rolling141127/CImg.h \
		src/constants.h \
		src/Retina.h \
		src/module.h \
		src/LinearFilter.h \
		src/SingleCompartment.h \
		src/SpikingOutput.h \
		src/SequenceOutput.h \
		src/GaussFilter.h \
		src/GratingGenerator.h \
		src/fixationalMovGrating.h \
		src/whiteNoise.h \
		src/impulse.h \
		src/StaticNonLinearity.h \
		src/ShortTermPlasticity.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/FileReader.o src/FileReader.cpp

build/whiteNoise.o: src/whiteNoise.cpp src/whiteNoise.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/whiteNoise.o src/whiteNoise.cpp

build/impulse.o: src/impulse.cpp src/impulse.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/impulse.o src/impulse.cpp

build/ShortTermPlasticity.o: src/ShortTermPlasticity.cpp src/ShortTermPlasticity.h \
		src/module.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/ShortTermPlasticity.o src/ShortTermPlasticity.cpp

build/fixationalMovGrating.o: src/fixationalMovGrating.cpp src/fixationalMovGrating.h \
		CImg-1.6.0_rolling141127/CImg.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/fixationalMovGrating.o src/fixationalMovGrating.cpp

build/constants.o: src/constants.cpp src/constants.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o build/constants.o src/constants.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

