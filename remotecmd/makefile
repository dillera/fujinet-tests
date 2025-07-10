PLATFORMS := apple2 c64 coco atari
-include defs.mk

ALL_TARGETS := $(addprefix $(TARGET).,$(PLATFORMS))

$(info PLATFORMS: $(PLATFORMS))
$(info ALL_TARGETS: $(ALL_TARGETS))
$(info CFILES: $(CFILES))

all: $(ALL_TARGETS)

$(TARGET).%: $(CFILES)
	$(MAKE) -f $*.mk

clean:
	rm -rf *.o *.lst *.map *.bin *_obj $(ALL_TARGETS)
