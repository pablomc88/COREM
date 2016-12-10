# Simplified makefile for COREM project
# This makefile does not have to be modified whenever a new source file is added to the project

# Target executable file:
EXE = corem

all: release

CPP = g++
CPP_FLAGS = -m64 -pipe -fopenmp -std=c++0x -Wall -Wno-unused-parameter -W -fPIE -D_REENTRANT -Dcimg_use_png
LINKER = g++ -o
LFLAGS = -Wall -lX11 -lpthread -lpng -fopenmp

# Declaration of variables
SRCDIR = src
OBJDIR = build
EXEDIR = .

debug: CPP_FLAGS += -g
# -fsanitize=undefined -fno-omit-frame-pointer
#debug: LFLAGS += -fsanitize=undefined
debug: $(EXE)

release: CPP_FLAGS += -O2
release: $(EXE)

# COREM main executable file 
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
#INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Main target
$(EXEDIR)/$(EXE): $(OBJECTS)
	$(LINKER) $@ $(OBJECTS) $(LFLAGS)

# To obtain object files which use header file
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	mkdir -p $(OBJDIR)
	$(CPP) -c $< -o $@ $(CPP_FLAGS)

# To obtain object files which do not use header
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CPP) -c $< -o $@ $(CPP_FLAGS)

# To remove generated temporary files
.PHONY: clean
clean:
	rm $(OBJECTS)
