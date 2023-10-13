/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * IOCTL drivers for switch ports control.
 */

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#include "../../includes/ports_io.h"

extern int nb_port_present_get(uint64_t *buf, int port_num);
extern int nb_port_lpmod_get(uint64_t *buf, int port_num);
extern int nb_port_lpmod_set(uint64_t *buf, int port_num);
extern int nb_port_reset_get(uint64_t *buf, int port_num);
extern int nb_port_reset_set(uint64_t *buf, int port_num);
extern int nb_port_rxlos_get(uint64_t *buf, int port_num);
extern int nb_port_i2c_get(uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
extern int nb_port_i2c_set(uint16_t *buf, int port_num, uint16_t addr, uint16_t reg);
extern int nb_port_xcvr_get(uint16_t *buf, int port_num, uint16_t reg);
extern int nb_port_xcvr_set(uint16_t *buf, int port_num, uint16_t reg);

static int loglv = 0x6;

static struct cdev pio_cdev;
static struct class *pio_class;
static pio_data_t *pio_data_p;


static ssize_t pio_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    PIO_DBG("... GO\n");
    return 0;
}


static ssize_t pio_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    PIO_DBG("... GO\n");
    return len;
}


static int pio_open(struct inode *inode, struct file *file)
{
    PIO_DBG("... GO\n");
    return 0;
}


static int pio_release(struct inode *inode, struct file *file)
{
    PIO_DBG("... GO\n");
    return 0;
}


static long _pio_i2c_read(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err, i, len;
    uint16_t curr_reg;
    pio_args_i2c_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_i2c_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    // Currently, FPGA FW only supports 1 byte R/W
    // TBD: Implement Block R/W when FPGA FW ready.
    len = (int)tmp.data_len;
    curr_reg = tmp.reg;
    for(i=0; i<len; i++) {
        err = nb_port_i2c_get(&tmp.data[i], tmp.port_num, tmp.addr, curr_reg);
        if (err < 0) {
            PIO_ERR("nb_port_i2c_get() fail <err>:%d <num>:%d <addr>:0x%02x <reg>:0x%02x\n",
                    err, tmp.port_num, tmp.addr, curr_reg);
            return -EIO;
        }
        curr_reg += 1;
    }
    if (copy_to_user((void __user *) arg, &tmp, sizeof(pio_args_i2c_t)) ) {
        PIO_ERR("copy_to_user() fail\n");
        return -EFAULT;
    }

    PIO_DBG("... OK\n");
    return 0;
}


static long _pio_i2c_write(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err, i, len;
    uint16_t curr_reg;
    pio_args_i2c_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_i2c_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    // Currently, FPGA FW only supports 1 byte R/W
    // TBD: Implement Block R/W when FPGA FW ready.
    len = (int)tmp.data_len;
    curr_reg = tmp.reg;
    for(i=0; i<len; i++) {
        err = nb_port_i2c_set(&tmp.data[i], tmp.port_num, tmp.addr, tmp.reg);
        if (err < 0) {
            PIO_DBG("nb_port_i2c_set() fail <err>:%d <num>:%d <addr>:0x%02x <reg>:0x%02x\n",
                    err, tmp.port_num, tmp.addr, curr_reg);
            return err;
        }
        curr_reg += 1;
    }

    PIO_DBG("... OK\n");
    return 0;
}



