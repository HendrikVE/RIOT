# name of your application
APPLICATION = demo-lndw

# If no BOARD is found in the environment, use this default:
BOARD ?= esp32-wroom-32

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEMODULE += ztimer
USEMODULE += ztimer_msec
USEMODULE += mfrc522
USEMODULE += pca9633
USEMODULE += aip31068

include $(RIOTBASE)/Makefile.include
