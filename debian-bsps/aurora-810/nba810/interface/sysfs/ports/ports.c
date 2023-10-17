/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SYSFS drivers for switch ports control.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "ports.h"


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

static struct ports_t *ports_p = NULL;
struct mutex eeprom_lock;

/* [QSFP attributes]
 *
 *  /sys/class/ports/<Port#>/present    [RO]
 *  /sys/class/ports/<Port#>/lpmod      [RW]
 *  /sys/class/ports/<Port#>/reset      [RW]
 *  /sys/class/ports/<Port#>/rxlos      [RO]
 *  /sys/class/ports/<Port#>/eeprom     [RO]
 */
static ssize_t present_show(struct device *dev,
					        struct device_attribute *attr,
					        char *buf)
{
    int err;
    uint64_t data;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);
    err = nb_port_present_get(&data, subdev_p->port_num);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 8, "%d\n", (int)(data & 0x1));
} 
static DEVICE_ATTR_RO(present);


static ssize_t lpmod_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
    int err;
    uint64_t data;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    err = nb_port_lpmod_get(&data, subdev_p->port_num);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 8, "%d\n", (int)(data & 0x1));
} 

static ssize_t lpmod_store(struct device *dev,
		                   struct device_attribute *attr, 
                           const char *buf, size_t count)
{
    ssize_t ret = 0;
    uint64_t data = 0;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    if(kstrtou64(buf, 0, &data)) {
        return -EBFONT;
    }
    if ((data != 0) && (data != 1)) {
        return -EBFONT;
    }
    ret = nb_port_lpmod_set(&data, subdev_p->port_num);
    if (ret < 0) {
        PORT_ERR("fail! <err>:%d\n",(int)ret);
    }
    return ret ? ret : count;
}
static DEVICE_ATTR_RW(lpmod);


static ssize_t reset_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
    int err;
    uint64_t data;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    err = nb_port_reset_get(&data, subdev_p->port_num);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 8, "%d\n", (int)(data & 0x1));
} 

static ssize_t reset_store(struct device *dev,
		                   struct device_attribute *attr, 
                           const char *buf, size_t count)
{
    ssize_t ret;
    uint64_t data;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    if(kstrtou64(buf, 0, &data)) {
        return -EBFONT;
    }
    ret = nb_port_reset_set(&data, subdev_p->port_num);
    if (ret < 0) {
        PORT_ERR("fail! <err>:%d\n",(int)ret);
    }
    return ret ? ret : count;
} 
static DEVICE_ATTR_RW(reset);


static ssize_t rxlos_show(struct device *dev,
                          struct device_attribute *attr,
                          char *buf)
{
    int err;
    uint64_t data;
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    err = nb_port_rxlos_get(&data, subdev_p->port_num);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 8, "%d\n", (int)(data & 0x1));
} 
static DEVICE_ATTR_RO(rxlos);

static struct attribute *ports_qsfp_l2_attrs[] = {
        &dev_attr_present.attr,
        &dev_attr_lpmod.attr,
        &dev_attr_reset.attr,
        &dev_attr_rxlos.attr,
	NULL
};

static int _select_qsfp_upper_page(int port_num, uint16_t page_num)
{
    int err;
    uint16_t data = page_num;
    err = nb_port_xcvr_set(&data, port_num, QSFP_REG_PAGE_SELECT);
    if (err < 0) {
        PORT_ERR("_select_qsfp_upper_page fail! <page>:%d <err>:%d\n", page_num, err);
    }
    return err;
}


static int _dump_qsfp_lower_page(int port_num, uint8_t *data)
{
    int i, err;
    uint16_t tmp;

    // Byte 0 ~ 127
    for (i=0; i<QSFP_VAL_PAGE_SIZE; i++) {
        err = nb_port_xcvr_get(&tmp, port_num, i);
        if (err < 0) {
            PORT_ERR("fail! <offs>:%d <err>:%d\n",i, err);
            return err;
        }
        data[i] = (uint8_t)(tmp & 0xff);
    }
    return 0;
}


