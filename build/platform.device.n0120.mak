PCB_LATEST = 418 # PCB version 4.18
SIGNATURE_INDEX = 1
TOOLCHAIN ?= arm-gcc-m7h

ifeq ($(DEVELOPMENT),0)
ifeq ($(IN_FACTORY),0)
EMBED_EXTRA_DATA ?= 1
endif
endif
