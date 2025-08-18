TARGET=rmttest
FUJINET_LIB_DIR=../../fujinet-lib-unified
FNLIB_INCLUDE=$(FUJINET_LIB_DIR)/include
FNLIB_LIBS=$(FUJINET_LIB_DIR)/build

CFILES= main.c diskcmd.c filecmd.c
HFILES= command.h deviceid.h diskcmd.h filecmd.h
AFILES= 
OBJDIR := $(PLATFORM)_obj
OBJS = $(addprefix $(OBJDIR)/, $(CFILES:.c=.o) $(AFILES:.s=.o))

# Find sources in the current dir
vpath %.c .
vpath %.s .