static int _dump_qsfp_upper_page(int port_num, uint16_t page_num, uint8_t *data)
{
    int reg_offs, i, err;
    uint16_t tmp;

    if (_select_qsfp_upper_page(port_num, page_num) < 0) {
        return -EIO;
    }
    // Byte 128 ~ 255
    for (i=0; i<QSFP_VAL_PAGE_SIZE; i++) {
        reg_offs = QSFP_VAL_PAGE_SIZE + i;
        err = nb_port_xcvr_get(&tmp, port_num, reg_offs);
        if (err < 0) {
            PORT_ERR("fail! <offs>:%d <err>:%d\n",i, err);
            return err;
        }
        data[i] = (uint8_t)(tmp & 0xff);
    }
    return 0;
}

static ssize_t unpaged_eeprom_read(int port_num, char *buf, loff_t off, size_t count)
{
    int reg_offs, err;
    uint16_t tmp;
    uint8_t eeprom[QSFP_VAL_EEPROM_SIZE_UNPAGED] = {0};

    mutex_lock(&eeprom_lock);
    for (reg_offs=0; reg_offs<QSFP_VAL_EEPROM_SIZE_UNPAGED; reg_offs++) {
        err = nb_port_xcvr_get(&tmp, port_num, reg_offs);
        if (err < 0) {
            goto err_unpaged_eeprom_read;
        }
        eeprom[reg_offs] = (uint8_t)(tmp & 0xff);
    }
    mutex_unlock(&eeprom_lock);
    return memory_read_from_buffer(buf, count, &off, eeprom, sizeof(eeprom));

err_unpaged_eeprom_read:
    mutex_unlock(&eeprom_lock);
    PORT_DBG("fail! <offs>:0x%x <err>:%d\n",reg_offs, err);
    return err;
}

static ssize_t qsfp_paging_eeprom_read(int port_num, char *buf, loff_t off, size_t count)
{
    int index, i, j;
    uint8_t tmp[QSFP_VAL_PAGE_SIZE] = {0};
    uint8_t eeprom[QSFP_VAL_EEPROM_SIZE_PAGED_SFF] = {0};

    index = 0;
    mutex_lock(&eeprom_lock);
    if (_dump_qsfp_lower_page(port_num, tmp) < 0) {
        goto err_qsfp_paging_eeprom_read;
    }
    for (i=0; i<QSFP_VAL_PAGE_SIZE; i++) {
        eeprom[index] = tmp[i];
        index++;
    }
    for (j=0; j<QSFP_VAL_SFF_UPPAGE_TOTAL; j++) {
        if (_dump_qsfp_upper_page(port_num, j, tmp) < 0) {
            goto err_qsfp_paging_eeprom_read;
        }
        for (i=0; i<QSFP_VAL_PAGE_SIZE; i++) {
            eeprom[index] = tmp[i];
            index++;
        }
    }
    mutex_unlock(&eeprom_lock);
    return memory_read_from_buffer(buf, count, &off, eeprom, sizeof(eeprom));

err_qsfp_paging_eeprom_read:
    mutex_unlock(&eeprom_lock);
    return -EIO;
}

static ssize_t qsfp_eeprom_read(int port_num, char *buf, loff_t off, size_t count)
{
    int err;
    uint16_t data;

    err = nb_port_xcvr_get(&data, port_num, QSFP_REG_PAGE_IMPLEMENTED);
    if (err < 0) {
        PORT_ERR("get QSFP_REG_PAGE_IMPLEMENTED fail! <err>:%d\n",err);
        return err;
    }
    /* SFF-8363 6.2.2
     * Upper memory flat or paged.
     * Bit 2 = 1b: Flat memory (lower and upper pages 00h only)
     * Bit 2 = 0b: Paging (at least upper page 03h implemented)
     */
    if (data & QSFP_VAL_FLAT_MEM_BIT_SFF) {
        return unpaged_eeprom_read(port_num, buf, off, count);
    } else {
        return qsfp_paging_eeprom_read(port_num, buf, off, count);
    }
}


static ssize_t cmis_eeprom_read(int port_num, char *buf, loff_t off, size_t count)
{
    int err;
    uint16_t data;

    err = nb_port_xcvr_get(&data, port_num, QSFP_REG_PAGE_IMPLEMENTED);
    if (err < 0) {
        PORT_ERR("get QSFP_REG_PAGE_IMPLEMENTED fail! <err>:%d\n",err);
        return err;
    }
    /* CMIS 8.2.1
     * Upper memory flat or paged.
     * 0b=Paged memory (pages 00h, 01h, 02h, 10h and 11h are implemented)
     * 1b=Flat memory (only page 00h implemented)
     */
    if (data & QSFP_VAL_FLAT_MEM_BIT_CMIS) {
        return unpaged_eeprom_read(port_num, buf, off, count);
    } else {
        /* Due to PAGE_SIZE limitation,
         * We uses the same dump behavior with QSFP paging EEPROM
         */
        return qsfp_paging_eeprom_read(port_num, buf, off, count);
    }
}

