CLANG_FORMAT ?= clang-format
SOURCES := $(shell git ls-files -co --exclude-standard '*.c' '*.h')

.PHONY: format format-check

format:
	$(CLANG_FORMAT) -i $(SOURCES)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES)
