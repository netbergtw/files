#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dmi.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/version.h>

#include "x86-64-netberg-aurora-621-cpld.h"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) \
    printk(KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#define BSP_LOG_R(fmt, args...) \
    _bsp_log (LOG_READ, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)

#define BSP_LOG_W(fmt, args...) \
    _bsp_log (LOG_WRITE, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)

#define I2C_READ_BYTE_DATA(ret, lock, i2c_client, reg) \
{ \
    mutex_lock(lock); \
    ret = i2c_smbus_read_byte_data(i2c_client, reg); \
    mutex_unlock(lock); \
    BSP_LOG_R("cpld[%d], reg=0x%03x, reg_val=0x%02x", data->index, reg, ret); \
}

#define I2C_WRITE_BYTE_DATA(ret, lock, i2c_client, reg, val) \
{ \
    mutex_lock(lock); \
    ret = i2c_smbus_write_byte_data(i2c_client, reg, val); \
    mutex_unlock(lock); \
    BSP_LOG_W("cpld[%d], reg=0x%03x, reg_val=0x%02x", data->index, reg, val); \
}

#define _SENSOR_DEVICE_ATTR_RO(_name, _func, _index)     \
    SENSOR_DEVICE_ATTR(_name, S_IRUGO, read_##_func, NULL, _index)

#define _SENSOR_DEVICE_ATTR_WO(_name, _func, _index)     \
    SENSOR_DEVICE_ATTR(_name, S_IWUSR, NULL, write_##_func, _index)

#define _SENSOR_DEVICE_ATTR_RW(_name, _func, _index)     \
    SENSOR_DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, read_##_func, write_##_func, _index)

#define _DEVICE_ATTR(_name)     \
    &sensor_dev_attr_##_name.dev_attr.attr

#define I2C_RW_RETRY_COUNT  3
#define I2C_RW_RETRY_INTERVAL 60

/* CPLD sysfs attributes index  */
enum cpld_sysfs_attributes {
    //CPLD 1

    CPLD_BOARD_ID_0,
    CPLD_BOARD_ID_1,
    CPLD_ID,
    CPLD_CHIP,
    CPLD_SKU_EXT,

    CPLD_EVT_CTRL,

    CPLD_MAC_RESET,
    CPLD_SYSTEM_RESET,
    CPLD_BMC_NTM_RESET,
    CPLD_USB_RESET,
    CPLD_I2C_MUX_RESET,
    CPLD_I2C_MUX_RESET_2,
    CPLD_MISC_RESET,

    CPLD_BRD_PRESENT,
    CPLD_PSU_STATUS,
    CPLD_SYSTEM_PWR,
    CPLD_MAC_SYNCE,
    CPLD_MAC_AVS,
    CPLD_SYSTEM_STATUS,
    CPLD_FAN_PRESENT,
    CPLD_WATCHDOG,
    CPLD_BOOT_SELECT,
    CPLD_MUX_CTRL,
    CPLD_MISC_CTRL_1,
    CPLD_MISC_CTRL_2,
    CPLD_TIMING_CTRL,

    CPLD_MAC_TEMP,

    CPLD_SYSTEM_LED_SYNC,
    CPLD_SYSTEM_LED_SYS,
    CPLD_SYSTEM_LED_FAN,
    CPLD_SYSTEM_LED_PSU_0,
    CPLD_SYSTEM_LED_PSU_1,
    CPLD_SYSTEM_LED_ID,

    //CPLD 2

    //interrupt status
    CPLD_SFP_INTR_PRESENT_0_7,
    CPLD_SFP_INTR_PRESENT_8_15,
    CPLD_SFP_INTR_PRESENT_16_23,
    CPLD_SFP_INTR_PRESENT_24_31,
    CPLD_SFP_INTR_PRESENT_32_39,
    CPLD_SFP_INTR_PRESENT_40_47,
    CPLD_QSFP_INTR_PRESENT_48_53,
    CPLD_QSFP_INTR_PORT_48_53,

    //interrupt mask
    CPLD_SFP_MASK_PRESENT_0_7,
    CPLD_SFP_MASK_PRESENT_8_15,
    CPLD_SFP_MASK_PRESENT_16_23,
    CPLD_SFP_MASK_PRESENT_24_31,
    CPLD_SFP_MASK_PRESENT_32_39,
    CPLD_SFP_MASK_PRESENT_40_47,
    CPLD_QSFP_MASK_PRESENT_48_53,
    CPLD_QSFP_MASK_PORT_48_53,

    //interrupt event
    CPLD_SFP_EVT_PRESENT_0_7,
    CPLD_SFP_EVT_PRESENT_8_15,
    CPLD_SFP_EVT_PRESENT_16_23,
    CPLD_SFP_EVT_PRESENT_24_31,
    CPLD_SFP_EVT_PRESENT_32_39,
    CPLD_SFP_EVT_PRESENT_40_47,
    CPLD_QSFP_EVT_PRESENT_48_53,
    CPLD_QSFP_EVT_PORT_48_53,

    CPLD_SFP_INTR_RX_LOS_0_7,
    CPLD_SFP_INTR_RX_LOS_8_15,
    CPLD_SFP_INTR_RX_LOS_16_23,
    CPLD_SFP_INTR_RX_LOS_24_31,
    CPLD_SFP_INTR_RX_LOS_32_39,
    CPLD_SFP_INTR_RX_LOS_40_47,

    CPLD_SFP_INTR_TX_FAULT_0_7,
    CPLD_SFP_INTR_TX_FAULT_8_15,
    CPLD_SFP_INTR_TX_FAULT_16_23,
    CPLD_SFP_INTR_TX_FAULT_24_31,
    CPLD_SFP_INTR_TX_FAULT_32_39,
    CPLD_SFP_INTR_TX_FAULT_40_47,

    CPLD_SFP_MASK_RX_LOS_0_7,
    CPLD_SFP_MASK_RX_LOS_8_15,
    CPLD_SFP_MASK_RX_LOS_16_23,
    CPLD_SFP_MASK_RX_LOS_24_31,
    CPLD_SFP_MASK_RX_LOS_32_39,
    CPLD_SFP_MASK_RX_LOS_40_47,

    CPLD_SFP_MASK_TX_FAULT_0_7,
    CPLD_SFP_MASK_TX_FAULT_8_15,
    CPLD_SFP_MASK_TX_FAULT_16_23,
    CPLD_SFP_MASK_TX_FAULT_24_31,
    CPLD_SFP_MASK_TX_FAULT_32_39,
    CPLD_SFP_MASK_TX_FAULT_40_47,

    CPLD_SFP_EVT_RX_LOS_0_7,
    CPLD_SFP_EVT_RX_LOS_8_15,
    CPLD_SFP_EVT_RX_LOS_16_23,
    CPLD_SFP_EVT_RX_LOS_24_31,
    CPLD_SFP_EVT_RX_LOS_32_39,
    CPLD_SFP_EVT_RX_LOS_40_47,

    CPLD_SFP_EVT_TX_FAULT_0_7,
    CPLD_SFP_EVT_TX_FAULT_8_15,
    CPLD_SFP_EVT_TX_FAULT_16_23,
    CPLD_SFP_EVT_TX_FAULT_24_31,
    CPLD_SFP_EVT_TX_FAULT_32_39,
    CPLD_SFP_EVT_TX_FAULT_40_47,

    CPLD_SFP_TX_DISABLE_0_7,
    CPLD_SFP_TX_DISABLE_8_15,
    CPLD_SFP_TX_DISABLE_16_23,
    CPLD_SFP_TX_DISABLE_24_31,
    CPLD_SFP_TX_DISABLE_32_39,
    CPLD_SFP_TX_DISABLE_40_47,

    CPLD_QSFP_RESET_48_53,
    CPLD_QSFP_LPMODE_48_53,

    //debug interrupt status
    DBG_CPLD_SFP_INTR_PRESENT_0_7,
    DBG_CPLD_SFP_INTR_PRESENT_8_15,
    DBG_CPLD_SFP_INTR_PRESENT_16_23,
    DBG_CPLD_SFP_INTR_PRESENT_24_31,
    DBG_CPLD_SFP_INTR_PRESENT_32_39,
    DBG_CPLD_SFP_INTR_PRESENT_40_47,
    DBG_CPLD_QSFP_INTR_PRESENT_48_53,
    DBG_CPLD_QSFP_INTR_PORT_48_53,

    //debug interrupt mask
    DBG_CPLD_SFP_INTR_RX_LOS_0_7,
    DBG_CPLD_SFP_INTR_RX_LOS_8_15,
    DBG_CPLD_SFP_INTR_RX_LOS_16_23,
    DBG_CPLD_SFP_INTR_RX_LOS_24_31,
    DBG_CPLD_SFP_INTR_RX_LOS_32_39,
    DBG_CPLD_SFP_INTR_RX_LOS_40_47,

    DBG_CPLD_SFP_INTR_TX_FAULT_0_7,
    DBG_CPLD_SFP_INTR_TX_FAULT_8_15,
    DBG_CPLD_SFP_INTR_TX_FAULT_16_23,
    DBG_CPLD_SFP_INTR_TX_FAULT_24_31,
    DBG_CPLD_SFP_INTR_TX_FAULT_32_39,
    DBG_CPLD_SFP_INTR_TX_FAULT_40_47,

    //BSP DEBUG
    BSP_DEBUG
};

enum bsp_log_types {
    LOG_NONE,
    LOG_RW,
    LOG_READ,
    LOG_WRITE
};

enum bsp_log_ctrl {
    LOG_DISABLE,
    LOG_ENABLE
};

/* CPLD sysfs attributes hook functions  */
static ssize_t read_cpld_callback(struct device *dev,
        struct device_attribute *da, char *buf);
static ssize_t write_cpld_callback(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count);
static u8 _read_cpld_reg(struct device *dev, u8 reg, u8 mask);
static ssize_t read_cpld_reg(struct device *dev, char *buf, u8 reg, u8 mask);
static ssize_t write_cpld_reg(struct device *dev, const char *buf, size_t count, u8 reg, u8 mask);

static LIST_HEAD(cpld_client_list);  /* client list for cpld */
static struct mutex list_lock;  /* mutex for client list */

struct cpld_client_node {
    struct i2c_client *client;
    struct list_head   list;
};

struct cpld_data {
    int index;                  /* CPLD index */
    struct mutex access_lock;   /* mutex for cpld access */
    u8 access_reg;              /* register to access */
};

typedef struct sysfs_info_s
{
    u8 reg;
    u8 mask;
    u8 permission;
} sysfs_info_t;

static sysfs_info_t sysfs_info[] = {
    //CPLD 1

    [CPLD_BOARD_ID_0] = {CPLD_BOARD_ID_0_REG, MASK_ALL, PERM_R},
    [CPLD_BOARD_ID_1] = {CPLD_BOARD_ID_1_REG, MASK_ALL, PERM_R},
    [CPLD_ID]         = {CPLD_ID_REG,         MASK_ALL, PERM_R},
    [CPLD_CHIP]       = {CPLD_CHIP_REG,       MASK_ALL, PERM_R},
    [CPLD_SKU_EXT]    = {CPLD_SKU_EXT_REG,    MASK_ALL, PERM_R},

    [CPLD_EVT_CTRL] = {CPLD_EVT_CTRL_REG, MASK_ALL, PERM_RW},

    [CPLD_MAC_RESET]       = {CPLD_MAC_RESET_REG,     MASK_ALL, PERM_RW},
    [CPLD_SYSTEM_RESET]    = {CPLD_SYSTEM_RESET_REG,  MASK_ALL, PERM_RW},
    [CPLD_BMC_NTM_RESET]   = {CPLD_BMC_NTM_RESET_REG, MASK_ALL, PERM_RW},
    [CPLD_USB_RESET]       = {CPLD_USB_RESET_REG,     MASK_ALL, PERM_RW},
    [CPLD_I2C_MUX_RESET]   = {CPLD_I2C_MUX_RESET_REG,   MASK_ALL, PERM_RW},
    [CPLD_I2C_MUX_RESET_2] = {CPLD_I2C_MUX_RESET_2_REG, MASK_ALL, PERM_RW},
    [CPLD_MISC_RESET]      = {CPLD_MISC_RESET_REG,      MASK_ALL, PERM_RW},

    [CPLD_BRD_PRESENT]   = {CPLD_BRD_PRESENT_REG, MASK_ALL, PERM_R},
    [CPLD_PSU_STATUS]    = {CPLD_PSU_STATUS_REG,  MASK_ALL, PERM_R},
    [CPLD_SYSTEM_PWR]    = {CPLD_SYSTEM_PWR_REG,  MASK_ALL, PERM_R},
    [CPLD_MAC_SYNCE]     = {CPLD_MAC_SYNCE_REG,   MASK_ALL, PERM_R},
    [CPLD_MAC_AVS]       = {CPLD_MAC_AVS_REG,     MASK_ALL, PERM_R},
    [CPLD_SYSTEM_STATUS] = {CPLD_SYSTEM_STATUS_REG, MASK_ALL, PERM_R},
    [CPLD_FAN_PRESENT]   = {CPLD_FAN_PRESENT_REG,   MASK_ALL, PERM_R},
    [CPLD_WATCHDOG]      = {CPLD_WATCHDOG_REG,    MASK_ALL, PERM_RW},
    [CPLD_BOOT_SELECT]   = {CPLD_BOOT_SELECT_REG, MASK_ALL, PERM_RW},
    [CPLD_MUX_CTRL]      = {CPLD_MUX_CTRL_REG,    MASK_ALL, PERM_RW},
    [CPLD_MISC_CTRL_1]   = {CPLD_MISC_CTRL_1_REG, MASK_ALL, PERM_RW},
    [CPLD_MISC_CTRL_2]   = {CPLD_MISC_CTRL_2_REG, MASK_ALL, PERM_RW},
    [CPLD_TIMING_CTRL]   = {CPLD_TIMING_CTRL_REG, MASK_ALL, PERM_RW},

    [CPLD_MAC_TEMP] = {CPLD_MAC_TEMP_REG, MASK_ALL, PERM_R},

    [CPLD_SYSTEM_LED_SYNC]  = {CPLD_SYSTEM_LED_SYNC_REG, CPLD_SYSTEM_LED_SYNC_MASK,  PERM_RW},
    [CPLD_SYSTEM_LED_SYS]   = {CPLD_SYSTEM_LED_SYS_REG,  CPLD_SYSTEM_LED_SYS_MASK,   PERM_RW},
    [CPLD_SYSTEM_LED_FAN]   = {CPLD_SYSTEM_LED_FAN_REG,  CPLD_SYSTEM_LED_FAN_MASK,   PERM_RW},
    [CPLD_SYSTEM_LED_PSU_0] = {CPLD_SYSTEM_LED_PSU_REG,  CPLD_SYSTEM_LED_PSU_0_MASK, PERM_RW},
    [CPLD_SYSTEM_LED_PSU_1] = {CPLD_SYSTEM_LED_PSU_REG,  CPLD_SYSTEM_LED_PSU_1_MASK, PERM_RW},
    [CPLD_SYSTEM_LED_ID]    = {CPLD_SYSTEM_LED_ID_REG,   CPLD_SYSTEM_LED_ID_MASK,    PERM_RW},

    //CPLD 2

    //interrupt status
    [CPLD_SFP_INTR_PRESENT_0_7]     = {CPLD_SFP_INTR_PRESENT_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_PRESENT_8_15]    = {CPLD_SFP_INTR_PRESENT_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_PRESENT_16_23]   = {CPLD_SFP_INTR_PRESENT_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_PRESENT_24_31]   = {CPLD_SFP_INTR_PRESENT_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_PRESENT_32_39]   = {CPLD_SFP_INTR_PRESENT_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_PRESENT_40_47]   = {CPLD_SFP_INTR_PRESENT_40_47_REG, MASK_ALL, PERM_R},
    [CPLD_QSFP_INTR_PRESENT_48_53]  = {CPLD_QSFP_INTR_PRESENT_48_53_REG,  MASK_ALL, PERM_R},
    [CPLD_QSFP_INTR_PORT_48_53]     = {CPLD_QSFP_INTR_PORT_48_53_REG,     MASK_ALL, PERM_R},

    //interrupt mask
    [CPLD_SFP_MASK_PRESENT_0_7]     = {CPLD_SFP_MASK_PRESENT_0_7_REG,   MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_PRESENT_8_15]    = {CPLD_SFP_MASK_PRESENT_8_15_REG,  MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_PRESENT_16_23]   = {CPLD_SFP_MASK_PRESENT_16_23_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_PRESENT_24_31]   = {CPLD_SFP_MASK_PRESENT_24_31_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_PRESENT_32_39]   = {CPLD_SFP_MASK_PRESENT_32_39_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_PRESENT_40_47]   = {CPLD_SFP_MASK_PRESENT_40_47_REG, MASK_ALL, PERM_RW},
    [CPLD_QSFP_MASK_PRESENT_48_53]  = {CPLD_QSFP_MASK_PRESENT_48_53_REG,  MASK_ALL, PERM_RW},
    [CPLD_QSFP_MASK_PORT_48_53]     = {CPLD_QSFP_MASK_PORT_48_53_REG,     MASK_ALL, PERM_RW},

    //interrupt event
    [CPLD_SFP_EVT_PRESENT_0_7]    = {CPLD_SFP_EVT_PRESENT_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_PRESENT_8_15]   = {CPLD_SFP_EVT_PRESENT_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_PRESENT_16_23]  = {CPLD_SFP_EVT_PRESENT_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_PRESENT_24_31]  = {CPLD_SFP_EVT_PRESENT_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_PRESENT_32_39]  = {CPLD_SFP_EVT_PRESENT_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_PRESENT_40_47]  = {CPLD_SFP_EVT_PRESENT_40_47_REG, MASK_ALL, PERM_R},
    [CPLD_QSFP_EVT_PRESENT_48_53] = {CPLD_QSFP_EVT_PRESENT_48_53_REG,  MASK_ALL, PERM_R},
    [CPLD_QSFP_EVT_PORT_48_53]    = {CPLD_QSFP_EVT_PORT_48_53_REG,     MASK_ALL, PERM_R},

    [CPLD_SFP_INTR_RX_LOS_0_7]   = {CPLD_SFP_INTR_RX_LOS_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_RX_LOS_8_15]  = {CPLD_SFP_INTR_RX_LOS_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_RX_LOS_16_23] = {CPLD_SFP_INTR_RX_LOS_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_RX_LOS_24_31] = {CPLD_SFP_INTR_RX_LOS_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_RX_LOS_32_39] = {CPLD_SFP_INTR_RX_LOS_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_RX_LOS_40_47] = {CPLD_SFP_INTR_RX_LOS_40_47_REG, MASK_ALL, PERM_R},

    [CPLD_SFP_INTR_TX_FAULT_0_7]   = {CPLD_SFP_INTR_TX_FAULT_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_TX_FAULT_8_15]  = {CPLD_SFP_INTR_TX_FAULT_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_TX_FAULT_16_23] = {CPLD_SFP_INTR_TX_FAULT_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_TX_FAULT_24_31] = {CPLD_SFP_INTR_TX_FAULT_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_TX_FAULT_32_39] = {CPLD_SFP_INTR_TX_FAULT_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_INTR_TX_FAULT_40_47] = {CPLD_SFP_INTR_TX_FAULT_40_47_REG, MASK_ALL, PERM_R},

    [CPLD_SFP_MASK_RX_LOS_0_7]   = {CPLD_SFP_MASK_RX_LOS_0_7_REG,   MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_RX_LOS_8_15]  = {CPLD_SFP_MASK_RX_LOS_8_15_REG,  MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_RX_LOS_16_23] = {CPLD_SFP_MASK_RX_LOS_16_23_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_RX_LOS_24_31] = {CPLD_SFP_MASK_RX_LOS_24_31_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_RX_LOS_32_39] = {CPLD_SFP_MASK_RX_LOS_32_39_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_RX_LOS_40_47] = {CPLD_SFP_MASK_RX_LOS_40_47_REG, MASK_ALL, PERM_RW},

    [CPLD_SFP_MASK_TX_FAULT_0_7]   = {CPLD_SFP_MASK_TX_FAULT_0_7_REG,   MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_TX_FAULT_8_15]  = {CPLD_SFP_MASK_TX_FAULT_8_15_REG,  MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_TX_FAULT_16_23] = {CPLD_SFP_MASK_TX_FAULT_16_23_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_TX_FAULT_24_31] = {CPLD_SFP_MASK_TX_FAULT_24_31_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_TX_FAULT_32_39] = {CPLD_SFP_MASK_TX_FAULT_32_39_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_MASK_TX_FAULT_40_47] = {CPLD_SFP_MASK_TX_FAULT_40_47_REG, MASK_ALL, PERM_RW},

    [CPLD_SFP_EVT_RX_LOS_0_7]   = {CPLD_SFP_EVT_RX_LOS_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_RX_LOS_8_15]  = {CPLD_SFP_EVT_RX_LOS_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_RX_LOS_16_23] = {CPLD_SFP_EVT_RX_LOS_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_RX_LOS_24_31] = {CPLD_SFP_EVT_RX_LOS_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_RX_LOS_32_39] = {CPLD_SFP_EVT_RX_LOS_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_RX_LOS_40_47] = {CPLD_SFP_EVT_RX_LOS_40_47_REG, MASK_ALL, PERM_R},

    [CPLD_SFP_EVT_TX_FAULT_0_7]   = {CPLD_SFP_EVT_TX_FAULT_0_7_REG,   MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_TX_FAULT_8_15]  = {CPLD_SFP_EVT_TX_FAULT_8_15_REG,  MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_TX_FAULT_16_23] = {CPLD_SFP_EVT_TX_FAULT_16_23_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_TX_FAULT_24_31] = {CPLD_SFP_EVT_TX_FAULT_24_31_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_TX_FAULT_32_39] = {CPLD_SFP_EVT_TX_FAULT_32_39_REG, MASK_ALL, PERM_R},
    [CPLD_SFP_EVT_TX_FAULT_40_47] = {CPLD_SFP_EVT_TX_FAULT_40_47_REG, MASK_ALL, PERM_R},

    [CPLD_SFP_TX_DISABLE_0_7]   = {CPLD_SFP_TX_DISABLE_0_7_REG,   MASK_ALL, PERM_RW},
    [CPLD_SFP_TX_DISABLE_8_15]  = {CPLD_SFP_TX_DISABLE_8_15_REG,  MASK_ALL, PERM_RW},
    [CPLD_SFP_TX_DISABLE_16_23] = {CPLD_SFP_TX_DISABLE_16_23_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_TX_DISABLE_24_31] = {CPLD_SFP_TX_DISABLE_24_31_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_TX_DISABLE_32_39] = {CPLD_SFP_TX_DISABLE_32_39_REG, MASK_ALL, PERM_RW},
    [CPLD_SFP_TX_DISABLE_40_47] = {CPLD_SFP_TX_DISABLE_40_47_REG, MASK_ALL, PERM_RW},

    [CPLD_QSFP_RESET_48_53]  = {CPLD_QSFP_RESET_48_53_REG, MASK_ALL,  PERM_RW},
    [CPLD_QSFP_LPMODE_48_53] = {CPLD_QSFP_LPMODE_48_53_REG, MASK_ALL, PERM_RW},

    //debug interrupt status
    [DBG_CPLD_SFP_INTR_PRESENT_0_7]     = {DBG_CPLD_SFP_INTR_PRESENT_0_7_REG,   MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_PRESENT_8_15]    = {DBG_CPLD_SFP_INTR_PRESENT_8_15_REG,  MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_PRESENT_16_23]   = {DBG_CPLD_SFP_INTR_PRESENT_16_23_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_PRESENT_24_31]   = {DBG_CPLD_SFP_INTR_PRESENT_24_31_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_PRESENT_32_39]   = {DBG_CPLD_SFP_INTR_PRESENT_32_39_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_PRESENT_40_47]   = {DBG_CPLD_SFP_INTR_PRESENT_40_47_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_QSFP_INTR_PRESENT_48_53]  = {DBG_CPLD_QSFP_INTR_PRESENT_48_53_REG,  MASK_ALL, PERM_R},
    [DBG_CPLD_QSFP_INTR_PORT_48_53]     = {DBG_CPLD_QSFP_INTR_PORT_48_53_REG,     MASK_ALL, PERM_R},

    //debug interrupt mask
    [DBG_CPLD_SFP_INTR_RX_LOS_0_7]   = {DBG_CPLD_SFP_INTR_RX_LOS_0_7_REG,   MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_RX_LOS_8_15]  = {DBG_CPLD_SFP_INTR_RX_LOS_8_15_REG,  MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_RX_LOS_16_23] = {DBG_CPLD_SFP_INTR_RX_LOS_16_23_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_RX_LOS_24_31] = {DBG_CPLD_SFP_INTR_RX_LOS_24_31_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_RX_LOS_32_39] = {DBG_CPLD_SFP_INTR_RX_LOS_32_39_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_RX_LOS_40_47] = {DBG_CPLD_SFP_INTR_RX_LOS_40_47_REG, MASK_ALL, PERM_R},

    [DBG_CPLD_SFP_INTR_TX_FAULT_0_7]   = {DBG_CPLD_SFP_INTR_TX_FAULT_0_7_REG,   MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_TX_FAULT_8_15]  = {DBG_CPLD_SFP_INTR_TX_FAULT_8_15_REG,  MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_TX_FAULT_16_23] = {DBG_CPLD_SFP_INTR_TX_FAULT_16_23_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_TX_FAULT_24_31] = {DBG_CPLD_SFP_INTR_TX_FAULT_24_31_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_TX_FAULT_32_39] = {DBG_CPLD_SFP_INTR_TX_FAULT_32_39_REG, MASK_ALL, PERM_R},
    [DBG_CPLD_SFP_INTR_TX_FAULT_40_47] = {DBG_CPLD_SFP_INTR_TX_FAULT_40_47_REG, MASK_ALL, PERM_R},
};

/* CPLD device id and data */
static const struct i2c_device_id cpld_id[] = {
    { "aurora_621_cpld1",  cpld1 },
    { "aurora_621_cpld2",  cpld2 },
    {}
};

char bsp_debug[2]="0";
u8 enable_log_read=LOG_DISABLE;
u8 enable_log_write=LOG_DISABLE;

/* Addresses scanned for cpld */
static const unsigned short cpld_i2c_addr[] = { 0x30, 0x31, I2C_CLIENT_END };

/* define all support register access of cpld in attribute */

// CPLD common
static _SENSOR_DEVICE_ATTR_RO(cpld_board_id_0, cpld_callback,  CPLD_BOARD_ID_0);
static _SENSOR_DEVICE_ATTR_RO(cpld_board_id_1, cpld_callback,  CPLD_BOARD_ID_1);
static _SENSOR_DEVICE_ATTR_RO(cpld_id,         cpld_callback,  CPLD_ID);
static _SENSOR_DEVICE_ATTR_RO(cpld_chip,       cpld_callback,  CPLD_CHIP);
static _SENSOR_DEVICE_ATTR_RO(cpld_sku_ext,    cpld_callback,  CPLD_SKU_EXT);

static _SENSOR_DEVICE_ATTR_RW(cpld_evt_ctrl,   cpld_callback, CPLD_EVT_CTRL);

//CPLD 1
static _SENSOR_DEVICE_ATTR_RW(cpld_mac_reset,       cpld_callback, CPLD_MAC_RESET);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_reset,    cpld_callback, CPLD_SYSTEM_RESET);
static _SENSOR_DEVICE_ATTR_RW(cpld_bmc_ntm_reset,   cpld_callback, CPLD_BMC_NTM_RESET);
static _SENSOR_DEVICE_ATTR_RW(cpld_usb_reset,       cpld_callback, CPLD_USB_RESET);
static _SENSOR_DEVICE_ATTR_RW(cpld_i2c_mux_reset,   cpld_callback, CPLD_I2C_MUX_RESET);
static _SENSOR_DEVICE_ATTR_RW(cpld_i2c_mux_reset_2, cpld_callback, CPLD_I2C_MUX_RESET_2);
static _SENSOR_DEVICE_ATTR_RW(cpld_misc_reset,      cpld_callback, CPLD_MISC_RESET);

static _SENSOR_DEVICE_ATTR_RO(cpld_psu_status,  cpld_callback, CPLD_PSU_STATUS);
static _SENSOR_DEVICE_ATTR_RO(cpld_mac_synce,   cpld_callback, CPLD_MAC_SYNCE);
static _SENSOR_DEVICE_ATTR_RO(cpld_fan_present, cpld_callback, CPLD_FAN_PRESENT);
static _SENSOR_DEVICE_ATTR_RW(cpld_mux_ctrl,    cpld_callback, CPLD_MUX_CTRL);

static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_sync,  cpld_callback, CPLD_SYSTEM_LED_SYNC);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_sys,   cpld_callback, CPLD_SYSTEM_LED_SYS);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_fan,   cpld_callback, CPLD_SYSTEM_LED_FAN);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_psu_0, cpld_callback, CPLD_SYSTEM_LED_PSU_0);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_psu_1, cpld_callback, CPLD_SYSTEM_LED_PSU_1);
static _SENSOR_DEVICE_ATTR_RW(cpld_system_led_id,    cpld_callback, CPLD_SYSTEM_LED_ID);

