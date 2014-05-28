#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
 
#include <linux/interrupt.h>
#include <linux/gpio.h>
 
 
#define DRIVER_AUTHOR "Igor <hardware.coder@gmail.com>"
#define DRIVER_DESC   "Tnterrupt Test"
 
// we want GPIO_17 (pin 11 on P5 pinout raspberry pi rev. 2 board)
// to generate interrupt
#define GPIO_CLOCK                17
#define GPIO_DATA                 4
 
// text below will be seen in 'cat /proc/interrupt' command
 
 
 
/****************************************************************************/
/* Interrupts variables block                                               */
/****************************************************************************/
short int irq_clock    = 0;
 
 
/****************************************************************************/
/* IRQ handler - fired on interrupt                                         */
/****************************************************************************/
static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
 
    unsigned long flags;
    
    // disable hard interrupts (remember them in flag 'flags')
    local_irq_save(flags);
    
    // NOTE:
    // Anonymous Sep 17, 2013, 3:16:00 PM:
    // You are putting printk while interupt are disabled. printk can block.
    // It's not a good practice.
    // 
    // hardware.coder:
    // http://stackoverflow.com/questions/8738951/printk-inside-an-interrupt-handler-is-it-really-that-bad
    
    //printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered !.\n",
    //        irq, (char *) dev_id);

    const int MAXN = 11;
    char data[MAXN];
    int iter[MAXN];
    int num= 0;
    int last_clk = 1;
    for (int i = 0; i < 10000000; i++) {
        int clk = gpio_get_value(GPIO_CLOCK);
        if (last_clk == 1 && clk == 0) {
            // falling edge
            data[num] = gpio_get_value(GPIO_DATA);
            iter[num] = i;
            num++;
            if (num >= MAXN) break;
        }
        last_clk = clk;
    }

    printk(KERN_NOTICE "kps2: wire: ");
    for (int i = 0; i< num; i++) {
        printk(KERN_NOTICE "%d", (int)data[i]);
    }
    printk(KERN_NOTICE "\n");

    if (num < MAXN) {
        printk(KERN_NOTICE "kps2: Received incomplete packet.\n");
        goto exit;
    }
    
    int parity = 1; // odd parity
    unsigned char code = 0;
    for (int i = 8; i >= 1; i--) {
        code <<= 1;
        code  |= data[i]; 
        parity ^= data[i];
    }

    if (data[9] != parity) {
        printk(KERN_NOTICE "kps2: Parity mismatch.\n");
        goto exit;
    }
    if (data[0] != 0)  {
        printk(KERN_NOTICE "kps2: Start bit mismatch.\n");
        goto exit;
    }
    if (data[10] != 1) {
        printk(KERN_NOTICE "kps2: Stop bit mismatch.\n");
        goto exit;
    }

    printk(KERN_NOTICE "kps2: Got scancode: 0x%x\n", (int)code);

 
   // restore hard interrupts
exit: local_irq_restore(flags);
 
   return IRQ_HANDLED;
}
 
 
/****************************************************************************/
/* This function configures interrupts.                                     */
/****************************************************************************/
void r_int_config(void) {
 
    for (int i = 0; i < 2; i++) {
        int gpio;
        const char *desc;
        switch(i) {
            case 0: gpio = GPIO_CLOCK; desc="kps2 clock"; break;
            case 1: gpio = GPIO_DATA; desc="kps2 data"; break;
        }
        if (gpio_request(gpio, desc)) {
            printk("GPIO request faiure: %d\n", gpio);
            return;
        }
        if (gpio_direction_input(gpio)) {
            printk("GPIO direction faiure: %d\n", gpio);
            return;
        }
    }

    
    if ( (irq_clock = gpio_to_irq(GPIO_CLOCK)) < 0 ) {
        printk("GPIO to IRQ mapping faiure %d\n", GPIO_CLOCK);
        return;
    }
    
    printk(KERN_NOTICE "Mapped int %d\n", irq_clock);
    
    if (request_irq(irq_clock,
                    (irq_handler_t ) r_irq_handler,
                    IRQF_TRIGGER_FALLING,
                    "kps2 clock interrupt",
                    "kps2 clock interrupt")) {
        printk("Irq Request failure\n");
        return;
    }
    
    return;
}
 
 
/****************************************************************************/
/* This function releases interrupts.                                       */
/****************************************************************************/
void r_int_release(void) {
 
   free_irq(irq_clock, "kps2 clock irq");
   gpio_free(GPIO_CLOCK);
   gpio_free(GPIO_DATA);
 
   return;
}
 
 
/****************************************************************************/
/* Module init / cleanup block.                                             */
/****************************************************************************/
int r_init(void) {
 
   printk(KERN_NOTICE "Hello !\n");
   r_int_config();
 
   return 0;
}
 
void r_cleanup(void) {
   printk(KERN_NOTICE "Goodbye\n");
   r_int_release();
 
   return;
}
 
 
module_init(r_init);
module_exit(r_cleanup);
 
 
/****************************************************************************/
/* Module licensing/description block.                                      */
/****************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
