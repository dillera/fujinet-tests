TARGET=rmttest
UNIFIED=1
ifdef UNIFIED
FUJINET_LIB_DIR=../../fujinet-lib-unified
FNLIB_INCLUDE=-I$(FUJINET_LIB_DIR)/include
else
FUJINET_LIB_DIR=../../fujinet-lib
FNLIB_INCLUDE=-I$(FUJINET_LIB_DIR) -I$(FUJINET_LIB_DIR)/coco/src/include
endif
FNLIB_LIBS=$(FUJINET_LIB_DIR)/build

CFILES= main.c diskcmd.c filecmd.c hexdump.c
HFILES= command.h deviceid.h diskcmd.h filecmd.h
AFILES= 
OBJDIR := $(PLATFORM)_obj
OBJS = $(addprefix $(OBJDIR)/, $(CFILES:.c=.o) $(AFILES:.s=.o))

# Find sources in the current dir
vpath %.c .
vpath %.s .
