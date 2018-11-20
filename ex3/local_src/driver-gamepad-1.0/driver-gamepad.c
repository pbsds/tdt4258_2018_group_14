/*
some parts of this driver are adapted from the
scull example driver found in LDD (3rd edition)
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "efm32gg.h"

struct cdev gamepad_cdev;

static int gamepad_major = 0;
static uint32_t* MODEL, DIN, DOUT, IF, IFC, EXTIPSELL, EXTIFALL, _ISER0, IEN; // virtual memory addresses
static uint32_t buttons = 0;
struct fasync_struct* async_queue;

// TODO: add more driver info here
MODULE_LICENSE("GPL");

static ssize_t gamepad_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
  printk(KERN_INFO "Reading");
  return copy_to_user(buff, &buttons, sizeof(buttons)) ? -EFAULT : 0; // copies buttons to user. returns error (EFAULT) if it didn't read 
}

static int gamepad_fasync(int fd, struct file *filp, int mode) {
  printk(KERN_INFO "\n%d , %d", fd, mode); // 3, 1 
  return fasync_helper(fd, filp, mode, &async_queue); // registers a process to signal to
}

static int gamepad_release(struct inode *inode, struct file *filp)
{
  //  gamepad_fasync(-1, filp, 0); // remove this filp from queue
  return 0;
}

static struct file_operations gamepad_fops = {
  .owner = THIS_MODULE,
  .read = gamepad_read,
  .fasync = gamepad_fasync,
  .release = gamepad_release,
};

static const struct of_device_id of_gamepad_device_match[] = {
  { .compatible = "tdt4258", },
  { },
};

MODULE_DEVICE_TABLE(of, of_gamepad_device_match);

irqreturn_t gamepad_interrupt(int irq, void* dev_id) {
  printk(KERN_INFO "interrupted");
  // tell GPIO we're handling the interrupt
  buttons = ioread32(IF);
  iowrite32(ioread32(IF), IFC);

  //send SIGIO to any process listening, if any exist
  if (async_queue) {
    printk(KERN_INFO "Sending interrupt");
    kill_fasync(&async_queue, SIGIO, POLL_IN); // POLL_IN means file can be read
  }
  return IRQ_HANDLED;
}

// init code
static int gamepad_probe(struct platform_device *dev) {
  printk(KERN_INFO "hello world");
  
  //assign major number to the gamepad, and allocate a chardev region to it.
  dev_t devno = MKDEV(0, 0); // the assignment to MKDEV isn't necessary, but if we don't, the compiler gives some very ornery warnings
  alloc_chrdev_region(&devno, 0, 1, "gamepad");
  gamepad_major = MAJOR(devno);

  // register the driver
  cdev_init(&gamepad_cdev, &gamepad_fops);
  gamepad_cdev.owner = THIS_MODULE;
  gamepad_cdev.ops = &gamepad_fops;
  int err = cdev_add(&gamepad_cdev, devno, 1);
  /* Fail gracefully if need be */
  if (err) {
    printk(KERN_NOTICE "Error %d adding gamepad", err);
    return err;
  }

  // this is copied verbatim from the compendium
  // create device in /dev/gamepad
  struct class *cl;
  cl = class_create(THIS_MODULE, "gamepad");
  device_create(cl, NULL, devno, NULL, "gamepad");

  // TODO: error check this. it returns a non-NULL pointer if it's all good
  // requests memory regions with corresponding sizes
  // all of these have 4-byte offsets, but some are only defined for 2 bytes
  request_mem_region(GPIO_PC_MODEL, 4, "gamepad");
  request_mem_region(GPIO_PC_DOUT, 2, "gamepad");
  request_mem_region(GPIO_PC_DIN, 2, "gamepad");
  request_mem_region(GPIO_EXTIPSELL, 4, "gamepad");
  request_mem_region(GPIO_EXTIFALL, 2, "gamepad");
  request_mem_region(GPIO_IEN, 2, "gamepad");
  request_mem_region(ISER0, 4, "gamepad");
  request_mem_region(GPIO_IF, 2, "gamepad");
  request_mem_region(GPIO_IFC, 2, "gamepad");

  // map physical address space to virtual memory
  MODEL = ioremap_nocache(GPIO_PC_MODEL, 4);
  DOUT = ioremap_nocache(GPIO_PC_DOUT, 2);
  DIN = ioremap_nocache(GPIO_PC_DIN, 2);
  EXTIPSELL = ioremap_nocache(GPIO_EXTIPSELL, 4);
  EXTIFALL = ioremap_nocache(GPIO_EXTIFALL, 2);
  IEN = ioremap_nocache(GPIO_IEN, 2);
  _ISER0 = ioremap_nocache(ISER0, 4);
  IF = ioremap_nocache(GPIO_IF, 2);
  IFC = ioremap_nocache(GPIO_IFC, 2);

  iowrite32(0x33333333, MODEL); // read mode on buttons
  iowrite32(0x00ff, DOUT); // pull high

  // request IRQ so the kernel handles interrupts for us
  // we use can_request_irq to be as safe as possible (NOTE: seems to not be exported to modules???)
  if (request_irq(17, gamepad_interrupt, 0, "gamepad", NULL)) {
    printk(KERN_INFO "Can't be assigned IRQ 17");
  }
  if (request_irq(18, gamepad_interrupt, 0, "gamepad", NULL)) {
    printk(KERN_INFO "Can't be assigned IRQ 18");
  }

  //enable interrupt generators
  iowrite32(0x22222222, EXTIPSELL); // set port C pin 0-7 as interrupt generators
  iowrite32(0xff, EXTIFALL);       // generate interrupts on 1->0 transitions
  iowrite32(0xff, IEN); // enable interrupt generation on port 0-7

  //enable interrupt handlers
  iowrite32(ioread32(_ISER0) | (0x1 << 11) | (0x1 << 1), _ISER0); // GPIO_ODD and GPIO_EVEN

  // clear interrupt flags
  iowrite32(ioread32(IF), IFC);

  printk(KERN_INFO "Successfully initialised gamepad with major %d", gamepad_major);
  return 0;
}