//CPLD 2
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_0_7,   cpld_callback, CPLD_SFP_INTR_PRESENT_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_8_15,  cpld_callback, CPLD_SFP_INTR_PRESENT_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_16_23, cpld_callback, CPLD_SFP_INTR_PRESENT_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_24_31, cpld_callback, CPLD_SFP_INTR_PRESENT_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_32_39, cpld_callback, CPLD_SFP_INTR_PRESENT_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_present_40_47, cpld_callback, CPLD_SFP_INTR_PRESENT_40_47);

static _SENSOR_DEVICE_ATTR_RO(cpld_qsfp_intr_present_48_53, cpld_callback, CPLD_QSFP_INTR_PRESENT_48_53);
static _SENSOR_DEVICE_ATTR_RO(cpld_qsfp_intr_port_48_53,    cpld_callback, CPLD_QSFP_INTR_PORT_48_53);

static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_0_7,   cpld_callback, CPLD_SFP_MASK_PRESENT_0_7);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_8_15,  cpld_callback, CPLD_SFP_MASK_PRESENT_8_15);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_16_23, cpld_callback, CPLD_SFP_MASK_PRESENT_16_23);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_24_31, cpld_callback, CPLD_SFP_MASK_PRESENT_24_31);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_32_39, cpld_callback, CPLD_SFP_MASK_PRESENT_32_39);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_present_40_47, cpld_callback, CPLD_SFP_MASK_PRESENT_40_47);

