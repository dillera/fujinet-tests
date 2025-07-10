PLATFORM=c64
-include defs.mk

all: $(TARGET).$(PLATFORM)

$(TARGET).$(PLATFORM): $(OBJS)
	$(link-bin)

-include cc65.common.mk
-include post.mk
