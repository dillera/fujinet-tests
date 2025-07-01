PLATFORM=apple2
-include common.mk

define single-to-double
  unsingle $< && mv $<.ad $@ && mv .AppleDouble/$<.ad .AppleDouble/$@
endef

all: $(TARGET).$(PLATFORM)

$(TARGET).$(PLATFORM): $(TARGET).bin
	$(single-to-double)
$(TARGET).bin: $(OBJS)
	echo OBJS: $(OBJS)
	$(link-bin)

-include cc65.common.mk
