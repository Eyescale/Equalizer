
SUBARCH = $(shell uname -m)

# The differences in compiler flags between 64 and 32 bit x86 machines
# warrant two sub-Makefiles
include $(TOP)/make/$(ARCH).$(SUBARCH).mk
