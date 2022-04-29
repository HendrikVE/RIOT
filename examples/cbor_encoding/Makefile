# name of your application
APPLICATION = cbor-encoding

COMPILE_BENCHMARK ?= 1
ifeq (1, $(COMPILE_BENCHMARK))
  DEVELHELP := 0
endif

COMPILE_ENCODER ?= 1
COMPILE_FULL_PARSER ?= 1
COMPILE_PARTIAL_PARSER ?= 1

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEPKG += tinycbor

CFLAGS += -DBENCHMARK_BUILD=$(COMPILE_BENCHMARK)

CFLAGS += -DCONFIG_COMPILE_ENCODER=$(COMPILE_ENCODER)
CFLAGS += -DCONFIG_COMPILE_FULL_PARSER=$(COMPILE_FULL_PARSER)
CFLAGS += -DCONFIG_COMPILE_PARTIAL_PARSER=$(COMPILE_PARTIAL_PARSER)

include $(RIOTBASE)/Makefile.include