static _SENSOR_DEVICE_ATTR_RW(cpld_qsfp_mask_present_48_53, cpld_callback, CPLD_QSFP_MASK_PRESENT_48_53);
static _SENSOR_DEVICE_ATTR_RW(cpld_qsfp_mask_port_48_53,    cpld_callback, CPLD_QSFP_MASK_PORT_48_53);

static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_0_7,   cpld_callback, CPLD_SFP_EVT_PRESENT_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_8_15,  cpld_callback, CPLD_SFP_EVT_PRESENT_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_16_23, cpld_callback, CPLD_SFP_EVT_PRESENT_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_24_31, cpld_callback, CPLD_SFP_EVT_PRESENT_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_32_39, cpld_callback, CPLD_SFP_EVT_PRESENT_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_present_40_47, cpld_callback, CPLD_SFP_EVT_PRESENT_40_47);

static _SENSOR_DEVICE_ATTR_RO(cpld_qsfp_evt_present_48_53,  cpld_callback, CPLD_QSFP_EVT_PRESENT_48_53);
static _SENSOR_DEVICE_ATTR_RO(cpld_qsfp_evt_port_48_53,     cpld_callback, CPLD_QSFP_EVT_PORT_48_53);

static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_0_7,   cpld_callback, CPLD_SFP_INTR_RX_LOS_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_8_15,  cpld_callback, CPLD_SFP_INTR_RX_LOS_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_16_23, cpld_callback, CPLD_SFP_INTR_RX_LOS_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_24_31, cpld_callback, CPLD_SFP_INTR_RX_LOS_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_32_39, cpld_callback, CPLD_SFP_INTR_RX_LOS_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_rx_los_40_47, cpld_callback, CPLD_SFP_INTR_RX_LOS_40_47);

