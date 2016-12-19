obj-m+=led.o

all:
	make C=2 -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	gcc -Wall led-test.c -o led-test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