static int _get_module_id(int port_num)
{
    int err, i;
    /* Base on SFF & CMIS
     * Module init_t max = 2 sec
     */
    int retry = 10;
    int period_ms = 200;
    uint16_t retval = 0;
    bool is_fail = true;

    for (i=0; i<retry; i++) {
        err = nb_port_xcvr_get(&retval, port_num, QSFP_REG_IDENTIFIER);
        if ((err >= 0) && (retval != 0)) {
            is_fail = false;
            break;
        }
        mdelay(period_ms);
    }
    if (is_fail) {
        PORT_ERR("Module can't be identified! <id>:0x%x <err>:%d\n", retval, err);
        return -EINVAL;
    }
    return (int)retval;
}

static ssize_t eeprom_read(struct file *file,
                           struct kobject *kobj,
                           struct bin_attribute *attr,
                           char *buf, loff_t off, size_t count)
{
    int err;
    int identifier;
    uint64_t presnet;

    struct device *dev = kobj_to_dev(kobj);
    struct subdev_t *subdev_p = dev_get_drvdata(dev);

    // Check present status
    err = nb_port_present_get(&presnet, subdev_p->port_num);
    if (err < 0) {
        PORT_ERR("nb_port_present_get fail! <err>:%d\n",err);
        return err;
    }
    if ((presnet & 0x1) == 0 ) {
        // not present
        return -ENXIO;
    }

    // Identify module type
    identifier = _get_module_id(subdev_p->port_num);
    if (err < 0) {
        return err;
    }
    switch (identifier) {
        case QSFP_VAL_ID_QSFP_Plus:
        case QSFP_VAL_ID_QSFP_28:
            return qsfp_eeprom_read(subdev_p->port_num, buf, off, count);

        case QSFP_VAL_ID_QSFP_DD:
        case QSFP_VAL_ID_QSFP_CMIS:
            return cmis_eeprom_read(subdev_p->port_num, buf, off, count);

        default:
            PORT_ERR("Invalid identifier:0x%x\n", identifier);
            break;
    }
    return -EINVAL;
}
BIN_ATTR_RO(eeprom, 0);


static struct bin_attribute *ports_qsfp_l2_bin_attrs[] = {
    &bin_attr_eeprom,
    NULL,
};

static const struct attribute_group ports_qsfp_l2_group = {
        .attrs = ports_qsfp_l2_attrs,
    .bin_attrs = ports_qsfp_l2_bin_attrs,
};

static const struct attribute_group *ports_qsfp_l2_groups[] = {
        &ports_qsfp_l2_group,
        NULL,
};

/* [Class attributes]
 *
 *  /sys/class/ports/all_present   [RO]
 *  /sys/class/ports/all_lpmod     [RW]
 *  /sys/class/ports/all_reset     [RW]
 *  /sys/class/ports/all_rxlos     [RO]
 */
static ssize_t all_present_show(struct class *class, 
                                struct class_attribute *attr,
                                char *buf)
{
    int get_all = -1;
    int err;
    uint64_t data;

    err = nb_port_present_get(&data, get_all);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 24, "0x%llx\n", data);
}
static CLASS_ATTR_RO(all_present);


static ssize_t all_lpmod_show(struct class *class, 
                              struct class_attribute *attr,
                              char *buf)
{
    int get_all = -1;
    int err;
    uint64_t data;

    err = nb_port_lpmod_get(&data, get_all);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 24, "0x%llx\n", data);
} 

static ssize_t all_lpmod_store(struct class *class, 
                               struct class_attribute *attr,
                               const char *buf, 
                               size_t count)
{
    ssize_t ret;
    uint64_t data;
    int get_all = -1;

    if(kstrtou64(buf, 0, &data)) {
        return -EBFONT;
    }
    ret = nb_port_lpmod_set(&data, get_all);
    if (ret < 0) {
        PORT_ERR("fail! <err>:%d\n",(int)ret);
    }
    return ret ? ret : count;
} 
static CLASS_ATTR_RW(all_lpmod);


