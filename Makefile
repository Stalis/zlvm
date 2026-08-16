CLANG_FORMAT ?= xcrun clang-format

.PHONY: format format-check

format:
	find . -type f \( -name '*.c' -o -name '*.h' \) -exec $(CLANG_FORMAT) -i {} +

format-check:
	find . -type f \( -name '*.c' -o -name '*.h' \) \
		-exec $(CLANG_FORMAT) --dry-run --Werror {} +
