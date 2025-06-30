PLATFORM=coco
-include common.mk

all: $(TARGET).$(PLATFORM)

$(TARGET).$(PLATFORM): $(OBJS)
	$(link-bin)

-include cmoc.common.mk
