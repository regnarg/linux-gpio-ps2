# my optional module name
MODULE=kps2
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

 
# this two variables, depends where you have you raspberry kernel source and tools installed
 
KERNEL_SRC=/usr/src/linux-3.6.11-12-ARCH+/
 
 
obj-m += ${MODULE}.o
 
module_upload=${MODULE}.ko
 
all: ${MODULE}.ko
 
${MODULE}.ko: ${MODULE}.c
	make -C ${KERNEL_SRC} M=$(PWD) CFLAGS=-std=c99  modules
 
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean
 
info:
	modinfo  ${module_upload}
