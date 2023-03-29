include ../Makefile.tests_common

FEATURES_REQUIRED += periph_can

BOARD = same54-xpro

USEMODULE += can
USEMODULE += ztimer_sec
USEMODULE += ztimer_msec

CFLAGS += -DISR_STACKSIZE="(1024)"
CFLAGS += -DCANDEV_SAMD5X_DEFAULT_TX_EVT_FIFO_ELTS_NUM=5
CFLAGS += -DCANDEV_SAMD5X_DEFAULT_TX_BUFFER_FIFO_QUEUE_NUM=2
CFLAGS += -DTEST_MODE=0

# Todo: remove
CFLAGS += \
  -Wno-error=unused-function \

DEVELHELP ?= 1

include $(RIOTBASE)/Makefile.include