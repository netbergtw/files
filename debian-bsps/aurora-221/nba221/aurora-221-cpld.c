/*
 * A cpld driver for the aurora-221
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/hwmon-sysfs.h>
#include <linux/gpio.h>

#define BSP_LOG_R(fmt, args...) \
    _bsp_log (LOG_READ, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)
#define BSP_LOG_W(fmt, args...) \
    _bsp_log (LOG_WRITE, KERN_INFO "%s:%s[%d]: " fmt "\r\n", \
            __FILE__, __func__, __LINE__, ##args)

#define BSP_PR(level, fmt, args...) _bsp_log (LOG_SYS, level "[BSP]" fmt "\r\n", ##args)
#define DRIVER_NAME "aurora_221_cpld"

/* LPC registers */

#define REG_BASE_CPU                      0x600
#define REG_BASE_MB                       0x700
#define REG_BASE_EC                       0xe000

//MB CPLD
#define REG_MB_BRD_ID_0                   (REG_BASE_MB + 0x00)
#define REG_MB_BRD_ID_1                   (REG_BASE_MB + 0x01)
#define REG_MB_CPLD_VERSION               (REG_BASE_MB + 0x02)
#define REG_MB_CPLD_BUILD                 (REG_BASE_MB + 0x04)
#define REG_MB_EXTEND_ID                  (REG_BASE_MB + 0x06)
#define REG_MB_MUX_RESET                  (REG_BASE_MB + 0x43)
#define REG_MB_FAN_STATUS                 (REG_BASE_MB + 0x55)
#define REG_MB_PSU_STATUS                 (REG_BASE_MB + 0x59)
#define REG_MB_PORT_LED_CLR               (REG_BASE_MB + 0x80)
#define REG_MB_SYS_LED_CTRL_1             (REG_BASE_MB + 0x81)
#define REG_MB_SYS_LED_STATUS_1           (REG_BASE_MB + 0x82)
#define REG_MB_SYS_LED_STATUS_2           (REG_BASE_MB + 0x83)
#define REG_MB_SYS_LED_STATUS_3           (REG_BASE_MB + 0x84)
#define REG_MB_POWER_EN                   (REG_BASE_MB + 0x95)
//EC
#define REG_EC_SEL                        (REG_BASE_EC + 0x394)
#define REG_EC_PSU_RST                    (REG_BASE_EC + 0x392)

#define MASK_ALL                          (0xFF)

#define MDELAY_LPC                        (5)
#define MDELAY_RESET_INTERVAL             (100)
#define MDELAY_RESET_FINISH               (500)

#define MULTIBIT_SET(addr, mask, value) (((addr)&((0xff)^(mask)))|((value)&(mask)))


/* LPC sysfs attributes index  */
enum cpld_sysfs_attributes {
    //MB CPLD
    ATT_MB_BRD_ID_0,
    ATT_MB_BRD_ID_1,
    ATT_MB_CPLD_1_VERSION,
    ATT_MB_CPLD_1_VERSION_H,
    ATT_MB_BRD_SKU_ID,
    ATT_MB_BRD_HW_ID,
    ATT_MB_BRD_ID_TYPE,
    ATT_MB_BRD_BUILD_ID,
    ATT_MB_BRD_DEPH_ID,
    ATT_MB_BRD_EXT_ID,
    ATT_MB_MUX_RESET,
    ATT_MB_FAN_STATUS,
    ATT_MB_PSU_STATUS,
    ATT_MB_PORT_LED_CLR,
    ATT_MB_LED_SYS,
    ATT_MB_LED_ID,
    ATT_MB_LED_POE,
    ATT_MB_LED_SPD,
    ATT_MB_LED_FAN,
    ATT_MB_LED_LNK,
    ATT_MB_LED_PWR0,
    ATT_MB_LED_PWR1,
    ATT_MB_POWER_EN,
    //EC
    ATT_EC_PSU_RESET
};

enum bases {
    BASE_DEC,
    BASE_HEX,
    BASE_NONE

};

