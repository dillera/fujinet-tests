PLATFORM=coco
-include defs.mk

all: $(TARGET).$(PLATFORM)

$(TARGET).$(PLATFORM): $(OBJS)
	$(link-bin)

-include cmoc.common.mk
-include post.mk
