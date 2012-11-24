BUILD_SUBDIRS = src
SCRIPT_SUBDIR = script

# build
#
all: $(BUILD_SUBDIRS)

.PHONY: $(BUILD_SUBDIRS)

$(BUILD_SUBDIRS):
	@cd $@ && $(MAKE)


run: $(SCRIPT_SUBDIR)

.PHONY: $(SCRIPT_SUBDIR)

$(SCRIPT_SUBDIR):
	@cd $@ && $(MAKE)

# clean
#
CLEAN_SUBDIRS = $(BUILD_SUBDIRS:%=%-clean)

.PHONY: $(CLEAN_SUBDIRS)

$(CLEAN_SUBDIRS):
	@cd $(@:%-clean=%) && $(MAKE) clean

clean: $(CLEAN_SUBDIRS)
	rm -fr data tmp log/* tables/*