static long _pio_xcvr_read(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err, i, len;
    uint16_t curr_reg;
    pio_args_xcvr_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_xcvr_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    // Setup XCVR bank selection
    // TBD: Page & Bank select will move to platform_drv with transaction handler.
    if (tmp.bank_num != PIO_RW_XCVR_CTL_BYPASS) {
        err = nb_port_xcvr_set(&tmp.bank_num, tmp.port_num, PORTS_XCVR_REG_BANK_SEL);
        if (err < 0) {
            PIO_DBG("Setup Xcvr Bank_Sel fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, PORTS_XCVR_REG_BANK_SEL);
            return err;
        }
    }
    // Setup XCVR page selection
    // TBD: Page & Bank select will move to platform_drv with transaction handler.
    if (tmp.page_num != PIO_RW_XCVR_CTL_BYPASS) {
        err = nb_port_xcvr_set(&tmp.page_num, tmp.port_num, PORTS_XCVR_REG_PAG_SEL);
        if (err < 0) {
            PIO_DBG("Setup Xcvr Page_Sel fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, PORTS_XCVR_REG_PAG_SEL);
            return err;
        }
    }
    // Currently, FPGA FW only supports 1 byte R/W
    // TBD: Implement Block R/W when FPGA FW ready.
    len = (int)tmp.data_len;
    curr_reg = tmp.reg;
    for(i=0; i<len; i++) {
        err = nb_port_xcvr_get(&tmp.data[i], tmp.port_num, curr_reg);
        if (err < 0) {
            PIO_DBG("nb_port_xcvr_get() fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, curr_reg);
            return err;
        }
        curr_reg += 1;
    }
    if (copy_to_user((void __user *) arg, &tmp, sizeof(pio_args_xcvr_t)) ) {
        PIO_ERR("copy_to_user() fail\n");
        return -EFAULT;
    }
    PIO_DBG("... OK\n");
    return 0;
}


static long _pio_xcvr_write(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err, i, len;
    uint16_t curr_reg;
    pio_args_xcvr_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_xcvr_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    // Setup XCVR bank selection
    // TBD: Page & Bank select will move to platform_drv with transaction handler.
    if (tmp.bank_num != PIO_RW_XCVR_CTL_BYPASS) {
        err = nb_port_xcvr_set(&tmp.bank_num, tmp.port_num, PORTS_XCVR_REG_BANK_SEL);
        if (err < 0) {
            PIO_DBG("Setup Xcvr Bank_Sel fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, PORTS_XCVR_REG_BANK_SEL);
            return err;
        }
    }
    // Setup XCVR page selection
    // TBD: Page & Bank select will move to platform_drv with transaction handler.
    if (tmp.page_num != PIO_RW_XCVR_CTL_BYPASS) {
        err = nb_port_xcvr_set(&tmp.page_num, tmp.port_num, PORTS_XCVR_REG_PAG_SEL);
        if (err < 0) {
            PIO_DBG("Setup Xcvr Page_Sel fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, PORTS_XCVR_REG_PAG_SEL);
            return err;
        }
    }
    // Currently, FPGA FW only supports 1 byte R/W
    // TBD: Implement Block R/W when FPGA FW ready.
    len = (int)tmp.data_len;
    curr_reg = tmp.reg;
    for(i=0; i<len; i++) {
        err = nb_port_xcvr_set(&tmp.data[i], tmp.port_num, curr_reg);
        if (err < 0) {
            PIO_DBG("nb_port_i2c_get() fail <err>:%d <num>:%d <reg>:0x%02x\n",
                    err, tmp.port_num, curr_reg);
            return err;
        }
        curr_reg += 1;
    }

    PIO_DBG("... OK\n");
    return 0;
}


static int _nb_port_ioexp_get(struct file *file, unsigned int cmd, unsigned long arg,
                              int (*func_ioexp_get) (uint64_t *buf, int port_num) )
{
    int err;
    pio_args_ioexp_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_ioexp_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    err = func_ioexp_get(&(tmp.data), tmp.port_num);
    if (err < 0) {
        PIO_ERR("func_ioexp_get() fail <err>:%d <num>:%d\n", err, tmp.port_num);
        return err;
    }
    if (copy_to_user((void __user *) arg, &tmp, sizeof(pio_args_ioexp_t)) ) {
        PIO_ERR("copy_to_user() fail\n");
        return -EFAULT;
    }

    PIO_DBG("... OK\n");
    return 0;
}


static int _nb_port_ioexp_set(struct file *file, unsigned int cmd, unsigned long arg,
                              int (*func_ioexp_set) (uint64_t *buf, int port_num) )
{
    int err;
    pio_args_ioexp_t tmp = {};

    PIO_DBG("... GO\n");
    if (copy_from_user(&tmp ,(void __user *) arg, sizeof(pio_args_ioexp_t))) {
        PIO_ERR("copy_from_user() fail\n");
        return -EFAULT;
    }
    err = func_ioexp_set(&(tmp.data), tmp.port_num);
    if (err < 0) {
        PIO_ERR("func_ioexp_set() fail <err>:%d <num>:%d\n", err, tmp.port_num);
        return err;
    }

    PIO_DBG("... OK\n");
    return 0;
}


static long pio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err;

    switch (cmd) {
        case PIO_IOCTL_I2C_READ:
            err = _pio_i2c_read(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_I2C_WRITE:
            err = _pio_i2c_write(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_XCVR_READ:
            err = _pio_xcvr_read(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;
            
        case PIO_IOCTL_XCVR_WRITE:
            err = _pio_xcvr_write(file, cmd, arg);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_PRESENT_READ:
            err = _nb_port_ioexp_get(file, cmd, arg, nb_port_present_get);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_LPMOD_READ:
            err = _nb_port_ioexp_get(file, cmd, arg, nb_port_lpmod_get);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_LPMOD_WRITE:
            err = _nb_port_ioexp_set(file, cmd, arg, nb_port_lpmod_set);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_REST_READ:
            err = _nb_port_ioexp_get(file, cmd, arg, nb_port_reset_get);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_REST_WRITE:
            err = _nb_port_ioexp_set(file, cmd, arg, nb_port_reset_set);
            if (err < 0) {
                return err;
            }
            break;

        case PIO_IOCTL_RXLOS_READ:
            err = _nb_port_ioexp_get(file, cmd, arg, nb_port_rxlos_get);
            if (err < 0) {
                return err;
            }
            break;

        default:
            PIO_ERR("Invalid cmd:%u\n", cmd);
            return -EINVAL;    
    }
    return 0;
}


static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = pio_read,
    .write          = pio_write,
    .open           = pio_open,
    .unlocked_ioctl = pio_ioctl,
    .release        = pio_release,
};


static int create_pio_data_obj(void)
{
    int err;

    pio_data_p = kzalloc(sizeof(*pio_data_p), GFP_KERNEL);
    if (!pio_data_p) {
        PIO_ERR("kzalloc pio_data_p fail!\n");
        err = -ENOMEM;
        goto err_create_pio_data_obj_1;
    }
    pio_data_p->major = MAJOR(0);
    pio_data_p->sfp_min = 0;     // TBD: Get form unify driver
    pio_data_p->sfp_max = 0;     // TBD: Get form unify driver
    pio_data_p->qsfp_min = 1;    // TBD: Get form unify driver
    pio_data_p->qsfp_max = 32;   // TBD: Get form unify driver
    pio_data_p->port_total = 32; // TBD: Get form unify driver

    PIO_DBG("... OK\n");
    return 0;

err_create_pio_data_obj_1:
    return err;
}


static void destroy_pio_data_obj(void)
{
    if (pio_data_p) {
        kfree(pio_data_p);
    }
}


static int create_pio_cdev(void)
{
    int err;

    err = alloc_chrdev_region(&pio_data_p->major, 
                              0, 
                              PORTS_IOCTL_DEV_NUM, 
                              "pio_dev");
    if (err < 0) {
        PIO_ERR("alloc_chrdev_region fail! <err>:%d\n", err);
        goto err_create_pio_cdev_1;
    }
    cdev_init(&pio_cdev, &fops);
    err = cdev_add(&pio_cdev, pio_data_p->major, PORTS_IOCTL_DEV_NUM);
    if (err < 0) {
        PIO_ERR("cdev_add fail! <err>:%d\n", err);
        goto err_create_pio_cdev_2;
    }

    PIO_DBG("... OK\n");
    return 0;

err_create_pio_cdev_2:
    unregister_chrdev_region(pio_data_p->major, PORTS_IOCTL_DEV_NUM);
err_create_pio_cdev_1:
    return err;
}


static void destroy_pio_clsdev(void)
{
    device_destroy(pio_class, pio_data_p->major);
    class_destroy(pio_class);
}


static int create_pio_clsdev(void)
{
    if(IS_ERR(pio_class = class_create(THIS_MODULE, PORTS_IOCTL_CLS_NAME))){
        PIO_ERR("class_create fail\n");
        goto err_create_pio_clsdev_1;
    }
    if(IS_ERR(device_create(pio_class, NULL, pio_data_p->major, NULL, PORTS_IOCTL_DEV_NAME))){
        pr_err("device_create fail\n");
        goto err_create_pio_clsdev_2;
    }
    return 0;

err_create_pio_clsdev_2:
    class_destroy(pio_class);
err_create_pio_clsdev_1:
    return -EPERM;
}


static void destroy_pio_cdev(void)
{
    dev_t devnum = MKDEV(pio_data_p->major, 0);
    cdev_del(&pio_cdev);
    unregister_chrdev_region(devnum, PORTS_IOCTL_DEV_NUM);
}


static int __init ports_ioctl_init(void) 
{
    int err;

    PIO_DBG("... GO\n");

    err = create_pio_data_obj(); 
    if (err < 0) {
        PIO_ERR("create_pio_data_obj fail! <err>:%d\n", err);
        goto err_ports_ioctl_init_1;
    }
    err = create_pio_cdev();
    if (err < 0) {
        PIO_ERR("create_pio_cdev fail! <err>:%d\n", err);
        goto err_ports_ioctl_init_2;
    }
    err = create_pio_clsdev();
    if (err < 0) {
        PIO_ERR("create_pio_class fail! <err>:%d\n", err);
        goto err_ports_ioctl_init_3;
    }

    PIO_INFO("Log level settings: ERR=%d INFO=%d DBG=%d\n", 
             (loglv & 0x4)>>2, (loglv & 0x2)>>1, (loglv & 0x1) );
    PIO_INFO("Version: %s\n", PIO_DRV_VERSION);
    PIO_INFO("... OK\n");
    return 0;

err_ports_ioctl_init_3:
    destroy_pio_cdev();
err_ports_ioctl_init_2:
    destroy_pio_data_obj();
err_ports_ioctl_init_1:
    return err;
}


static void __exit ports_ioctl_exit(void)
{
    PIO_DBG("... GO\n");
    destroy_pio_clsdev();
    destroy_pio_cdev();
    destroy_pio_data_obj();
    PIO_INFO("... GO\n");
}


module_init(ports_ioctl_init);
module_exit(ports_ioctl_exit);

module_param(loglv, int, 0644);
MODULE_PARM_DESC(loglv, "Module log level (all=0xf, err=0x4, info=0x2, dbg=0x1, off=0x0)");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netberg");
MODULE_DESCRIPTION("nba810 Ports IOCTL driver");
