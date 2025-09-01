#################################################################
# DISK creation



SUFFIX =
DISK_TASKS += .po
AUTOBOOT := -l
APPLE_TOOLS_DIR := __PARENT_RELATIVE_DIR__/apple-tools

.po:
	$(call RMFILES,$(DIST_DIR)/$(PROGRAM_TGT).po)
	$(call RMFILES,$(DIST_DIR)/$(PROGRAM))

# $(PROGRAM) is the name of the app (like weather) - we'll use that name to add with add-file.sh
# $(PROGRAM_TGT) is the name of the current target build (apple2 / apple2enh)
# if the program name is too large it won't autostart!

	$(APPLE_TOOLS_DIR)/mk-bitsy.sh $(DIST_DIR)/$(PROGRAM_TGT).po $(PROGRAM_TGT)$(SUFFIX)
	$(APPLE_TOOLS_DIR)/add-file.sh $(AUTOBOOT) $(DIST_DIR)/$(PROGRAM_TGT).po $(DIST_DIR)/$(PROGRAM_TGT)$(SUFFIX) $(PROGRAM)
