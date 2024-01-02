all : testtop

testtop : testtop.c
	gcc -o $@ $^ -I../../rv003usb/ch32v003fun/minichlink -ludev -lm

clean :
	rm -rf *.o *~ testtop ledtest

