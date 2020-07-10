# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

ARDUINO = /usr/share/arduino
ARDUINO_DIR = $(ARDUINO)
ARDMK_VENDOR = archlinux-arduino

DISTRO_ID := $(shell grep -i "^ID=" /etc/os-release | cut -d"=" -f 2 2>/dev/null)

# Check if using archlinux
ifeq ($(DISTRO_ID), arch)
	ARDMK_VENDOR = archlinux-arduino
else
	ARDMK_VENDOR = arduino
endif
BOARD_TAG = uno

ARDUINO_LIBS = Bounce2 SPI Ethernet

include $(ARDUINO)/Arduino.mk