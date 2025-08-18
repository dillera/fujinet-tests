CC=cmoc
AS=lwasm
CFLAGS=-I$(FNLIB_INCLUDE)
AFLAGS=
LIBS=-L $(FNLIB_LIBS) -lfujinet.$(PLATFORM)

define link-bin
  $(CC) -o $@ $(LDFLAGS) $^ $(LIBS) 2>&1 | sed -e 's/'$$'\033''[[][0-9][0-9]*m//g'
endef

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $< 2>&1 | sed -e 's/'$$'\033''[[][0-9][0-9]*m//g'


$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) -c $(AFLAGS) -o $@ $< 2>&1 | sed -e 's/'$$'\033''[[][0-9][0-9]*m//g'
