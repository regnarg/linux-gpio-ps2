# my optional module name
MODULE=gpio-ps2-serio
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

 
# this two variables, depends where you have you raspberry kernel source and tools installed
 
#KERNEL_SRC=/usr/src/linux-3.6.11-12-ARCH+/
KERNEL_SRC=/usr/lib/modules/3.12.20-4-ARCH/build/
 
 
obj-m += ${MODULE}.o
 
module_upload=${MODULE}.ko
 
all: ${MODULE}.ko
 
${MODULE}.ko: ${MODULE}.c
	make -C ${KERNEL_SRC} M=$(PWD) modules
 
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean
 
info:
	modinfo  ${module_upload}
