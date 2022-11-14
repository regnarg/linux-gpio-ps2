#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
 
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include <linux/serio.h>
 
#define DRIVER_AUTHOR "Filip Stedronsky <p@regnarg.cz>, Jiri Kalvoda <jirikalvoda@kam.mff.cuni.cz>"
#define DRIVER_DESC   "Bit-banged read-only PS2 driver using GPIO."
#define MODNAME "gpio-ps2-serio"
#define LOGPREFIX MODNAME ": "

// Inspired by GPIO interrupt example by Igor <hardware.coder@gmail.com>
// from <http://morethanuser.blogspot.cz/2013/04/raspbery-pi-gpio-interrupts-in-kernel.html>
 

#define PS2_PKTSIZE 11 //bits

static int gpio_clk = -1;
module_param_named(gpio_clk, gpio_clk, int, 0);
MODULE_PARM_DESC(gpio_clk, "GPIO number of the clock line");
static int gpio_data = -1;
module_param_named(gpio_data, gpio_data, int, 0);
MODULE_PARM_DESC(gpio_data, "GPIO number of the data line");

static void *devid = "fkgslkensdlbknsd";// pointer used as unique cookie for register_irq
static struct serio *gpio_ps2_serio;
 
/****************************************************************************/
/* Interrupts variables block                                               */
/****************************************************************************/
static short int irq_clk    = 0;
 
 
/****************************************************************************/
/* IRQ handler - fired on interrupt                                         */
/****************************************************************************/
static irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
 
    // unsigned long flags;
    //
    // // disable hard interrupts (remember them in flag 'flags')
    // //local_irq_save(flags);
    
    // NOTE:
    // Anonymous Sep 17, 2013, 3:16:00 PM:
    // You are putting printk while interupt are disabled. printk can block.
    // It's not a good practice.
    // 
    // hardware.coder:
    // http://stackoverflow.com/questions/8738951/printk-inside-an-interrupt-handler-is-it-really-that-bad
    
    //printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered !.\n",
    //        irq, (char *) dev_id);

    char data[PS2_PKTSIZE];
    int iter[PS2_PKTSIZE];
    int num= 0;
    int last_clk = 1;
	int last_falling_edge_i = 0;
	int i;
    for (i = 0; i < 5000 + last_falling_edge_i; i++) {
        int clk = gpio_get_value(gpio_clk);
        if (last_clk == 1 && clk == 0) {
            // falling edge
			last_falling_edge_i = i;
            data[num] = gpio_get_value(gpio_data);
            iter[num] = i;
            num++;
            if (num >= PS2_PKTSIZE) break;
        }
        last_clk = clk;
    }

    printk(KERN_NOTICE LOGPREFIX "wire: ");
    for (int i = 0; i< num; i++) {
        printk(KERN_NOTICE "%d", (int)data[i]);
    }
    printk(KERN_NOTICE "\n");

    if (num < PS2_PKTSIZE) {
        printk(KERN_NOTICE LOGPREFIX "Received incomplete packet in %d iterations.\n", i);
        goto exit;
    }
    
    int parity = 1; // odd parity
    unsigned char scancode = 0;
    for (int i = 8; i >= 1; i--) {
        scancode <<= 1;
        scancode  |= data[i]; 
        parity ^= data[i];
    }

    int serio_err = 0;
    if (data[9] != parity) {
        printk(KERN_NOTICE LOGPREFIX "Parity mismatch.\n");
        serio_err = SERIO_PARITY;
    }
    if (data[0] != 0)  {
        printk(KERN_NOTICE LOGPREFIX "Start bit mismatch.\n");
        serio_err = SERIO_FRAME;
    }
    if (data[10] != 1) {
        printk(KERN_NOTICE LOGPREFIX "Stop bit mismatch.\n");
        serio_err = SERIO_FRAME;
    }

    printk(KERN_NOTICE LOGPREFIX "Got scancode: 0x%x in %d iterations.\n", (int)scancode, i);
    serio_interrupt(gpio_ps2_serio, scancode, serio_err);
 
exit:
    // // restore hard interrupts
	//local_irq_restore(flags);
 
   return IRQ_HANDLED;
}
 
 
/****************************************************************************/
/* Module init / cleanup block.                                             */
/****************************************************************************/
int gpio_ps2_serio_init(void) {
    int err;
    if (gpio_clk == -1 || gpio_data == -1) {
        printk(KERN_ERR LOGPREFIX "The parameters 'gpio_clk' and 'gpio_data' are required.");
        return -EINVAL;
    }
 
    printk(KERN_NOTICE LOGPREFIX "initializing\n");
    for (int i = 0; i < 2; i++) {
        int gpio;
        const char *desc;
        switch(i) {
            case 0: gpio = gpio_clk;  desc = MODNAME " clk"; break;
            case 1: gpio = gpio_data; desc = MODNAME " data"; break;
        }
        if ((err = gpio_request(gpio, desc))) {
            printk("GPIO request faiure: %d\n", gpio);
            return err;
        }
        if ((err = gpio_direction_input(gpio))) {
            printk("GPIO direction faiure: %d\n", gpio);
            return err;
        }
    }

    
    if ( (irq_clk = gpio_to_irq(gpio_clk)) < 0 ) {
        printk("GPIO to IRQ mapping faiure %d\n", gpio_clk);
        return irq_clk;
    }
    
    printk(KERN_NOTICE "Mapped int %d\n", irq_clk);

    gpio_ps2_serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
    if (!gpio_ps2_serio)
        return -ENOMEM;

    gpio_ps2_serio->id.type = SERIO_8042;
    strlcpy(gpio_ps2_serio->name, "GPIO PS/2 to serio wrapper",
            sizeof(gpio_ps2_serio->name));
    strlcpy(gpio_ps2_serio->phys, "GPIO/serio0",
            sizeof(gpio_ps2_serio->phys));

    serio_register_port(gpio_ps2_serio);
    dev_info(&gpio_ps2_serio->dev, "%s\n", gpio_ps2_serio->name);
    
    if ((err = request_irq(irq_clk,
                    (irq_handler_t ) irq_handler,
                    IRQF_TRIGGER_FALLING,
                    MODNAME " clk interrupt",
                    devid))) {
        printk("Irq Request failure\n");
        return err;
    }
 
    return 0;
}
 
void gpio_ps2_serio_cleanup(void) {
    printk(KERN_NOTICE "Goodbye\n");
    serio_unregister_port(gpio_ps2_serio);
    free_irq(irq_clk, devid);
    gpio_free(gpio_clk);
    gpio_free(gpio_data);
}
 
 
module_init(gpio_ps2_serio_init);
module_exit(gpio_ps2_serio_cleanup);
 
 
/****************************************************************************/
/* Module licensing/description block.                                      */
/****************************************************************************/
MODULE_LICENSE("GPL");