static ssize_t all_reset_show(struct class *class, 
                              struct class_attribute *attr,
                              char *buf)
{
    int get_all = -1;
    int err;
    uint64_t data;

    err = nb_port_reset_get(&data, get_all);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 24, "0x%llx\n", data);
} 

static ssize_t all_reset_store(struct class *class, 
                               struct class_attribute *attr,
                               const char *buf, 
                               size_t count)
{
    ssize_t ret;
    uint64_t data;
    int get_all = -1;

    if(kstrtou64(buf, 0, &data)) {
        return -EBFONT;
    }
    ret = nb_port_reset_set(&data, get_all);
    if (ret < 0) {
        PORT_ERR("fail! <err>:%d\n",(int)ret);
    }
    return ret ? ret : count;
} 
static CLASS_ATTR_RW(all_reset);


static ssize_t all_rxlos_show(struct class *class, 
                                struct class_attribute *attr,
                                char *buf)
{
    int get_all = -1;
    int err;
    uint64_t data;

    err = nb_port_rxlos_get(&data, get_all);
    if (err < 0) {
        PORT_ERR("fail! <err>:%d\n",err);
        return err;
    }
    return snprintf(buf, 24, "0x%llx\n", data);
}
static CLASS_ATTR_RO(all_rxlos);


static struct attribute *ports_class_attrs[] = {
	&class_attr_all_present.attr,
    &class_attr_all_lpmod.attr,
    &class_attr_all_reset.attr,
    &class_attr_all_rxlos.attr,
	NULL,
};
ATTRIBUTE_GROUPS(ports_class);


static struct class ports_class = {
	.name  = PORT_STR_CLASS_NAME,
	.owner = THIS_MODULE,
	.class_groups = ports_class_groups,
};


static struct subdev_t * _create_subdev_qsfp(int port_num)
{
    struct subdev_t *subdev_p = NULL;
    dev_t subdev_num = MKDEV(ports_p->major, port_num);

    /* Create L2 node */
    subdev_p = kzalloc(sizeof(*subdev_p), GFP_KERNEL);
    if (!subdev_p) {
        PORT_ERR("kzalloc subdev_p fail!\n");
        goto err_create_qobj_1;
    }
    subdev_p->devt_num = subdev_num;
    subdev_p->port_num = port_num;
    subdev_p->dev_p = device_create_with_groups(&ports_class,          /* struct class *cls                     */
                                                NULL,                  /* struct device *parent                 */
                                                subdev_num,            /* dev_t devt                            */
                                                subdev_p,              /* void *private_data                    */
                                                ports_qsfp_l2_groups,  /* const struct attribute_group **groups */
                                                "%d",                  /* const char *fmt                       */
                                                port_num
                                                );
    if (IS_ERR(subdev_p->dev_p)) {
        PORT_ERR("device_create L2 fail!\n");
        goto err_create_qobj_2;
    }
    subdev_p->devt_num = subdev_num;

    PORT_DBG("port:%d ... OK\n", port_num);
    return subdev_p;

err_create_qobj_2:
    kfree(subdev_p);
err_create_qobj_1:
    return NULL;
}


static void delete_ports_interface(void)
{
    int id = 0;
    struct subdev_t *l2_subdev_p = NULL;
    struct subdev_t *l3_subdev_p = NULL;
    
    if (ports_p) {
        for (id = 0; id < ports_p->port_total; id++) {
            /* L2 objects */
            l2_subdev_p = ports_p->subdev_p[id];
            if (l2_subdev_p) {
                /* L3 objects */
                l3_subdev_p = l2_subdev_p->subdev_p;
                if (l3_subdev_p) {
                    device_unregister(l3_subdev_p->dev_p);
                    device_destroy(&ports_class, l3_subdev_p->devt_num);
                    kfree(l3_subdev_p);
                }
                device_unregister(l2_subdev_p->dev_p);
                device_destroy(&ports_class, l2_subdev_p->devt_num);
                kfree(l2_subdev_p);
            }
        }
    }
    unregister_chrdev_region(MKDEV(ports_p->major, 0), ports_p->subdev_total);
    PORT_DBG("... OK\n");
}


