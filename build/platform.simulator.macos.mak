TOOLCHAIN ?= apple
EXE = bin

APPLE_PLATFORM = macos
APPLE_PLATFORM_MIN_VERSION = 10.10
EPSILON_TELEMETRY ?= 0

ifeq ($(DEBUG),1)
ARCHS = $(shell uname -m)
else
ARCHS = arm64 x86_64
endif

ifdef ARCH
BUILD_DIR := $(BUILD_DIR)/$(ARCH)
else
HANDY_TARGETS_EXTENSIONS = app
endif
