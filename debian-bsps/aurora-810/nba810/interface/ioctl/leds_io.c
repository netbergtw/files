/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * IOCTL drivers for switch leds control.
 */

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/errno.h>

#include "../../includes/leds_io.h"


extern int nb_led_port_get(uint32_t *buf, int port_num);
extern int nb_led_port_set(uint32_t *buf, int port_num);

static int loglv = 0x6;

static struct cdev lio_cdev;
static struct class *lio_class;

typedef struct {
    dev_t major;
} lio_data_t ;
static lio_data_t *lio_data_p;


static ssize_t lio_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    LIO_DBG("... GO\n");
    return 0;
}


static ssize_t lio_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    LIO_DBG("... GO\n");
    return len;
}


static int lio_open(struct inode *inode, struct file *file)
{
    LIO_DBG("... GO\n");
    return 0;
}


static int lio_release(struct inode *inode, struct file *file)
{
    LIO_DBG("... GO\n");
    return 0;
}


static int led_get(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err;
    lio_args_port_led_t tmp = {};

    LIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(lio_args_port_led_t))) {
        LIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    err = nb_led_port_get(&(tmp.data), tmp.port_num);
    if (err < 0) {
        LIO_ERR("nb_led_port_get() fail <err>:%d <num>:%d\n", err, tmp.port_num);
        return err;
    }
    if (copy_to_user((void __user *) arg, &tmp, sizeof(lio_args_port_led_t)) ) {
        LIO_ERR("copy_to_user() fail\n");
        return -EFAULT;
    }

    LIO_DBG("... OK\n");
    return 0;
}


static int led_set(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err;
    lio_args_port_led_t tmp = {};

    LIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(lio_args_port_led_t))) {
        LIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    err = nb_led_port_set(&(tmp.data), tmp.port_num);
    if (err < 0) {
        LIO_ERR("nb_led_port_set() fail <err>:%d <num>:%d\n", err, tmp.port_num);
        return err;
    }

    LIO_DBG("... OK\n");
    return 0;
}


static long lio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err;

    switch (cmd) {
        case LIO_IOCTL_PORT_GET:
            err = led_get(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;

        case LIO_IOCTL_PORT_SET:
            err = led_set(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;

        default:
            LIO_ERR("Invalid cmd:%u\n", cmd);
            return -EINVAL;       
    }
    return 0;
}


static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = lio_read,
    .write          = lio_write,
    .open           = lio_open,
    .unlocked_ioctl = lio_ioctl,
    .release        = lio_release,
};


static int create_lio_data_obj(void)
{
    int err;

    lio_data_p = kzalloc(sizeof(*lio_data_p), GFP_KERNEL);
    if (!lio_data_p) {
        LIO_ERR("kzalloc lio_data_p fail!\n");
        err = -ENOMEM;
        goto err_create_lio_data_obj_1;
    }
    lio_data_p->major = MAJOR(0);
    // lio_data_p->sfp_min = 0;     // TBD: Get form unify driver
    // lio_data_p->sfp_max = 0;     // TBD: Get form unify driver
    // lio_data_p->qsfp_min = 1;    // TBD: Get form unify driver
    // lio_data_p->qsfp_max = 32;   // TBD: Get form unify driver
    // lio_data_p->port_total = 32; // TBD: Get form unify driver

    LIO_DBG("... OK\n");
    return 0;

err_create_lio_data_obj_1:
    return err;
}


static void destroy_lio_data_obj(void)
{
    if (lio_data_p) {
        kfree(lio_data_p);
    }
}


static int create_lio_cdev(void)
{
    int err;

    err = alloc_chrdev_region(&lio_data_p->major, 
                              0, 
                              LEDS_IOCTL_DEV_NUM, 
                              "lio_dev");
    if (err < 0) {
        LIO_ERR("alloc_chrdev_region fail! <err>:%d\n", err);
        goto err_create_lio_cdev_1;
    }
    cdev_init(&lio_cdev, &fops);
    err = cdev_add(&lio_cdev, lio_data_p->major, LEDS_IOCTL_DEV_NUM);
    if (err < 0) {
        LIO_ERR("cdev_add fail! <err>:%d\n", err);
        goto err_create_lio_cdev_2;
    }

    LIO_DBG("... OK\n");
    return 0;

err_create_lio_cdev_2:
    unregister_chrdev_region(lio_data_p->major, LEDS_IOCTL_DEV_NUM);
err_create_lio_cdev_1:
    return err;
}


static void destroy_lio_clsdev(void)
{
    device_destroy(lio_class, lio_data_p->major);
    class_destroy(lio_class);
}


static int create_lio_clsdev(void)
{
    if(IS_ERR(lio_class = class_create(THIS_MODULE, LEDS_IOCTL_CLS_NAME))){
        LIO_ERR("class_create fail\n");
        goto err_create_lio_clsdev_1;
    }
    if(IS_ERR(device_create(lio_class, NULL, lio_data_p->major, NULL, LEDS_IOCTL_DEV_NAME))){
        pr_err("device_create fail\n");
        goto err_create_lio_clsdev_2;
    }
    return 0;

err_create_lio_clsdev_2:
    class_destroy(lio_class);
err_create_lio_clsdev_1:
    return -EPERM;
}


static void destroy_lio_cdev(void)
{
    dev_t devnum = MKDEV(lio_data_p->major, 0);
    cdev_del(&lio_cdev);
    unregister_chrdev_region(devnum, LEDS_IOCTL_DEV_NUM);
}


static int __init leds_ioctl_init(void) 
{
    int err;

    LIO_DBG("... GO\n");

    err = create_lio_data_obj(); 
    if (err < 0) {
        LIO_ERR("create_lio_data_obj fail! <err>:%d\n", err);
        goto err_leds_ioctl_init_1;
    }
    err = create_lio_cdev();
    if (err < 0) {
        LIO_ERR("create_lio_cdev fail! <err>:%d\n", err);
        goto err_leds_ioctl_init_2;
    }
    err = create_lio_clsdev();
    if (err < 0) {
        LIO_ERR("create_lio_class fail! <err>:%d\n", err);
        goto err_leds_ioctl_init_3;
    }

    LIO_INFO("Log level settings: ERR=%d INFO=%d DBG=%d\n", 
             (loglv & 0x4)>>2, (loglv & 0x2)>>1, (loglv & 0x1) );
    LIO_INFO("Version: %s\n", LIO_DRV_VERSION);
    LIO_INFO("... OK\n");
    return 0;

err_leds_ioctl_init_3:
    destroy_lio_cdev();
err_leds_ioctl_init_2:
    destroy_lio_data_obj();
err_leds_ioctl_init_1:
    return err;
}


static void __exit leds_ioctl_exit(void)
{
    LIO_DBG("... GO\n");
    destroy_lio_clsdev();
    destroy_lio_cdev();
    destroy_lio_data_obj();
    LIO_INFO("... GO\n");
}


module_init(leds_ioctl_init);
module_exit(leds_ioctl_exit);

module_param(loglv, int, 0644);
MODULE_PARM_DESC(loglv, "Module log level (all=0xf, err=0x4, info=0x2, dbg=0x1, off=0x0)");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netberg");
MODULE_DESCRIPTION("nba810 LEDs IOCTL driver");