static int create_ports_interface(void) 
{
    int err = 0;
    int port_num = 0;
    struct subdev_t *pobj_p = NULL;

    err = alloc_chrdev_region(&ports_p->major, 0, ports_p->subdev_total, PORT_STR_CLASS_NAME);
    if (err < 0) {
        PORT_ERR("alloc_chrdev_region fail!\n");
        goto err_reg_ports_if_1;
    }
    for (port_num = 1; port_num <= ports_p->port_total; port_num++) {
        if ((port_num >= ports_p->sfp_min) && (port_num <= ports_p->sfp_max)) { 
            /*
             *  TBD: SFP port handling
             */
            PORT_ERR("Not implement ready! <err>:SFP_port\n");
            goto err_reg_ports_if_2;
        }
        if ((port_num >= ports_p->qsfp_min) && (port_num <= ports_p->qsfp_max)) { 
            pobj_p = _create_subdev_qsfp(port_num);
            if (pobj_p) {
                ports_p->subdev_p[(port_num - 1)] = pobj_p;
                continue;
            }
            PORT_ERR("port: %d ... fail!\n", port_num);
            err = -EPERM;
            goto err_reg_ports_if_2;
        }
    }
    PORT_DBG("... OK\n");
    return 0;

err_reg_ports_if_2:
    delete_ports_interface();
err_reg_ports_if_1:
    return err;
}


static void delete_ports_object(void)
{
    if (ports_p) {
        if (ports_p->subdev_p) {            
            kfree(ports_p->subdev_p);
        }
        kfree(ports_p);
    }
}


static int create_ports_object(void) 
{
    int err = 0;

    ports_p = kzalloc(sizeof(*ports_p), GFP_KERNEL);
    if (!ports_p) {
        PORT_ERR("kzalloc ports_p fail!\n");
        err = -ENOMEM;
        goto err_create_ports_obj_1;
    }
    ports_p->major = MAJOR(0);
    ports_p->sfp_min = 0;     /* TBD: Get from board driver */
    ports_p->sfp_max = 0;     /* TBD: Get from board driver */
    ports_p->qsfp_min = 1;    /* TBD: Get from board driver */
    ports_p->qsfp_max = 32;   /* TBD: Get from board driver */
    ports_p->port_total = ((ports_p->sfp_max - ports_p->sfp_min + 1) + 
                           (ports_p->qsfp_max - ports_p->qsfp_min + 1));
    ports_p->subdev_total = ports_p->port_total;
    ports_p->subdev_p = kzalloc((sizeof(struct subdev_t) * ports_p->port_total), GFP_KERNEL);
    if (!ports_p->subdev_p) {
        PORT_ERR("kzalloc subdev_p fail!\n");
        err = -ENOMEM;
        goto err_create_ports_obj_2;
    }

    PORT_DBG("... OK!\n");
    return 0;

err_create_ports_obj_2:
    kfree(ports_p);
err_create_ports_obj_1:
    return err;
}


static int __init ports_sysfs_init(void) 
{
    int err = -1;

    PORT_DBG("... GO\n");
    err = class_register(&ports_class);
    if (err < 0) {
        PORT_ERR("class_register fail.\n");
        goto err_init_1;
    }
    err = create_ports_object();
    if (err < 0) {
        PORT_ERR("create_ports_object fail!\n");
        goto err_init_2;
    }
    err = create_ports_interface();
    if (err < 0) {
        PORT_ERR("create_ports_interface fail!\n");
        goto err_init_3;
    }
    mutex_init(&eeprom_lock);

    PORT_INFO("Log level settings: ERR=%d INFO=%d DBG=%d\n", 
              (loglv & 0x4)>>2, (loglv & 0x2)>>1, (loglv & 0x1) );
    PORT_INFO("Version: %s\n", PORT_DRV_VERSION);
    PORT_INFO("... OK\n");
    return 0;

err_init_3:
    delete_ports_object();
err_init_2:
    class_unregister(&ports_class);
err_init_1:    
    return err;
}


static void __exit ports_sysfs_exit(void)
{
    PORT_DBG("... GO\n");
    mutex_destroy(&eeprom_lock);
    delete_ports_interface();
    delete_ports_object();
    class_unregister(&ports_class);
    PORT_INFO("... OK\n");
}


module_init(ports_sysfs_init);
module_exit(ports_sysfs_exit);

module_param(loglv, int, 0644);
MODULE_PARM_DESC(loglv, "Module log level (all=0xf, err=0x4, info=0x2, dbg=0x1, off=0x0)");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netberg");
MODULE_DESCRIPTION("nba810 Ports SYSFS driver");