enum bsp_log_types {
    LOG_NONE,
    LOG_RW,
    LOG_READ,
    LOG_WRITE,
    LOG_SYS
};

enum bsp_log_ctrl {
    LOG_DISABLE,
    LOG_ENABLE
};

struct cpld_data_s {
    struct mutex    access_lock;
};

struct cpld_data_s *cpld_data;
char bsp_version[16];
char bsp_debug[32];
char bsp_reg[8]="0x0";
u8 enable_log_read=LOG_DISABLE;
u8 enable_log_write=LOG_DISABLE;
u8 enable_log_sys=LOG_ENABLE;

/* mask len and shift */
static void _get_len_shift(u8 mask, u8 *len, u8 *shift)
{
    int i;
    bool found=false;
    *len=0;
    *shift=0;

    for(i=0; i<8; ++i) {
        if(mask & 1) {
            *len = *len + 1;
            if(!found) {
                *shift = i;
                found = true;
            }
        }
        mask >>= 1;
    }
}

/* reg mask and shift */
static u8 _mask_shift(u8 val, u8 mask)
{
    u8 shift=0;
    u8 len;

    _get_len_shift(mask, &len, &shift);

    return (val & mask) >> shift;
}

static u8 _bit_operation(u8 reg_val, u8 bit, u8 bit_val)
{
    if(bit_val == 0)
        reg_val = reg_val & ~(1 << bit);
    else
        reg_val = reg_val | (1 << bit);
    return reg_val;
}

