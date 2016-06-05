# $Id: GNUmakefile,v 1.28.2.7 2009/05/22 14:22:48 gandzyuk Exp $

include ../../LinuxMakefile.defines

PROJECT_ROOT = ./../..

LOCAL_CPP_FL = -DXP_UNIX

OBJ = boxtime.o \
 condition.o \
 file.o \
 ipaddress.o \
 mutex.o \
 refcounted.o \
 semaphorp.o \
 socket.o \
 system_exception.o \
 task.o \
 tcpclient.o \
 tcpserver.o \
 tcpsocket.o \
 thread.o \
 timer.o \
 useful.o


SRC = boxtime.cpp \
 condition.cpp \
 file.cpp \
 ipaddress.cpp \
 mutex.cpp \
 refcounted.cpp \
 semaphorp.cpp \
 socket.cpp \
 system_exception.cpp \
 task.cpp \
 tcpclient.cpp \
 tcpserver.cpp \
 tcpsocket.cpp \
 thread.cpp \
 timer.cpp \
 useful.cpp

LOCAL_INCLUDE_PATH = -I. -I../include


MAIN = ../lib/libcommonlib.a
MAIN_D = ../lib/libcommonlib_d.a

release: $(MAIN)
debug: $(MAIN_D)


$(MAIN) $(MAIN_D): $(OBJ)
	ar -rc $@ $(OBJ)
	ar -s $@
