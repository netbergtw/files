#include "netberg_fpga_node.h"

#define DRVNAME "netberg_fpga_module"

static ssize_t attr_show(struct kobject *kobj, struct attribute *attr, char *buffer) {
    struct netberg_node *node = container_of(kobj, struct netberg_node, kobj);
    struct netberg_attribute *netberg_attr = container_of(attr, struct netberg_attribute, attr);
    if (!netberg_attr->show)
        return -EIO;
    return netberg_attr->show(node, netberg_attr, buffer);
}

static ssize_t attr_store(struct kobject *kobj, struct attribute *attr, const char *buffer, size_t size) {
    struct netberg_node *node = container_of(kobj, struct netberg_node, kobj);
    struct netberg_attribute *netberg_attr = container_of(attr, struct netberg_attribute, attr);
    if (!netberg_attr->store)
        return -EIO;
    return netberg_attr->store(node, netberg_attr, buffer, size);
}

static const struct sysfs_ops netberg_sysfs_ops = {
    .show = attr_show,
    .store = attr_store,
};

static void netberg_release(struct kobject *kobj) {
    struct netberg_node *node = container_of(kobj, struct netberg_node, kobj);
    port_arr[node->port.num - 1] = NULL;
    kfree(node);
}

/*
 * Global
 */
uint32_t all_present = 0x0;
struct netberg_dlw_struct netberg_dlw;
static uint16_t eeprom_offset = 0x0;

/*
 * Attributes
 */

// Root attributes
static DEVICE_ATTR(fpga_version, S_IRUGO, fpga_version_read, NULL);
static DEVICE_ATTR(all_present, S_IRUGO, all_present_read, NULL);

static struct attribute *netberg_fpga_root_attributes[] = {&dev_attr_fpga_version.attr, &dev_attr_all_present.attr,
                                                      NULL};

static const struct attribute_group netberg_fpga_root_group = {
    .attrs = netberg_fpga_root_attributes,
};

// Port attributes
static SNOBJ_ATTR(led, S_IWUSR | S_IRUGO, led_read, led_write);
static SNOBJ_ATTR(temp, S_IRUGO, temp_read, NULL);
static SNOBJ_ATTR(present, S_IRUGO, present_read, NULL);
static SNOBJ_ATTR(reset, S_IWUSR, NULL, reset_write);
static SNOBJ_ATTR(lp_mode, S_IWUSR | S_IRUGO, lp_mode_read, lp_mode_write);
static SNOBJ_ATTR(int, S_IRUGO, int_read, NULL);
static SNOBJ_ATTR(eeprom_op, S_IWUSR | S_IRUGO, eeprom_op_read, eeprom_op_write);
static BIN_ATTR_RO(eeprom, 0);

static struct bin_attribute *netberg_fpga_port_bin_attributes[] = {
    &bin_attr_eeprom,
    NULL,
};
static struct attribute *netberg_fpga_port_attributes[] = {
    &snobj_attr_led.attr,
    &snobj_attr_present.attr,
    &snobj_attr_reset.attr,
    &snobj_attr_lp_mode.attr,
    &snobj_attr_int.attr,
    &snobj_attr_temp.attr,
    &snobj_attr_eeprom_op.attr,
    NULL,
};

static const struct attribute_group netberg_fpga_port_group = {
    .attrs = netberg_fpga_port_attributes,
    .bin_attrs = netberg_fpga_port_bin_attributes,
};

static struct kobj_type netberg_ktype = {
    .release = netberg_release,
    .sysfs_ops = &netberg_sysfs_ops,
};

// Root
static ssize_t fpga_version_read(struct device *dev, struct device_attribute *attr, char *buf) {
    ssize_t ret;
    uint32_t version;
    ret = sn_fpga_version_read(&version);
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return ret;
    }
    sprintf(buf, "FPGA version: 0x%X\n", version);
    return strlen(buf);
}
static ssize_t all_present_read(struct device *dev, struct device_attribute *attr, char *buf) {
    sprintf(buf, "%x", all_present);
    return strlen(buf);
}
// Port
static ssize_t led_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) {
    ssize_t ret;
    uint8_t val;
    ret = sn_port_read_led(node->port, &val);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    sprintf(buff, "%d", val);
    return strlen(buff);
exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}
static ssize_t led_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count) {
    ssize_t ret;
    uint32_t val;
    ret = kstrtou32(buff, 10, &val);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    ret = sn_port_write_led(node->port, val);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return count;
exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}

