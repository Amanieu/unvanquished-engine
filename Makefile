#########################
# Unvanquished Makefile #
#########################

#############################################################################
# SETUP
#############################################################################

COMPILE_PLATFORM = $(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

COMPILE_ARCH = $(shell uname -m | sed -e s/i.86/x86/)

ifeq ($(COMPILE_PLATFORM), darwin)
  COMPILE_ARCH = $(shell uname -p | sed -e s/i.86/x86/)
else ifeq ($(COMPILE_PLATFORM), windowsnt)
  COMPILE_PLATFORM = mingw32
endif

ifeq ($(COMPILE_ARCH), powerpc)
  COMPILE_ARCH = ppc
else ifeq ($(COMPILE_ARCH), powerpc64)
  COMPILE_ARCH = ppc64
else ifeq ($(COMPILE_ARCH), amd64)
  COMPILE_ARCH = x86_64
endif

# User configuration
-include Makefile.local

# Platform and architecture to build for, change these for cross-compiling
export PLATFORM ?= $(COMPILE_PLATFORM)
export ARCH ?= $(COMPILE_ARCH)

# Debug build
export DEBUG ?= 0
ifeq ($(DEBUG), 1)
  export BUILD_DIR = build/debug-$(PLATFORM)-$(ARCH)
else
  export BUILD_DIR = build/release-$(PLATFORM)-$(ARCH)
endif

#############################################################################
# TARGETS
#############################################################################

# You can set the default targets in Makefile.local
TARGETS ?= help

default: $(TARGETS)

help:
	@echo "Pseudo-targets:"
	@echo "  all       - Build all targets"
	@echo "  clean     - Remove build files"
	@echo
	@echo "Targets:"
	@echo "  engine    - Engine binary"
	@echo "  game      - Game module"
	@echo "  server    - Dedicated server"
	@echo "  editor    - Editor"
	@echo "  test      - Unit tests"

all: engine game server editor

clean:
	@rm -rf build

engine:
	@echo ====================
	@echo Building engine binary
	@echo ====================
	@$(MAKE) -f Makefile.sub TARGET=engine

game:
	@echo ====================
	@echo Building game module
	@echo ====================
	@$(MAKE) -f Makefile.sub TARGET=game

server:
	@echo =========================
	@echo Building dedicated server
	@echo =========================
	@$(MAKE) -f Makefile.sub TARGET=server

editor:
	@echo ===============
	@echo Building editor
	@echo ===============
	@$(MAKE) -f Makefile.sub TARGET=editor

test:
	@echo ===================
	@echo Building unit tests
	@echo ===================
	@$(MAKE) -f Makefile.sub TARGET=test

.PHONY: default help all clean distclean engine game server editor test
