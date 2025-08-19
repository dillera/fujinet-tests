PLATFORM=coco
-include defs.mk

all: $(TARGET).$(PLATFORM) $(TARGET).dsk

$(TARGET).$(PLATFORM): $(OBJS)
	$(link-bin)

$(OBJDIR)/main.o: $(FNLIB_LIBS)/libfujinet.$(PLATFORM).a

$(TARGET).dsk: $(TARGET).$(PLATFORM)
	$(RM) $@
	decb dskini $@
	decb copy -t -0 autoexec.bas $@,AUTOEXEC.BAS
	decb copy -b -2 $< $@,$(shell echo $(TARGET) | tr '[:lower:]' '[:upper:]').BIN

-include cmoc.common.mk
-include post.mk
