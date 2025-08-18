CC=cl65
CFLAGS=-O --cpu 6502 -I$(FNLIB_INCLUDE)
AFLAGS=--cpu 6502
LIBS=-L $(FNLIB_LIBS) fujinet.$(PLATFORM).lib

define link-bin
  $(CC) -vm -t $(PLATFORM) $(LDFLAGS) $^ $(LIBS) -o $@
endef

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -l $(basename $@).lst -c $(CFLAGS) -t $(PLATFORM) -o $@ $<

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) -l $(basename $@).lst -c $(AFLAGS) -t $(PLATFORM) -o $@ $<
