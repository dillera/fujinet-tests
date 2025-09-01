PLATFORMS := apple2
-include defs.mk

ALL_TARGETS := $(addprefix $(TARGET).,$(PLATFORMS))

$(info PLATFORMS: $(PLATFORMS))
$(info ALL_TARGETS: $(ALL_TARGETS))
$(info CFILES: $(CFILES))

all: $(ALL_TARGETS)


$(TARGET).%: $(CFILES)
	$(MAKE) -f $*.mk


SUFFIX := .a2s
DISK_TASKS += .po
AUTOBOOT := -l
APPLE_TOOLS_DIR := ../tools/apple-tools

DIST_DIR := .
PROGRAM_TGT := rmttest
PROGRAM := rmttest


disk:
	$(APPLE_TOOLS_DIR)/mk-bitsy.sh $(DIST_DIR)/$(PROGRAM_TGT).po $(PROGRAM_TGT)$(SUFFIX)
	$(APPLE_TOOLS_DIR)/add-file.sh $(AUTOBOOT) $(DIST_DIR)/$(PROGRAM_TGT).po $(DIST_DIR)/$(PROGRAM_TGT)$(SUFFIX) $(PROGRAM)



clean:
	rm -rf *.o *.lst *.map *.bin *_obj $(ALL_TARGETS)