void prst_scan_init(struct netberg_dlw_struct *tmp_dlw) {
    INIT_DELAYED_WORK(&(tmp_dlw->worker), prst_scan_timeout);
    schedule_delayed_work(&(tmp_dlw->worker), 3 * HZ);
}
void prst_scan_timeout(struct work_struct *work) {
    // struct netberg_dlw_struct *tmp_dlw = container_of(work, struct netberg_dlw_struct, worker.work);
    ssize_t ret;
    uint32_t all_prst_new = 0;
    int port_num;
    bool present;
    ret = sn_port_read_all_present(&all_prst_new);
    if (ret) {
        PRINT_ERR("%zd", ret);
        goto quick_exit;
    }
    // NOTE: convert fpga_port_map to front_port_map

    if (all_prst_new != all_present) {
        for (port_num = 1; port_num <= PORT_NUM; port_num++) {
            present = all_prst_new >> (port_num - 1) & 0x1;
            if ((all_present >> (port_num - 1) & 0x1) != present) {
                PRINT_INFO("Port%u %s\n", port_num, present ? "Present" : "Absent");
            }
        }
        all_present = all_prst_new;
    }
    schedule_delayed_work(&netberg_dlw.worker, 1 * HZ);
    return;
quick_exit:
    schedule_delayed_work(&netberg_dlw.worker, 0);
    return;

}
static ssize_t present_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) {
    bool present;
    // ssize_t ret;
    present = all_present >> (node->port.num - 1) & 0x1;
    sprintf(buff, "%d", present);
    return strlen(buff);
}
static ssize_t eeprom_read(struct file *file, struct kobject *kobj, struct bin_attribute *attr, char *buff, loff_t off,
                           size_t count) {
    uint8_t eeprom[MODULE_EEPROM_SIZE] = {0};
    ssize_t ret;
    struct netberg_node *node = container_of(kobj, struct netberg_node, kobj);
    ret = sn_port_read_eeprom(node->port, eeprom, off, count);
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return ret;
    }
    return memory_read_from_buffer(buff, count, &off, eeprom, sizeof(eeprom));
}

static ssize_t temp_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) {
    ssize_t ret;
    int8_t temp;
    ret = sn_port_temp_read(node->port, &temp);
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return ret;
    }
    sprintf(buff, "%d", temp);
    return strlen(buff);
}
static ssize_t reset_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count) {
    ssize_t ret;
    uint32_t val;
    bool reset;
    ret = kstrtou32(buff, 10, &val);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    reset = val ? true : false;
    ret = sn_port_write_reset(node->port, reset);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return count;
exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}
static ssize_t int_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) { return strlen(buff); }
static ssize_t lp_mode_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count) {
    ssize_t ret;
    uint32_t val;
    bool lpmode;
    ret = kstrtou32(buff, 10, &val);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    lpmode = val ? true : false;
    ret = sn_port_write_lpmode(node->port, lpmode);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return count;
exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}
static ssize_t lp_mode_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) {
    ssize_t ret;
    bool lpmode;
    ret = sn_port_read_lpmode(node->port, &lpmode);
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return 0;
    }
    sprintf(buff, "%d", lpmode);
    return strlen(buff);
}
static ssize_t eeprom_op_write(struct netberg_node *node, struct netberg_attribute *attr, const char *buff, size_t count) {
    ssize_t ret;
    int read = 0;
    char *const delim = " ";
    char *token, *cur = (char *)buff;
    uint16_t eeprom_len = 0;
    uint8_t eeprom_val[MODULE_EEPROM_SIZE] = {0};
    long value = 0;

    while ((token = strsep(&cur, delim)) != NULL) {
        ret = kstrtol(token, 10, &value);
        if (ret != SN_SUCCESS) {
            PRINT_ERR("[Netberg] eeprom_op Invalid argument %lx\n", value);
            goto exit_fail;
        }
        if (value > 255) {
            ret = -22;
            PRINT_ERR("[Netberg] eeprom_op Invalid argument %lx\n", value);
            goto exit_fail;
        }
        if (read == 0) {
            eeprom_offset = value & 0xff;
        } else if (read == 1) {
            if ((value & 0xff) == 0) {
                return count;
            } else {
                eeprom_len = value & 0xff;
            }
        } else if (eeprom_offset + read >= MODULE_EEPROM_SIZE) {
            ret = -22;
            goto exit_fail;
        } else {
            eeprom_val[eeprom_offset + read - 2] = value & 0xff;
        }
        read++;
    }
    ret = sn_port_write_eeprom(node->port, eeprom_val, eeprom_offset, eeprom_len);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }

    return count;

exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}
static ssize_t eeprom_op_read(struct netberg_node *node, struct netberg_attribute *attr, char *buff) {
    ssize_t ret;
    uint8_t eeprom_len = 1;
    uint8_t eeprom_val[MODULE_EEPROM_SIZE] = {0};
    int idx;

    ret = sn_port_read_eeprom(node->port, eeprom_val, eeprom_offset, eeprom_len);

    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    for (idx = 0; idx < eeprom_len; idx++) {
        if (idx == 0) {
            sprintf(buff, "%02x %02x", eeprom_offset, eeprom_val[idx]);
        } else {
            sprintf(buff, "%s %02x", buff, eeprom_val[idx]);
        }
    }
    sprintf(buff, "%s \n", buff);
    return strlen(buff);

exit_fail:
    PRINT_ERR("%zd", ret);
    return ret;
}
/*
 * Module
 */
