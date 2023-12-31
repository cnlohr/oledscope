all : sintable.h flash

TARGET:=hwspitest
CH32V003FUN=../rv003usb/ch32v003fun/ch32v003fun

sintable.h : sintablegen
	./sintablegen > sintable.h

sintablegen : sintablegen.c
	gcc -o $@ $^ -lm

include ../rv003usb/ch32v003fun/ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean
	rm -rf sintable.h sintablegen


