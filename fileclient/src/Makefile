PROJECT_ROOT = ../..

include $(PROJECT_ROOT)/LinuxMakefile.defines

OBJ = fileclient.o \
      mainframe.o \
      user_tasks.o

SRC = fileclient.cpp \
      mainframe.cpp \
      user_tasks.cpp

LIBS = -lpthread

LOCAL_INCLUDE_PATH = \
	-I. \
	-I../include \
	-I$(PROJECT_ROOT)/commonlib/include
	-I$(PROJECT_ROOT)/fileserver/include

MAIN = ../bin/fileclient
MAIN_D = ../bin/fileclient_d

LOCAL_CPP_FL += -I$(PROJECT_ROOT)/commonlib/lib 

release: LOCAL_LIBS =  \
  $(PROJECT_ROOT)/commonlib/lib/libcommonlib.a

release: $(MAIN)

debug: LOCAL_LIBS =  \
  $(PROJECT_ROOT)/commonlib/lib/libcommonlib_d.a

debug: $(MAIN_D)

$(MAIN) $(MAIN_D): $(OBJ)
	$(C++) $(CPP_FL) -o $@ $(OBJ) $(LOCAL_LIBS) $(LIBS)
