PROJECT_ROOT = ../..

include $(PROJECT_ROOT)/LinuxMakefile.defines

OBJ = dispatcher.o \
      fileserver.o \
      server_parser.o \
      server_tasks.o

SRC = dispatcher.cpp \
      fileserver.cpp \
      server_parser.cpp \
      server_tasks.cpp

LIBS = -lpthread

LOCAL_INCLUDE_PATH = \
	-I. \
	-I../include \
	-I$(PROJECT_ROOT)/commonlib/include

MAIN = ../bin/fileserver
MAIN_D = ../bin/fileserver_d

LOCAL_CPP_FL += -I$(PROJECT_ROOT)/commonlib/lib 

release: LOCAL_LIBS =  \
  $(PROJECT_ROOT)/commonlib/lib/libcommonlib.a

release: $(MAIN)

debug: LOCAL_LIBS =  \
  $(PROJECT_ROOT)/commonlib/lib/libcommonlib_d.a

debug: $(MAIN_D)

$(MAIN) $(MAIN_D): $(OBJ)
	$(C++) $(CPP_FL) -o $@ $(OBJ) $(LOCAL_LIBS) $(LIBS)
