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
#include <asm/uaccess.h>

struct cdev gamepad_cdev;

static int gamepad_major = 0;

MODULE_LICENSE("GPL");

//file operations
int gamepad_open(struct inode *inode, struct file *filp) {
  printk(KERN_INFO "gamepad was just opened");
  return 0;
}

int gamepad_release(struct inode *inode, struct file *filp) {
  return 0;
}

ssize_t gamepad_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
  return 0;
}

ssize_t gamepad_write(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
  return 0;
}

static struct file_operations gamepad_fops = {
  .owner = THIS_MODULE,
  .read = gamepad_read,
  .write = gamepad_write,
  .open = gamepad_open,
  .release = gamepad_release,
};

static const struct of_device_id of_gamepad_device_match[] = {
  { .compatible = "tdt4258", },
  { },
};

MODULE_DEVICE_TABLE(of, of_gamepad_device_match);

// init code
static int gamepad_probe(struct platform_device *dev) {
  printk(KERN_INFO "hello world");

  dev_t devno = MKDEV(gamepad_major, 0);
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

  struct class *cl;
  cl = class_create(THIS_MODULE, "gamepad");
  device_create(cl, NULL, devno, NULL, "gamepad");
  printk(KERN_INFO "Successfully initialised gamepad with major %d", gamepad_major);
  return 0;
}

// exit code
static int gamepad_remove(struct platform_device *dev) {
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
