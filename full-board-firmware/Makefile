all : flash

TARGET:=usbfirmware
CH32V003FUN=../rv003usb/ch32v003fun/ch32v003fun
#MINICHLINK?=../ch32v003fun/minichlink
ADDITIONAL_C_FILES+=../rv003usb/rv003usb/rv003usb.S ../rv003usb/rv003usb/rv003usb.c
EXTRA_CFLAGS:=-I../rv003usb/lib -I../rv003usb/rv003usb

include ../rv003usb/ch32v003fun/ch32v003fun/ch32v003fun.mk


install_udev_rules : 
	sudo cp 99-oledscope.rules /etc/udev/rules.d/
	sudo udevadm control --reload
	sudo udevadm trigger

flash : cv_flash
clean : cv_clean


