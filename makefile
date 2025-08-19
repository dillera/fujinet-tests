DESTDIR=../unified/coco/build/dist/data
AUTORUN=autorun.dsk

coco-lwm: $(DESTDIR)/$(AUTORUN)

.PHONY: $(DESTDIR)/$(AUTORUN)

$(DESTDIR)/$(AUTORUN):
	cd .. ; defoogi make -C $(CURDIR)/../fujinet-lib-unified -f coco.mk
	cd .. ; defoogi make -C $(CURDIR)/remotecmd -f coco.mk
	cp remotecmd/rmttest.dsk $@

all:
	make -C cmdtest
	make -C remotecmd
