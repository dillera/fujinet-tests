PLATFORMS := apple2 c64 coco atari
ALL_TARGETS := $(addprefix cmdtest.,$(PLATFORMS))

-include common.mk

$(info PLATFORMS: $(PLATFORMS))
$(info ALL_TARGETS: $(ALL_TARGETS))
$(info CFILES: $(CFILES))

all: $(ALL_TARGETS)

cmdtest.%: $(CFILES)
	$(MAKE) -f $*.mk

clean:
	rm -rf *.o *.lst *.map *.bin *_obj $(ALL_TARGETS)
