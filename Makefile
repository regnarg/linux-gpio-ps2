# my optional module name
MODULE=gpio-ps2-serio
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

 
# this two variables, depends where you have you raspberry kernel source and tools installed
 
#KERNEL_SRC=/usr/src/linux-3.6.11-12-ARCH+/
#KERNEL_SRC=/usr/lib/modules/3.12.20-4-ARCH/build/
KERNEL_SRC=/usr/src/linux-headers-$(shell uname -r)/
 
 
obj-m += ${MODULE}.o
 
module_upload=${MODULE}.ko
 
all: ${MODULE}.ko
 
${MODULE}.ko: ${MODULE}.c
	make -C ${KERNEL_SRC} M=$(PWD) modules

install:
	make INSTALL_MOD_DIR=linux-gpio-ps2 -C ${KERNEL_SRC} M=$(PWD) modules_install
	depmod

.PHONY: install
 
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean
 
info:
	modinfo  ${module_upload}