static int _bsp_log(u8 log_type, char *fmt, ...)
{
    if((log_type==LOG_READ  && enable_log_read) ||
        (log_type==LOG_WRITE && enable_log_write) ||
        (log_type==LOG_SYS && enable_log_sys) ) {
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

/* get cpld register value */
static u8 _read_cpld_reg(u16 reg, u8 mask)
{
    u8 reg_val=0x0, reg_mk_shf_val=0x0;

    mutex_lock(&cpld_data->access_lock);
    reg_val = inb(reg);
    mutex_unlock(&cpld_data->access_lock);

    reg_mk_shf_val = _mask_shift(reg_val, mask);

    BSP_LOG_R("reg=0x%03x, reg_val=0x%02x, mask=0x%02x, reg_mk_shf_val=0x%02x", reg, reg_val, mask, reg_mk_shf_val);

    return reg_mk_shf_val;
}

/* get cpld register value */
static ssize_t read_cpld_reg(u16 reg, u8 mask, u8 base, char *buf)
{
    u8 reg_val;
    int len=0;

    reg_val = _read_cpld_reg(reg, mask);
    if(base == BASE_HEX) {
        len=sprintf(buf, "0x%02x\n", reg_val);
    } else {
        len=sprintf(buf,"%d\n", reg_val);
    }

    return len;
}

/* set cpld register value */
static ssize_t write_cpld_reg(u16 reg, u8 mask, const char *buf, size_t count)
{
    u8 reg_val, reg_val_now, shift, mask_len;

    if(kstrtou8(buf, 0, &reg_val) < 0)
        return -EINVAL;

    _get_len_shift(mask, &mask_len, &shift);

    // set signal bit
    if(mask_len == 1) {
        reg_val_now = _read_cpld_reg(reg, MASK_ALL);
        reg_val = _bit_operation(reg_val_now, shift, reg_val);
    // set multi bit
    } else if (mask_len > 1) {
        reg_val_now = _read_cpld_reg(reg, MASK_ALL);
        reg_val = MULTIBIT_SET(reg_val_now, mask, reg_val<<shift);
    }

    mutex_lock(&cpld_data->access_lock);

    outb(reg_val, reg);
    mdelay(MDELAY_LPC);

    mutex_unlock(&cpld_data->access_lock);

    BSP_LOG_W("reg=0x%03x, reg_val=0x%02x, mask=0x%02x", reg, reg_val, mask);

    return count;
}

/* get mb cpld version in human readable format */
static ssize_t read_mb_cpld_1_version_h(struct device *dev,
        struct device_attribute *da, char *buf)
{
    ssize_t len=0;
    u16 reg = REG_MB_CPLD_VERSION;
    u8 mask = MASK_ALL;
    u8 mask_major = 0b11000000;
    u8 mask_minor = 0b00111111;
    u8 reg_val;
    u8 major, minor, build;

    mutex_lock(&cpld_data->access_lock);
    reg_val = _mask_shift(inb(reg), mask);
    mutex_unlock(&cpld_data->access_lock);
    major = _mask_shift(reg_val, mask_major);
    minor = _mask_shift(reg_val, mask_minor);
    BSP_LOG_R("reg=0x%03x, reg_val=0x%02x", reg, reg_val);

    reg = REG_MB_CPLD_BUILD;
    mutex_lock(&cpld_data->access_lock);
    reg_val = _mask_shift(inb(reg), mask);
    mutex_unlock(&cpld_data->access_lock);
    build = reg_val;
    BSP_LOG_R("reg=0x%03x, reg_val=0x%02x", reg, reg_val);

    len=sprintf(buf, "%d.%02d.%03d\n", major, minor, build);
    return len;
}

/* set mb_mux_reset register value */
static ssize_t write_mb_mux_reset(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    u8 val = 0;
    u8 reg_val = 0;
    static int mux_reset_flag = 0;

    if(kstrtou8(buf, 0, &val) < 0)
        return -EINVAL;

    if(mux_reset_flag == 0) {
        if(val == 0) {
            mutex_lock(&cpld_data->access_lock);
            mux_reset_flag = 1;
            printk(KERN_INFO "i2c mux reset is triggered...\n");
            reg_val = inb(REG_MB_MUX_RESET);
            outb((reg_val & 0b11111000), REG_MB_MUX_RESET);
            mdelay(100);
            outb((reg_val | 0b00000111), REG_MB_MUX_RESET);
            mdelay(500);
            mux_reset_flag = 0;
            mutex_unlock(&cpld_data->access_lock);
        } else {
            return -EINVAL;
        }
    } else {
        printk(KERN_INFO "i2c mux is resetting... (ignore)\n");
        mutex_lock(&cpld_data->access_lock);
        mutex_unlock(&cpld_data->access_lock);
    }

    return count;
}

/* get cpld register value */
static ssize_t read_cpld_callback(struct device *dev,
        struct device_attribute *da, char *buf)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u16 reg = 0;
    u8 mask = MASK_ALL;
    u8 base = BASE_DEC;

    switch (attr->index) {
        //MB CPLD
        case ATT_MB_BRD_ID_0:
            reg = REG_MB_BRD_ID_0;
            break;
        case ATT_MB_BRD_ID_1:
            reg = REG_MB_BRD_ID_1;
            break;
        case ATT_MB_CPLD_1_VERSION:
            reg = REG_MB_CPLD_VERSION;
            break;
        case ATT_MB_BRD_SKU_ID:
            reg = REG_MB_BRD_ID_0;
            mask = 0xFF;
            break;
        case ATT_MB_BRD_HW_ID:
            reg = REG_MB_BRD_ID_1;
            mask = 0x3;
            break;
        case ATT_MB_BRD_ID_TYPE:
            reg = REG_MB_BRD_ID_1;
            mask = 0x80;
            break;
        case ATT_MB_BRD_BUILD_ID:
            reg = REG_MB_BRD_ID_1;
            mask = 0x18;
            break;
        case ATT_MB_BRD_DEPH_ID:
            reg = REG_MB_BRD_ID_1;
            mask = 0x4;
            break;
        case ATT_MB_BRD_EXT_ID:
            reg = REG_MB_EXTEND_ID;
            mask = 0x07;
            break;
        case ATT_MB_MUX_RESET:
            reg = REG_MB_MUX_RESET;
            mask = 0x07;
            base = BASE_HEX;
            break;
        case ATT_MB_FAN_STATUS:
            reg = REG_MB_FAN_STATUS;
            base = BASE_HEX;
            break;
        case ATT_MB_PSU_STATUS:
            reg = REG_MB_PSU_STATUS;
            base = BASE_HEX;
            break;
        case ATT_MB_PORT_LED_CLR:
            reg = REG_MB_PORT_LED_CLR;
            mask = 0x1;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_SYS:
            reg = REG_MB_SYS_LED_CTRL_1;
            mask = 0xF0;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_ID:
            reg = REG_MB_SYS_LED_CTRL_1;
            mask = 0xF;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_POE:
            reg = REG_MB_SYS_LED_STATUS_1;
            mask = 0xF0;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_SPD:
            reg = REG_MB_SYS_LED_STATUS_1;
            mask = 0xF;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_FAN:
            reg = REG_MB_SYS_LED_STATUS_2;
            mask = 0xF0;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_LNK:
            reg = REG_MB_SYS_LED_STATUS_2;
            mask = 0xF;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_PWR1:
            reg = REG_MB_SYS_LED_STATUS_3;
            mask = 0xF0;
            base = BASE_HEX;
            break;
        case ATT_MB_LED_PWR0:
            reg = REG_MB_SYS_LED_STATUS_3;
            mask = 0xF;
            base = BASE_HEX;
            break;
        case ATT_MB_POWER_EN:
            reg = REG_MB_POWER_EN;
            base = BASE_HEX;
            break;
        default:
            return -EINVAL;
    }
    return read_cpld_reg(reg, mask, base, buf);
}

/* set cpld register value */
static ssize_t write_cpld_callback(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    u16 reg = 0;
    u8 mask = MASK_ALL;

    switch (attr->index) {
        case ATT_MB_PORT_LED_CLR:
            reg = REG_MB_PORT_LED_CLR;
            mask = 0x1;
            break;
        case ATT_MB_LED_SYS:
            reg = REG_MB_SYS_LED_CTRL_1;
            mask = 0xF0;
            break;
        case ATT_MB_LED_ID:
            reg = REG_MB_SYS_LED_CTRL_1;
            mask = 0xF;
            break;
        case ATT_MB_POWER_EN:
            reg = REG_MB_POWER_EN;
            break;
        default:
            return -EINVAL;
    }
    return write_cpld_reg(reg, mask, buf, count);
}

/* set psu_reset register value */
static ssize_t write_ec_psu_reset(struct device *dev,
        struct device_attribute *da, const char *buf, size_t count)
{
    u8 val = 0;
    u8 reg_val = 0;
    static int psu_reset_flag = 0;

    if (kstrtou8(buf, 0, &val) < 0)
        return -EINVAL;

    if (psu_reset_flag == 0) {
        if (val == 0) {
            mutex_lock(&cpld_data->access_lock);
            psu_reset_flag = 1;
            reg_val = 0x1;
            outb(reg_val, REG_EC_SEL);
            BSP_LOG_W("reg=0x%04x, reg_val=0x%02x", REG_EC_SEL, reg_val);
            reg_val = 0x21;
            outb(reg_val, REG_EC_PSU_RST);
            BSP_LOG_W("reg=0x%04x, reg_val=0x%02x", REG_EC_PSU_RST, reg_val);
            psu_reset_flag = 0;
            mutex_unlock(&cpld_data->access_lock);
        } else {
            return -EINVAL;
        }
    } else {
        printk(KERN_INFO "PSU is resetting... (ignore)\n");
        mutex_lock(&cpld_data->access_lock);
        mutex_unlock(&cpld_data->access_lock);
    }

    return count;
}

//SENSOR_DEVICE_ATTR - MB
static SENSOR_DEVICE_ATTR(board_id_0,        S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_ID_0);
static SENSOR_DEVICE_ATTR(board_id_1,        S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_ID_1);
static SENSOR_DEVICE_ATTR(mb_cpld_1_version,   S_IRUGO, read_cpld_callback, NULL, ATT_MB_CPLD_1_VERSION);
static SENSOR_DEVICE_ATTR(mb_cpld_1_version_h, S_IRUGO, read_mb_cpld_1_version_h, NULL, ATT_MB_CPLD_1_VERSION_H);
static SENSOR_DEVICE_ATTR(board_sku_id,      S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_SKU_ID);
static SENSOR_DEVICE_ATTR(board_hw_id,       S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_HW_ID);
static SENSOR_DEVICE_ATTR(board_id_type,     S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_ID_TYPE);
static SENSOR_DEVICE_ATTR(board_build_id,    S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_BUILD_ID);
static SENSOR_DEVICE_ATTR(board_deph_id,     S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_DEPH_ID);
static SENSOR_DEVICE_ATTR(board_ext_id,      S_IRUGO, read_cpld_callback, NULL, ATT_MB_BRD_EXT_ID);
static SENSOR_DEVICE_ATTR(mux_reset,         S_IRUGO | S_IWUSR, read_cpld_callback, write_mb_mux_reset, ATT_MB_MUX_RESET);
static SENSOR_DEVICE_ATTR(fan_status,        S_IRUGO, read_cpld_callback, NULL, ATT_MB_FAN_STATUS);
static SENSOR_DEVICE_ATTR(psu_status,        S_IRUGO, read_cpld_callback, NULL, ATT_MB_PSU_STATUS);
static SENSOR_DEVICE_ATTR(port_led_clear,    S_IRUGO | S_IWUSR, read_cpld_callback, write_cpld_callback, ATT_MB_PORT_LED_CLR);
static SENSOR_DEVICE_ATTR(led_sys,           S_IRUGO | S_IWUSR, read_cpld_callback, write_cpld_callback, ATT_MB_LED_SYS);
static SENSOR_DEVICE_ATTR(led_id,            S_IRUGO | S_IWUSR, read_cpld_callback, write_cpld_callback, ATT_MB_LED_ID);
static SENSOR_DEVICE_ATTR(led_poe,           S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_POE);
static SENSOR_DEVICE_ATTR(led_spd,           S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_SPD);
static SENSOR_DEVICE_ATTR(led_fan,           S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_FAN);
static SENSOR_DEVICE_ATTR(led_lnk,           S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_LNK);
static SENSOR_DEVICE_ATTR(led_pwr1,          S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_PWR1);
static SENSOR_DEVICE_ATTR(led_pwr0,          S_IRUGO, read_cpld_callback, NULL, ATT_MB_LED_PWR0);
static SENSOR_DEVICE_ATTR(power_en,          S_IRUGO | S_IWUSR, read_cpld_callback, write_cpld_callback, ATT_MB_POWER_EN);
//SENSOR_DEVICE_ATTR - EC
static SENSOR_DEVICE_ATTR(psu_reset,         S_IRUGO | S_IWUSR, NULL, write_ec_psu_reset, ATT_EC_PSU_RESET);

static struct attribute *mb_cpld_attrs[] = {
    &sensor_dev_attr_board_id_0.dev_attr.attr,
    &sensor_dev_attr_board_id_1.dev_attr.attr,
    &sensor_dev_attr_mb_cpld_1_version.dev_attr.attr,
	&sensor_dev_attr_mb_cpld_1_version_h.dev_attr.attr,
	&sensor_dev_attr_board_sku_id.dev_attr.attr,
	&sensor_dev_attr_board_hw_id.dev_attr.attr,
	&sensor_dev_attr_board_id_type.dev_attr.attr,
	&sensor_dev_attr_board_build_id.dev_attr.attr,
	&sensor_dev_attr_board_deph_id.dev_attr.attr,
	&sensor_dev_attr_board_ext_id.dev_attr.attr,
	&sensor_dev_attr_mux_reset.dev_attr.attr,
	&sensor_dev_attr_fan_status.dev_attr.attr,
	&sensor_dev_attr_psu_status.dev_attr.attr,
	&sensor_dev_attr_port_led_clear.dev_attr.attr,
	&sensor_dev_attr_led_sys.dev_attr.attr,
	&sensor_dev_attr_led_id.dev_attr.attr,
	&sensor_dev_attr_led_poe.dev_attr.attr,
	&sensor_dev_attr_led_spd.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&sensor_dev_attr_led_lnk.dev_attr.attr,
	&sensor_dev_attr_led_pwr0.dev_attr.attr,
	&sensor_dev_attr_led_pwr1.dev_attr.attr,
	&sensor_dev_attr_power_en.dev_attr.attr,
    NULL,
};

static struct attribute *ec_attrs[] = {
    &sensor_dev_attr_psu_reset.dev_attr.attr,
    NULL,
};

static struct attribute_group mb_cpld_attr_grp = {
    .name = "mb_cpld",
    .attrs = mb_cpld_attrs,
};

static struct attribute_group ec_attr_grp = {
    .name = "ec",
    .attrs = ec_attrs,
};

static void cpld_dev_release( struct device * dev)
{
    return;
}

static struct platform_device cpld_dev = {
    .name           = DRIVER_NAME,
    .id             = -1,
    .dev = {
                    .release = cpld_dev_release,
    }
};

static int cpld_drv_probe(struct platform_device *pdev)
{
    int i = 0, grp_num = 2;
    int err[5] = {0};
    struct attribute_group *grp;

    cpld_data = devm_kzalloc(&pdev->dev, sizeof(struct cpld_data_s),
                    GFP_KERNEL);
    if(!cpld_data)
        return -ENOMEM;

    mutex_init(&cpld_data->access_lock);

    for (i=0; i<grp_num; ++i) {
        switch (i) {
            case 0:
                grp = &mb_cpld_attr_grp;
                break;
            case 1:
                grp = &ec_attr_grp;
                break;
            default:
                break;
        }

        err[i] = sysfs_create_group(&pdev->dev.kobj, grp);
        if(err[i]) {
            printk(KERN_ERR "Cannot create sysfs for group %s\n", grp->name);
            goto exit;
        } else {
            continue;
        }
    }

    return 0;

exit:
    for (i=0; i<grp_num; ++i) {
        switch (i) {
            case 0:
                grp = &mb_cpld_attr_grp;
                break;
            case 1:
                grp = &ec_attr_grp;
                break;
            default:
                break;
        }

        sysfs_remove_group(&pdev->dev.kobj, grp);
        if(!err[i]) {
            //remove previous successful cases
            continue;
        } else {
            //remove first failed case, then return
            return err[i];
        }
    }
    return 0;
}

static int cpld_drv_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &mb_cpld_attr_grp);
    sysfs_remove_group(&pdev->dev.kobj, &ec_attr_grp);

    return 0;
}

static struct platform_driver cpld_drv = {
    .probe  = cpld_drv_probe,
    .remove = __exit_p(cpld_drv_remove),
    .driver = {
    .name   = DRIVER_NAME,
    },
};

int cpld_init(void)
{
    int err = 0;

    err = platform_driver_register(&cpld_drv);
    if(err) {
    	printk(KERN_ERR "%s(#%d): platform_driver_register failed(%d)\n",
                __func__, __LINE__, err);

    	return err;
    }

    err = platform_device_register(&cpld_dev);
    if(err) {
    	printk(KERN_ERR "%s(#%d): platform_device_register failed(%d)\n",
                __func__, __LINE__, err);
    	platform_driver_unregister(&cpld_drv);
    	return err;
    }

    return err;
}

void cpld_exit(void)
{
    platform_driver_unregister(&cpld_drv);
    platform_device_unregister(&cpld_dev);
}

MODULE_AUTHOR("<support@netbergtw.com>");
MODULE_DESCRIPTION("aurora-221 cpld driver");
MODULE_LICENSE("GPL");

module_init(cpld_init);
module_exit(cpld_exit);
