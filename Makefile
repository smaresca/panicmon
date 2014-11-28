#Copyright (c) 2009, Steve Maresca/Zentific LLC 

obj-m += panicmon.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	#rm Module.symvers 2>&1 > /dev/null
