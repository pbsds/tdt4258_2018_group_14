/*
some parts of this driver are adapted from the
scull example driver found in LDD (3rd edition)
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "efm32gg.h"

struct cdev gamepad_cdev;

static int gamepad_major = 0;
static uint32_t* MODEL, DIN, DOUT;

// TODO: add more driver info here
MODULE_LICENSE("GPL");

ssize_t gamepad_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
  const unsigned int buttons = ioread32(DIN); // you will probably have to change this part to instead read IF for interrupts
  return copy_to_user(buff, &buttons, sizeof(buttons)) ? -EFAULT : 0; // copies DIN to user. returns error (EFAULT) if it didn't read 
}

static struct file_operations gamepad_fops = {
  .owner = THIS_MODULE,
  .read = gamepad_read,
};

static const struct of_device_id of_gamepad_device_match[] = {
  { .compatible = "tdt4258", },
  { },
};

MODULE_DEVICE_TABLE(of, of_gamepad_device_match);

// init code
static int gamepad_probe(struct platform_device *dev) {
  printk(KERN_INFO "hello world");

  dev_t devno = MKDEV(0, 0); // the assignment to MKDEV isn't necessary, but if we don't, the compiler gives some very ornery warnings
  alloc_chrdev_region(&devno, 0, 1, "gamepad");
  gamepad_major = MAJOR(devno);

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
  struct class *cl;
  cl = class_create(THIS_MODULE, "gamepad");
  device_create(cl, NULL, devno, NULL, "gamepad");

  // TODO: error check this. it returns a non-NULL pointer if it's all good
  // requests memory regions with corresponding sizes
  // all of these have 4-byte offsets, but DOUT and DIN are only defined for 2 bytes
  request_mem_region(GPIO_PC_MODEL, 4, "gamepad");
  request_mem_region(GPIO_PC_DOUT, 2, "gamepad");
  request_mem_region(GPIO_PC_DIN, 2, "gamepad");

  MODEL = ioremap_nocache(GPIO_PC_MODEL, 4);
  DIN = ioremap_nocache(GPIO_PC_DIN, 2);
  DOUT = ioremap_nocache(GPIO_PC_DOUT, 2);

  iowrite32(0x33333333, MODEL); // read mode on buttons
  iowrite32(0x00ff, DOUT); // pull high

  printk(KERN_INFO "Successfully initialised gamepad with major %d", gamepad_major);
  return 0;
}

// exit code
static int gamepad_remove(struct platform_device *dev) {
  // unmaps virtual mem
  iounmap(MODEL);
  iounmap(DIN);
  iounmap(DOUT);

  // releases memory regions after we're done with them, not necessary but good practice
  release_mem_region(GPIO_PC_MODEL, 4);
  release_mem_region(GPIO_PC_DOUT, 2);
  release_mem_region(GPIO_PC_DIN, 2);
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
