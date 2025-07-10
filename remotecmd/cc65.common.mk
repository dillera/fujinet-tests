CC=cl65
# FUJINET_LIB_DIR=$(HOME)/source/fujinet/fujinet-lib
# CFLAGS=-O --cpu 6502 -I$(FUJINET_LIB_DIR)
CFLAGS=-O --cpu 6502 -I$(FUJINET_LIB_DIR)/include
AFLAGS=--cpu 6502
LIBS=-L $(FUJINET_LIB_DIR)/build fujinet.$(PLATFORM).lib

define link-bin
  $(CC) -vm -t $(PLATFORM) $(LDFLAGS) $^ $(LIBS) -o $@
endef

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -l $(basename $@).lst -c $(CFLAGS) -t $(PLATFORM) -o $@ $<

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) -l $(basename $@).lst -c $(AFLAGS) -t $(PLATFORM) -o $@ $<
