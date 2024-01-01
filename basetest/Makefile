all : sintable.h flash

TARGET:=basetest
CH32V003FUN=../rv003usb/ch32v003fun/ch32v003fun

sintable.h : sintablegen
ifeq ($(OS),Windows_NT)
	sintablegen.exe > sintable.h
else
	./sintablegen > sintable.h
endif

sintablegen : sintablegen.c
	gcc -o $@ $^ -lm

include ../rv003usb/ch32v003fun/ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean
	rm -rf sintable.h sintablegen


