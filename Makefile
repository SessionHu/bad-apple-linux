KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
PWD := $(shell pwd)

obj-m := ba_module.o

all: ba.h
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	mkdir -p /lib/modules/$(KVER)/kernel/extra/
	install -m 0644 ba_module.ko /lib/modules/$(KVER)/kernel/extra/
	depmod -a
	echo ba_module > /etc/modules-load.d/ba_module.conf

ba.h: ba.asc
	xxd -i ba.asc ba.h