static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_0_7,   cpld_callback, CPLD_SFP_INTR_TX_FAULT_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_8_15,  cpld_callback, CPLD_SFP_INTR_TX_FAULT_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_16_23, cpld_callback, CPLD_SFP_INTR_TX_FAULT_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_24_31, cpld_callback, CPLD_SFP_INTR_TX_FAULT_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_32_39, cpld_callback, CPLD_SFP_INTR_TX_FAULT_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_intr_tx_fault_40_47, cpld_callback, CPLD_SFP_INTR_TX_FAULT_40_47);

static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_0_7,   cpld_callback, CPLD_SFP_MASK_RX_LOS_0_7);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_8_15,  cpld_callback, CPLD_SFP_MASK_RX_LOS_8_15);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_16_23, cpld_callback, CPLD_SFP_MASK_RX_LOS_16_23);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_24_31, cpld_callback, CPLD_SFP_MASK_RX_LOS_24_31);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_32_39, cpld_callback, CPLD_SFP_MASK_RX_LOS_32_39);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_rx_los_40_47, cpld_callback, CPLD_SFP_MASK_RX_LOS_40_47);

static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_0_7,   cpld_callback, CPLD_SFP_MASK_TX_FAULT_0_7);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_8_15,  cpld_callback, CPLD_SFP_MASK_TX_FAULT_8_15);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_16_23, cpld_callback, CPLD_SFP_MASK_TX_FAULT_16_23);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_24_31, cpld_callback, CPLD_SFP_MASK_TX_FAULT_24_31);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_32_39, cpld_callback, CPLD_SFP_MASK_TX_FAULT_32_39);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_mask_tx_fault_40_47, cpld_callback, CPLD_SFP_MASK_TX_FAULT_40_47);

