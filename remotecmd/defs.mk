TARGET=rmttest
FUJINET_LIB_DIR=$(HOME)/source/fujinet/fujinet-lib-unified

CFILES= main.c diskcmd.c filecmd.c
HFILES= command.h deviceid.h diskcmd.h filecmd.h
AFILES= 
OBJDIR := $(PLATFORM)_obj
OBJS = $(addprefix $(OBJDIR)/, $(CFILES:.c=.o) $(AFILES:.s=.o))

# Find sources in the current dir
vpath %.c .
vpath %.s .
