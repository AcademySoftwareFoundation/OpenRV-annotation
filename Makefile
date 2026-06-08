SHELL := bash
.PHONY: all configure build test clean coverage coverage-setup sanitize \
        format format-check install-hooks

BUILD_DIR    := build
COVERAGE_DIR := build-coverage
SANITIZE_DIR := build-sanitize

VENV  := coverage/.venv
GCOVR := $(VENV)/bin/gcovr

# Native build. TwkPaint is a geometry library — the WASM build is handled
# by OpenRV-annotation-wasm, which consumes this repo as a submodule.

all: build

configure:
	cmake -B $(BUILD_DIR) -G Ninja

build: configure
	cmake --build $(BUILD_DIR)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# Create the venv and install coverage tools. Reruns only when
# requirements.txt changes (gcovr binary acts as the sentinel).
coverage-setup: $(GCOVR)

$(GCOVR): coverage/requirements.txt
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --quiet --upgrade pip
	$(VENV)/bin/pip install --quiet -r coverage/requirements.txt

# Build with --coverage, run tests, and generate an HTML report.
coverage: coverage-setup
	cmake -B $(COVERAGE_DIR) -G Ninja -DENABLE_COVERAGE=ON
	cmake --build $(COVERAGE_DIR)
	ctest --test-dir $(COVERAGE_DIR) --output-on-failure
	mkdir -p coverage/report
	$(GCOVR) --config coverage/gcovr.cfg

# Sanitize: builds with AddressSanitizer + UBSanitizer and runs tests.
sanitize:
	cmake -B $(SANITIZE_DIR) -G Ninja -DENABLE_SANITIZERS=ON
	cmake --build $(SANITIZE_DIR)
	ctest --test-dir $(SANITIZE_DIR) --output-on-failure

CPP_SOURCES := $(shell find TwkMath TwkPaint tests -name '*.cpp' -o -name '*.h')

# Format all C++ sources in-place.
format:
	clang-format -i $(CPP_SOURCES)

# Check formatting without modifying files (used in CI and the pre-commit hook).
format-check:
	clang-format --dry-run --Werror $(CPP_SOURCES)

# Symlink the pre-commit hook into .git/hooks so it runs automatically.
install-hooks:
	ln -sf ../../hooks/pre-commit .git/hooks/pre-commit
	@echo "pre-commit hook installed."

clean:
	rm -rf $(BUILD_DIR) $(COVERAGE_DIR) $(SANITIZE_DIR) $(VENV) coverage/report/