static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_0_7,   cpld_callback, CPLD_SFP_EVT_RX_LOS_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_8_15,  cpld_callback, CPLD_SFP_EVT_RX_LOS_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_16_23, cpld_callback, CPLD_SFP_EVT_RX_LOS_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_24_31, cpld_callback, CPLD_SFP_EVT_RX_LOS_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_32_39, cpld_callback, CPLD_SFP_EVT_RX_LOS_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_rx_los_40_47, cpld_callback, CPLD_SFP_EVT_RX_LOS_40_47);

static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_0_7,   cpld_callback, CPLD_SFP_EVT_TX_FAULT_0_7);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_8_15,  cpld_callback, CPLD_SFP_EVT_TX_FAULT_8_15);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_16_23, cpld_callback, CPLD_SFP_EVT_TX_FAULT_16_23);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_24_31, cpld_callback, CPLD_SFP_EVT_TX_FAULT_24_31);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_32_39, cpld_callback, CPLD_SFP_EVT_TX_FAULT_32_39);
static _SENSOR_DEVICE_ATTR_RO(cpld_sfp_evt_tx_fault_40_47, cpld_callback, CPLD_SFP_EVT_TX_FAULT_40_47);

static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_0_7,   cpld_callback, CPLD_SFP_TX_DISABLE_0_7);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_8_15,  cpld_callback, CPLD_SFP_TX_DISABLE_8_15);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_16_23, cpld_callback, CPLD_SFP_TX_DISABLE_16_23);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_24_31, cpld_callback, CPLD_SFP_TX_DISABLE_24_31);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_32_39, cpld_callback, CPLD_SFP_TX_DISABLE_32_39);
static _SENSOR_DEVICE_ATTR_RW(cpld_sfp_tx_disable_40_47, cpld_callback, CPLD_SFP_TX_DISABLE_40_47);

