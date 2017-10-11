# Makefile to build library 'mighty_looper_lib' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = mighty_looper_lib

# source file that invokes setup methods of all classes in the library
lib.setup.sources = src/mighty_looper_lib.c

# input source file (class name == source file basename)
#class.sources = counter.c helloworld.c
class.sources = $(filter-out $(lib.setup.sources),$(wildcard src/*.c))

# build all classes into a single library executable
make-lib-executable=yes

# all extra files to be included in binary distribution of the library
#datafiles = counter-help.pd helloworld-help.pd mighty_looper_lib-meta.pd README.md
datafiles = \
$(wildcard help/*.pd) \
mighty_looper_lib-meta.pd

PDLIBDIR = ..

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

# destination directory for library build
DESTDIR = ./build
