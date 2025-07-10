TARGET=cmdtest

CFILES= main.c machineid.c
AFILES= 
OBJDIR := $(PLATFORM)_obj
OBJS = $(addprefix $(OBJDIR)/, $(CFILES:.c=.o) $(AFILES:.s=.o))

# Find sources in the current dir
vpath %.c .
vpath %.s .