// exit code
static int gamepad_remove(struct platform_device *dev) {
  // unmaps virtual mem
  iounmap(MODEL);
  iounmap(DIN);
  iounmap(DOUT);
  iounmap(EXTIPSELL);
  iounmap(EXTIFALL);
  iounmap(IEN);
  iounmap(IFC);
  iounmap(IF);
  iounmap(_ISER0);

  // releases memory regions after we're done with them, not necessary but good practice
  release_mem_region(GPIO_PC_MODEL, 4);
  release_mem_region(GPIO_PC_DOUT, 2);
  release_mem_region(GPIO_PC_DIN, 2);
  release_mem_region(GPIO_EXTIPSELL, 4);
  release_mem_region(GPIO_EXTIFALL, 2);
  release_mem_region(GPIO_IEN, 2);
  release_mem_region(GPIO_IFC, 2);
  release_mem_region(GPIO_IF, 2);
  release_mem_region(ISER0, 4);
  // free odd and even GPIO interrupts
  free_irq(17, NULL);
  free_irq(18, NULL);

  return 0;
}

static struct platform_driver gamepad_driver = {
  .probe = gamepad_probe,
  .remove = gamepad_remove,
  .driver = {
    .name = "gamepad",
    .owner = THIS_MODULE,
    .of_match_table = of_gamepad_device_match,
  },
};

// registers driver, which runs gamepad_probe
static int gamepad_device_init(void) {
  return platform_driver_register(&gamepad_driver);
}

// unregisters driver, which runs gamepad_remove
static void gamepad_device_exit(void) {
  platform_driver_unregister(&gamepad_driver);
}

module_init(gamepad_device_init);
module_exit(gamepad_device_exit);