static _SENSOR_DEVICE_ATTR_RW(cpld_qsfp_reset_48_53,  cpld_callback, CPLD_QSFP_RESET_48_53);
static _SENSOR_DEVICE_ATTR_RW(cpld_qsfp_lpmode_48_53, cpld_callback, CPLD_QSFP_LPMODE_48_53);

static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_0_7,   cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_0_7);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_8_15,  cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_8_15);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_16_23, cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_16_23);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_24_31, cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_24_31);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_32_39, cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_32_39);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_present_40_47, cpld_callback, DBG_CPLD_SFP_INTR_PRESENT_40_47);

static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_qsfp_intr_present_48_53, cpld_callback, DBG_CPLD_QSFP_INTR_PRESENT_48_53);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_qsfp_intr_port_48_53,    cpld_callback, DBG_CPLD_QSFP_INTR_PORT_48_53);

static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_0_7,   cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_0_7);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_8_15,  cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_8_15);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_16_23, cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_16_23);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_24_31, cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_24_31);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_32_39, cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_32_39);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_rx_los_40_47, cpld_callback, DBG_CPLD_SFP_INTR_RX_LOS_40_47);

static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_0_7,   cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_0_7);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_8_15,  cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_8_15);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_16_23, cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_16_23);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_24_31, cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_24_31);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_32_39, cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_32_39);
static _SENSOR_DEVICE_ATTR_RW(dbg_cpld_sfp_intr_tx_fault_40_47, cpld_callback, DBG_CPLD_SFP_INTR_TX_FAULT_40_47);

/* define support attributes of cpldx */

/* cpld 1 */
static struct attribute *cpld1_attributes[] = {
    _DEVICE_ATTR(cpld_board_id_0),
    _DEVICE_ATTR(cpld_board_id_1),

    _DEVICE_ATTR(cpld_id),
    _DEVICE_ATTR(cpld_chip),
    _DEVICE_ATTR(cpld_sku_ext),

    _DEVICE_ATTR(cpld_evt_ctrl),

    _DEVICE_ATTR(cpld_mac_reset),
    _DEVICE_ATTR(cpld_system_reset),
    _DEVICE_ATTR(cpld_bmc_ntm_reset),
    _DEVICE_ATTR(cpld_usb_reset),
    _DEVICE_ATTR(cpld_i2c_mux_reset),
    _DEVICE_ATTR(cpld_i2c_mux_reset_2),
    _DEVICE_ATTR(cpld_misc_reset),

    _DEVICE_ATTR(cpld_psu_status),
    _DEVICE_ATTR(cpld_mac_synce),
    _DEVICE_ATTR(cpld_fan_present),
    _DEVICE_ATTR(cpld_mux_ctrl),

    _DEVICE_ATTR(cpld_system_led_sync),
    _DEVICE_ATTR(cpld_system_led_sys),
    _DEVICE_ATTR(cpld_system_led_fan),
    _DEVICE_ATTR(cpld_system_led_psu_0),
    _DEVICE_ATTR(cpld_system_led_psu_1),
    _DEVICE_ATTR(cpld_system_led_id),

    NULL
};

/* cpld 2 */
static struct attribute *cpld2_attributes[] = {
    _DEVICE_ATTR(cpld_id),
    _DEVICE_ATTR(cpld_chip),

    _DEVICE_ATTR(cpld_sfp_intr_present_0_7),
    _DEVICE_ATTR(cpld_sfp_intr_present_8_15),
    _DEVICE_ATTR(cpld_sfp_intr_present_16_23),
    _DEVICE_ATTR(cpld_sfp_intr_present_24_31),
    _DEVICE_ATTR(cpld_sfp_intr_present_32_39),
    _DEVICE_ATTR(cpld_sfp_intr_present_40_47),

    _DEVICE_ATTR(cpld_qsfp_intr_present_48_53),
    _DEVICE_ATTR(cpld_qsfp_intr_port_48_53),

    _DEVICE_ATTR(cpld_sfp_mask_present_0_7),
    _DEVICE_ATTR(cpld_sfp_mask_present_8_15),
    _DEVICE_ATTR(cpld_sfp_mask_present_16_23),
    _DEVICE_ATTR(cpld_sfp_mask_present_24_31),
    _DEVICE_ATTR(cpld_sfp_mask_present_32_39),
    _DEVICE_ATTR(cpld_sfp_mask_present_40_47),

    _DEVICE_ATTR(cpld_qsfp_mask_present_48_53),
    _DEVICE_ATTR(cpld_qsfp_mask_port_48_53),

    _DEVICE_ATTR(cpld_sfp_evt_present_0_7),
    _DEVICE_ATTR(cpld_sfp_evt_present_8_15),
    _DEVICE_ATTR(cpld_sfp_evt_present_16_23),
    _DEVICE_ATTR(cpld_sfp_evt_present_24_31),
    _DEVICE_ATTR(cpld_sfp_evt_present_32_39),
    _DEVICE_ATTR(cpld_sfp_evt_present_40_47),

    _DEVICE_ATTR(cpld_qsfp_evt_present_48_53),
    _DEVICE_ATTR(cpld_qsfp_evt_port_48_53),

    _DEVICE_ATTR(cpld_evt_ctrl),