static int netberg_fpga_probe(struct platform_device *pdev) {
    ssize_t ret = 0;
    struct pci_dev *pcidev;
    uint32_t pci_addr;

    PRINT_INFO("Drive probe start");
    pcidev = pci_get_device(NETBERG_FPGA_PCI_VENDOR_ID_XILINX, NETBERG_FPGA_PCI_DEVICE_ID_XILINX, NULL);
    if (!pcidev) {
        return -ENODEV;
    }
    if (pci_enable_device(pcidev)) {
        PRINT_ERR("Netberg pci enable or config fail\n");
        pci_dev_put(pcidev);
        return -ENODEV;
    }
    if (pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &pci_addr)) {
        PRINT_ERR("Netberg pci enable or config fail\n");
        pci_dev_put(pcidev);
        return -ENODEV;
    }
    sn_fpga_print("Netberg PCI MEM address : %#x \n", pci_addr);
    ret = platform_probe(pci_addr);
    if (ret != SN_SUCCESS) {
        PRINT_ERR("Driver probe fail: %zd", ret);
        pci_dev_put(pcidev);
        platform_exit();
        return ret;
    }
    ret = sysfs_create_group(&pdev->dev.kobj, &netberg_fpga_root_group);
    if (ret != SN_SUCCESS) {
        PRINT_INFO("Create group fail: %zd", ret);
        platform_exit();
        pci_dev_put(pcidev);
        return ret;
    }
    prst_scan_init(&netberg_dlw);
    PRINT_INFO("Driver probe done: %zd", ret);
    return ret;
}
static int netberg_fpga_remove(struct platform_device *pdev) {
    sysfs_remove_group(&pdev->dev.kobj, &netberg_fpga_root_group);
    platform_exit();
    return 0;
}
static struct platform_driver netberg_fpga_driver = {
    .probe = netberg_fpga_probe,
    .remove = netberg_fpga_remove,
    .driver =
        {
            .name = DRVNAME,
            .owner = THIS_MODULE,
        },
};

static int __init netberg_fpga_init(void) {
    ssize_t ret;
    int port_num;
    struct netberg_node *p = NULL;

    PRINT_INFO("Init start");
    ret = platform_driver_register(&netberg_fpga_driver);
    if (ret < 0)
        goto exit;

    ret = platform_init();
    if (ret != SN_SUCCESS) {
        goto exit;
    }
    netberg_data = kzalloc(sizeof(struct netberg_fpga_data), GFP_KERNEL);
    if (!netberg_data) {
        ret = -ENOMEM;
        platform_driver_unregister(&netberg_fpga_driver);
        goto exit;
    }
    sn_fpga_print("platform_device_register_simple\n");
    netberg_data->pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
    if (IS_ERR(netberg_data->pdev)) {
        ret = PTR_ERR(netberg_data->pdev);
        platform_driver_unregister(&netberg_fpga_driver);
        kfree(netberg_data);
        goto exit;
    }
    sn_fpga_print("platform_device_register_simple done\n");
    for (port_num = 1; port_num <= PORT_NUM; port_num++) {
        p = kzalloc(sizeof(struct netberg_node), GFP_KERNEL);
        if (!p) {
            ret = -EIO;
            sn_fpga_print("port%d dnode kzalloc fail\n", port_num);
            platform_driver_unregister(&netberg_fpga_driver);
            goto exit;
        }
        p->port.num = port_num;
        ret = kobject_init_and_add(&p->kobj, &netberg_ktype, &netberg_data->pdev->dev.kobj, "port%d", port_num);
        if (ret != SN_SUCCESS) {
            // PRINT_ERR("kobject_init_and_add port%d fail\n", port);
            sn_fpga_print("port%d dnode kobject_init_and_add fail\n", port_num);
            kfree(&p);
            ret = EINVAL;
            platform_driver_unregister(&netberg_fpga_driver);
            goto exit;
        }
        ret = sysfs_create_group(&p->kobj, &netberg_fpga_port_group);
        if (ret != SN_SUCCESS) {
            //  RINT_ERR("sysfs_create_group port%d fail\n", port);
            sn_fpga_print("port%d dnode sysfs_create_group fail\n", port_num);
            kfree(&p);
            ret = EINVAL;
            platform_driver_unregister(&netberg_fpga_driver);
            goto exit;
        }
        port_arr[port_num - 1] = p;
        sn_fpga_print("create port%d dnode \n", port_num);
    }
// PRINT_INFO("Init done");
exit:
    return ret;
}
static void __exit netberg_fpga_exit(void) {
    platform_device_unregister(netberg_data->pdev);
    platform_driver_unregister(&netberg_fpga_driver);
    cancel_delayed_work_sync(&netberg_dlw.worker);
    kfree(netberg_data);
}
module_init(netberg_fpga_init);
module_exit(netberg_fpga_exit);

MODULE_AUTHOR("Netberg");
MODULE_DESCRIPTION("netberg netberg_fpga module driver");
MODULE_LICENSE("GPL");
