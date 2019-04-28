CONTIKI_PROJECT = benchmark_app
# If the target is Cooja, choose the separate Makefile.cooja
ifeq ($(TARGET),preon32)
include $(CURDIR)/Makefile.preon
else
ifeq ($(TARGET),preon32l4)
include $(CURDIR)/Makefile.preon
else
include $(CURDIR)/Makefile.cooja
CFLAGS += -std=gnu99
endif
endif