    _DEVICE_ATTR(cpld_sfp_intr_rx_los_0_7),
    _DEVICE_ATTR(cpld_sfp_intr_rx_los_8_15),
    _DEVICE_ATTR(cpld_sfp_intr_rx_los_16_23),
    _DEVICE_ATTR(cpld_sfp_intr_rx_los_24_31),
    _DEVICE_ATTR(cpld_sfp_intr_rx_los_32_39),
    _DEVICE_ATTR(cpld_sfp_intr_rx_los_40_47),

    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_0_7),
    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_8_15),
    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_16_23),
    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_24_31),
    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_32_39),
    _DEVICE_ATTR(cpld_sfp_intr_tx_fault_40_47),

    _DEVICE_ATTR(cpld_sfp_mask_rx_los_0_7),
    _DEVICE_ATTR(cpld_sfp_mask_rx_los_8_15),
    _DEVICE_ATTR(cpld_sfp_mask_rx_los_16_23),
    _DEVICE_ATTR(cpld_sfp_mask_rx_los_24_31),
    _DEVICE_ATTR(cpld_sfp_mask_rx_los_32_39),
    _DEVICE_ATTR(cpld_sfp_mask_rx_los_40_47),

    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_0_7),
    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_8_15),
    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_16_23),
    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_24_31),
    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_32_39),
    _DEVICE_ATTR(cpld_sfp_mask_tx_fault_40_47),

    _DEVICE_ATTR(cpld_sfp_evt_rx_los_0_7),
    _DEVICE_ATTR(cpld_sfp_evt_rx_los_8_15),
    _DEVICE_ATTR(cpld_sfp_evt_rx_los_16_23),
    _DEVICE_ATTR(cpld_sfp_evt_rx_los_24_31),
    _DEVICE_ATTR(cpld_sfp_evt_rx_los_32_39),
    _DEVICE_ATTR(cpld_sfp_evt_rx_los_40_47),

    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_0_7),
    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_8_15),
    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_16_23),
    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_24_31),
    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_32_39),
    _DEVICE_ATTR(cpld_sfp_evt_tx_fault_40_47),

    _DEVICE_ATTR(cpld_sfp_tx_disable_0_7),
    _DEVICE_ATTR(cpld_sfp_tx_disable_8_15),
    _DEVICE_ATTR(cpld_sfp_tx_disable_16_23),
    _DEVICE_ATTR(cpld_sfp_tx_disable_24_31),
    _DEVICE_ATTR(cpld_sfp_tx_disable_32_39),
    _DEVICE_ATTR(cpld_sfp_tx_disable_40_47),

    _DEVICE_ATTR(cpld_qsfp_reset_48_53),
    _DEVICE_ATTR(cpld_qsfp_lpmode_48_53),

    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_0_7),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_8_15),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_16_23),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_24_31),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_32_39),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_present_40_47),

    _DEVICE_ATTR(dbg_cpld_qsfp_intr_present_48_53),
    _DEVICE_ATTR(dbg_cpld_qsfp_intr_port_48_53),

    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_0_7),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_8_15),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_16_23),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_24_31),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_32_39),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_rx_los_40_47),

    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_0_7),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_8_15),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_16_23),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_24_31),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_32_39),
    _DEVICE_ATTR(dbg_cpld_sfp_intr_tx_fault_40_47),

    NULL
};

/* cpld 1 attributes group */
static const struct attribute_group cpld1_group = {
    .attrs = cpld1_attributes,
};

/* cpld 2 attributes group */
static const struct attribute_group cpld2_group = {
    .attrs = cpld2_attributes,
};

/* reg shift */
static u8 _shift(u8 mask)
{
    int i=0, mask_one=1;

    for(i=0; i<8; ++i) {
        if ((mask & mask_one) == 1)
            return i;
        else
            mask >>= 1;
    }

    return -1;
}

/* reg mask and shift */
static u8 _mask_shift(u8 val, u8 mask)
{
    int shift=0;

    shift = _shift(mask);

    return (val & mask) >> shift;
}

static int _bsp_log(u8 log_type, char *fmt, ...)
{
    if ((log_type==LOG_READ  && enable_log_read) ||
        (log_type==LOG_WRITE && enable_log_write)) {
        va_list args;
        int r;

        va_start(args, fmt);
        r = vprintk(fmt, args);
        va_end(args);

        return r;
    } else {
        return 0;
    }
}

static ssize_t read_cpld_callback(struct device *dev,
        struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u8 reg = 0;
    u8 mask = MASK_ALL;

    if (IS_PERM_R(sysfs_info[attr->index].permission)) {
        reg = sysfs_info[attr->index].reg;
        mask = sysfs_info[attr->index].mask;
    } else {
        dev_err(dev, "%s() error, attr->index=%d\n", __func__, attr->index);
                    return -EINVAL;
    }

    return read_cpld_reg(dev, buf, reg, mask);
}


/* set cpld register value */
static ssize_t write_cpld_callback(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u8 reg = 0;
    u8 mask = MASK_ALL;

    if (IS_PERM_W(sysfs_info[attr->index].permission)) {
        reg = sysfs_info[attr->index].reg;
        mask = sysfs_info[attr->index].mask;
    } else {
        dev_err(dev, "%s() error, attr->index=%d\n", __func__, attr->index);
                    return -EINVAL;
    }

    return write_cpld_reg(dev, buf, count, reg, mask);
}

/* get cpld register value */
static u8 _read_cpld_reg(struct device *dev,
                    u8 reg,
                    u8 mask)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    int reg_val;

    I2C_READ_BYTE_DATA(reg_val, &data->access_lock, client, reg);

    if (unlikely(reg_val < 0)) {
        return reg_val;
    } else {
        reg_val=_mask_shift(reg_val, mask);
        return reg_val;
    }
}



/* get cpld register value */
static ssize_t read_cpld_reg(struct device *dev,
                    char *buf,
                    u8 reg,
                    u8 mask)
{
    int reg_val;

    reg_val = _read_cpld_reg(dev, reg, mask);
    if (unlikely(reg_val < 0)) {
        dev_err(dev, "read_cpld_reg() error, reg_val=%d\n", reg_val);
        return reg_val;
    } else {
        return sprintf(buf, "0x%02x\n", reg_val);
    }
}

/* set cpld register value */
static ssize_t write_cpld_reg(struct device *dev,
                    const char *buf,
                    size_t count,
                    u8 reg,
                    u8 mask)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct cpld_data *data = i2c_get_clientdata(client);
    u8 reg_val, reg_val_now, shift;
    int ret = 0;

    if (kstrtou8(buf, 0, &reg_val) < 0)
        return -EINVAL;

    //apply continuous bits operation if mask is specified, discontinuous bits are not supported
    if (mask != MASK_ALL) {
        reg_val_now = _read_cpld_reg(dev, reg, MASK_ALL);
        if (unlikely(reg_val_now < 0)) {
            dev_err(dev, "write_cpld_reg() error, reg_val_now=%d\n", reg_val_now);
            return reg_val_now;
        } else {
            //clear bits in reg_val_now by the mask
            reg_val_now &= ~mask;
            //get bit shift by the mask
            shift = _shift(mask);
            //calculate new reg_val
            reg_val = reg_val_now | (reg_val << shift);
        }
    }

    I2C_WRITE_BYTE_DATA(ret, &data->access_lock,
               client, reg, reg_val);

    if (unlikely(ret < 0)) {
        dev_err(dev, "write_cpld_reg() error, return=%d\n", ret);
        return ret;
    }

    return count;
}

/* add valid cpld client to list */
static void cpld_add_client(struct i2c_client *client)
{
    struct cpld_client_node *node = NULL;

    node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
    if (!node) {
        dev_info(&client->dev,
            "Can't allocate cpld_client_node for index %d\n",
            client->addr);
        return;
    }

    node->client = client;

    mutex_lock(&list_lock);
    list_add(&node->list, &cpld_client_list);
    mutex_unlock(&list_lock);
}

/* remove exist cpld client in list */
static void cpld_remove_client(struct i2c_client *client)
{
    struct list_head    *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int found = 0;

    mutex_lock(&list_lock);
    list_for_each(list_node, &cpld_client_list) {
        cpld_node = list_entry(list_node,
                struct cpld_client_node, list);

        if (cpld_node->client == client) {
            found = 1;
            break;
        }
    }

    if (found) {
        list_del(list_node);
        kfree(cpld_node);
    }
    mutex_unlock(&list_lock);
}

