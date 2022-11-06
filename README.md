
# This has been superseded by a driver included in the kernel, `drivers/input/serio/ps2-gpio.c`

---

# PS/2 over GPIO for Linux

A simple kernel module that allows connecting a PS/2 keyboard using GPIO
(one input for data and one for clock).

This can be used e.g. to connect and old PS/2 keyboard to a Raspberry PI
or a similar SBC and thus save one USB port.

The implementation is a bit quick-and-dirty so you probably shouldn't
use it in production.

# Installation guide

```
make
make install
```

Create module configuration file `modprobe.d/linux-gpio-ps2.conf` (fill your pin numbers in GPIO/BCM pin numbering):
```
options gpio-ps2-serio gpio_clk=17
options gpio-ps2-serio gpio_data=27
```

Load the module automatically:
```
echo gpio-ps2-serio > modules-load.d/gpio-ps2.conf
```

Then reboot.
