#ifndef _NETBERG_FPGA_NODE_H_
#define _NETBERG_FPGA_NODE_H_

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/workqueue.h>

//#include <linux/pci.h>
//#include <linux/delay.h>

#include "platform.h"
#include "utils.h"

#define SNOBJ_ATTR(_name, _mode, _show, _store) \
    struct netberg_attribute snobj_attr_##_name = __ATTR(_name, _mode, _show, _store)

struct netberg_fpga_data {
    struct platform_device *pdev;
};

struct netberg_node {
    struct kobject kobj;
    port_t port;
};

struct netberg_dlw_struct {
    struct delayed_work worker;
};

struct netberg_attribute {
    struct attribute attr;
    ssize_t (*show)(struct netberg_node *obj, struct netberg_attribute *attr, char *buf);
    ssize_t (*store)(struct netberg_node *obj, struct netberg_attribute *attr, const char *buf, size_t count);
};

/*
 * Global variables
 */
struct netberg_fpga_data *netberg_data = NULL;
struct netberg_node *port_arr[PORT_NUM] = {0};

static ssize_t attr_show(struct kobject *kobj, struct attribute *attr, char *buffer);
static ssize_t attr_store(struct kobject *kobj, struct attribute *attr, const char *buffer, size_t size);
// static void netberg_release(struct kobject *kobj);

// Device attributes
static ssize_t fpga_version_read(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t all_present_read(struct device *dev, struct device_attribute *attr, char *buff);

// Port attributes
void prst_scan_init(struct netberg_dlw_struct *tmp_dlw);
void prst_scan_timeout(struct work_struct *work);
static ssize_t present_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
static ssize_t eeprom_read(struct file *file, struct kobject *kobj, struct bin_attribute *attr, char *buff, loff_t off,
                           size_t count);
static ssize_t eeprom_op_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count);
static ssize_t eeprom_op_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
static ssize_t temp_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
static ssize_t reset_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count);
static ssize_t int_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
static ssize_t lp_mode_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count);
static ssize_t lp_mode_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
static ssize_t led_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count);
static ssize_t led_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff);
#endif