/* cpld drvier probe */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
static int cpld_probe(struct i2c_client *client)
#else
static int cpld_probe(struct i2c_client *client, 
    const struct i2c_device_id *dev_id)
#endif
{
    int status;
    struct cpld_data *data = NULL;
    int ret = -EPERM;

    data = kzalloc(sizeof(struct cpld_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    /* init cpld data for client */
    i2c_set_clientdata(client, data);
    mutex_init(&data->access_lock);

    if (!i2c_check_functionality(client->adapter,
                I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_info(&client->dev,
            "i2c_check_functionality failed (0x%x)\n",
            client->addr);
        status = -EIO;
        goto exit;
    }

    /* get cpld id from device */
    dev_info(&client->dev,
            "getting cpld id (0x%x) at addr (0x%x)\n", 
	    CPLD_ID_REG, client->addr);

    ret = i2c_smbus_read_byte_data(client, CPLD_ID_REG);

    if (ret < 0) {
        dev_info(&client->dev,
            "fail to get cpld id (0x%x) at addr (0x%x)\n",
            CPLD_ID_REG, client->addr);
        status = -EIO;
        goto exit;
    }
    dev_info(&client->dev, "cpld id %d(device) \n", ret);

    if (INVALID(ret, cpld1, cpld2)) {
        dev_info(&client->dev,
            "cpld id %d(device) not valid\n", ret);
        //status = -EPERM;
        //goto exit;
    }

#if 0
    /* change client name for each cpld with index */
    snprintf(client->name, sizeof(client->name), "%s_%d", client->name,
            data->index);
#endif

    data->index = ret;

    /* register sysfs hooks for different cpld group */
    dev_info(&client->dev, "probe cpld with index %d\n", data->index);
    switch (data->index) {
    case cpld1:
        status = sysfs_create_group(&client->dev.kobj,
                    &cpld1_group);
        break;
    case cpld2:
        status = sysfs_create_group(&client->dev.kobj,
                    &cpld2_group);
        break;
    default:
        status = -EINVAL;
    }

    if (status)
        goto exit;

    dev_info(&client->dev, "chip found\n");

    /* add probe chip to client list */
    cpld_add_client(client);

    return 0;
exit:
    switch (data->index) {
    case cpld1:
        sysfs_remove_group(&client->dev.kobj, &cpld1_group);
        break;
    case cpld2:
        sysfs_remove_group(&client->dev.kobj, &cpld2_group);
        break;
    default:
        break;
    }
    return status;
}

/* cpld drvier remove */
static 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
void 
#else
int
#endif
cpld_remove(struct i2c_client *client)
{
    struct cpld_data *data = i2c_get_clientdata(client);

    switch (data->index) {
    case cpld1:
        sysfs_remove_group(&client->dev.kobj, &cpld1_group);
        break;
    case cpld2:
        sysfs_remove_group(&client->dev.kobj, &cpld2_group);
        break;
    }
    cpld_remove_client(client);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0)
    return 0
#endif
}

static int aurora_621_cpld_read_internal(struct i2c_client *client, u8 reg)
{
    int retry = I2C_RW_RETRY_COUNT;
    int reg_val = 0;
    struct cpld_data *data = i2c_get_clientdata(client);

    while (retry) {
        I2C_READ_BYTE_DATA(reg_val, &data->access_lock, client, reg);
        if (unlikely(reg_val < 0)) {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;

            if (retry == 0) {
                dev_err(&client->dev, "%s() retry %d times but still failed, reg=%x\n", __func__, I2C_RW_RETRY_COUNT, reg);
            }

            continue;
        }

        break;
    }

    return reg_val;
}

static int aurora_621_cpld_write_internal(struct i2c_client *client, u8 reg, u8 value)
{
    int ret = 0, retry = I2C_RW_RETRY_COUNT;
    struct cpld_data *data = i2c_get_clientdata(client);

    while (retry) {
        I2C_WRITE_BYTE_DATA(ret, &data->access_lock, client, reg, value);
        if (unlikely(ret < 0)) {
            msleep(I2C_RW_RETRY_INTERVAL);
            retry--;

            if (retry == 0) {
                dev_err(&client->dev, "%s() retry %d times but still failed, reg=%x\n", __func__, I2C_RW_RETRY_COUNT, reg);
            }

            continue;
        }
        break;
    }

    return ret;
}

/*
int aurora_621_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            ret = aurora_621_cpld_write_internal(cpld_node->client, reg, value);
            break;
        } else {
            pr_err("cpld_node->client->addr=%x, cpld_addr=%x\n", cpld_node->client->addr, cpld_addr);
        }
    }

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(aurora_621_cpld_write);
*/

int aurora_621_cpld_psu_mux_sel(u8 mux_sel)
{
    unsigned short cpld_addr = cpld_i2c_addr[0];
    u8 reg = CPLD_MUX_CTRL_REG;
    u8 reg_val = 0;
    u8 psu_mux_mask = 0x06;
    u8 mux_sel_val = 0;

    struct list_head   *list_node = NULL;
    struct cpld_client_node *cpld_node = NULL;
    int ret = -EIO;

    switch(mux_sel) {
    case 0:
        //psu 0
        mux_sel_val = 0x04;
        break;
    case 1:
        //psu 1
        mux_sel_val = 0x02;
        break;
    default:
        //bmc
        mux_sel_val = psu_mux_mask;
        break;
    }

    mutex_lock(&list_lock);

    list_for_each(list_node, &cpld_client_list)
    {
        cpld_node = list_entry(list_node, struct cpld_client_node, list);

        if (cpld_node->client->addr == cpld_addr) {
            //read current reg value
            reg_val = aurora_621_cpld_read_internal(cpld_node->client, reg);
            //clear psu_mux_sel bits (bit 1 and 2)
            reg_val &= ~psu_mux_mask;
            //modify psu_mux_sel bits (bit 1 and 2)
            reg_val |= mux_sel_val;
            //write reg value
            aurora_621_cpld_write_internal(cpld_node->client, reg, reg_val);

            break;
        } else {
            pr_err("cpld_node->client->addr=%x, cpld_addr=%x\n", cpld_node->client->addr, cpld_addr);
        }
    }

    mutex_unlock(&list_lock);

    return ret;
}
EXPORT_SYMBOL(aurora_621_cpld_psu_mux_sel);

MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
    .class      = I2C_CLASS_HWMON,
    .driver = {
        .name = "x86_64_netberg_aurora_621_cpld",
    },
    .probe = cpld_probe,
    .remove = cpld_remove,
    .id_table = cpld_id,
    .address_list = cpld_i2c_addr,
};

static int __init cpld_init(void)
{
    mutex_init(&list_lock);
    return i2c_add_driver(&cpld_driver);
}

static void __exit cpld_exit(void)
{
    i2c_del_driver(&cpld_driver);
}

MODULE_AUTHOR("Netberg <support@netbergtw.com>");
MODULE_DESCRIPTION("x86_64_netberg_aurora_621_cpld driver");
MODULE_LICENSE("GPL");

module_init(cpld_init);
module_exit(cpld_exit);
