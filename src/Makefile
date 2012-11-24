# makefile for libpng using gcc (generic, static library)
# Copyright (C) 2000 Cosmin Truta
# Copyright (C) 1995 Guy Eric Schalnat, Group 42, Inc.
# For conditions of distribution and use, see copyright notice in png.h

# Compiler, linker, lib and other tools
CC = g++
LD = $(CC)
AR = ar rcs
RANLIB = ranlib
RM = rm -fr

CDEBUG = -g -DPNG_DEBUG=5 -D_SET_DEBUG_
LDDEBUG =
CRELEASE = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -g #-O2
LDRELEASE = #-s
CFLAGS = -Wall -Werror -Wno-unused $(CRELEASE)
CXXFLAGS = -Wall -Werror -Wno-unused $(CRELEASE)
LDFLAGS = -L. -lz -lm $(LDRELEASE)
TARGET = ../bin/

# File extensions
O=.o
A=.a
E=

# Targets
all: CreateSimpleTable1$(E) CreateSimpleTable2$(E) PYTHONLIB

CreateSimpleTable1$(E): CreateSimpleTable1$(O) SimpleRoutingTable2$(O) Address$(O) VMEM$(O) VMEMstd$(O) MyUtil$(O) BufFile$(O)
	$(LD) -o $(TARGET)$@ CreateSimpleTable1$(O) SimpleRoutingTable2$(O) Address$(O) VMEM$(O) VMEMstd$(O) MyUtil$(O) BufFile$(O) $(LDFLAGS)

CreateSimpleTable2$(E): CreateSimpleTable2$(O) SimpleRoutingTable2$(O) VMEM$(O) VMEMstd$(O) BGPUpdate$(O) Address$(O) MyUtil$(O) BufFile$(O) DateTime$(O)
	$(LD) -o $(TARGET)$@ CreateSimpleTable2$(O) SimpleRoutingTable2$(O) VMEM$(O) VMEMstd$(O) BGPUpdate$(O) Address$(O) MyUtil$(O) BufFile$(O) DateTime$(O) $(LDFLAGS)

PYTHONLIB: setup.py PYGetSurePath$(O) SimpleRoutingTable2$(O) Address$(O) VMEM$(O) VMEMstd$(O) MyUtil$(O)
	python setup.py build
	cp build/lib.*/bgp.so ../script/

clean:
	$(RM) *$(O) *.core build
