DESTDIR=../unified/coco/build/dist/data
AUTORUN=autorun.dsk

define defoogi-make
  cd .. ; defoogi make -C $(CURDIR)/../fujinet-lib-unified -f $@.mk
  cd .. ; defoogi make -C $(CURDIR)/remotecmd -f $@.mk
endef

apple2:
	$(defoogi-make)
	rsync -P remotecmd/rmttest.po ayce:Sites/loderunner/paumaed/disks/rmttest.po

c64:
	$(defoogi-make)
	rsync -P remotecmd/rmttest.c64 ayce:Sites/loderunner/paumaed/public/rmttest.prg

coco-lwm: $(DESTDIR)/$(AUTORUN)

.PHONY: $(DESTDIR)/$(AUTORUN)

$(DESTDIR)/$(AUTORUN):
	cd .. ; defoogi make -C $(CURDIR)/../fujinet-lib-unified -f coco.mk
	cd .. ; defoogi make -C $(CURDIR)/remotecmd -f coco.mk
	cp remotecmd/rmttest.dsk $@

all:
	make -C cmdtest
	make -C remotecmd
