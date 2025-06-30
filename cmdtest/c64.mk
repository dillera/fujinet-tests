PLATFORM=c64
-include common.mk

all: $(TARGET).$(PLATFORM)

$(TARGET).$(PLATFORM): $(OBJS)
	$(link-bin)

-include cc65.common.mk
