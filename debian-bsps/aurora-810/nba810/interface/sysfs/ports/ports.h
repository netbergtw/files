/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PORTS_H_
#define _PORTS_H_

#include "../../../includes/base_nb.h"

#define PORT_ERR(fmt, args...)  NB_LOG_ERR("[PORTS]", fmt, ##args)
#define PORT_INFO(fmt, args...) NB_LOG_INFO("[PORTS]", fmt, ##args)
#define PORT_DBG(fmt, args...)  NB_LOG_DBG("[PORTS]", fmt, ##args)


#define PORT_DRV_VERSION   "1.0.0"

/* TBD: Get from platform driver
 */
#define PORT_VAL_SFP_MIN 0
#define PORT_VAL_SFP_MAX 0
#define PORT_VAL_QSFP_MIN 1
#define PORT_VAL_QSFP_MAX 32
#define PORT_VAL_TOTAL_NUM 32

#define PORT_STR_CLASS_NAME "ports"
#define PORT_STR_L3DIR_I2C_NAME "i2c"

#define QSFP_REG_IDENTIFIER             0
#define QSFP_REG_PAGE_IMPLEMENTED       2
#define QSFP_REG_PAGE_SELECT            127

#define QSFP_VAL_ID_QSFP_Plus           0x0d
#define QSFP_VAL_ID_QSFP_28             0x11
#define QSFP_VAL_ID_QSFP_DD             0x18
#define QSFP_VAL_ID_QSFP_CMIS           0x1E
#define QSFP_VAL_PAGE_SIZE              128
#define QSFP_VAL_SFF_UPPAGE_TOTAL       4
#define QSFP_VAL_CMIS_UPPAGE_TOTAL      256
#define QSFP_VAL_EEPROM_SIZE_UNPAGED    (2 * QSFP_VAL_PAGE_SIZE)
#define QSFP_VAL_EEPROM_SIZE_PAGED_SFF  ((1 + QSFP_VAL_SFF_UPPAGE_TOTAL) * QSFP_VAL_PAGE_SIZE)
#define QSFP_VAL_EEPROM_SIZE_PAGED_CMIS ((1 + QSFP_VAL_CMIS_UPPAGE_TOTAL) * QSFP_VAL_PAGE_SIZE)
#define QSFP_VAL_FLAT_MEM_BIT_SFF       (1<<2)
#define QSFP_VAL_FLAT_MEM_BIT_CMIS      (1<<7)


struct subdev_t {
    int port_num;
    dev_t devt_num;
    struct device *dev_p;
    struct subdev_t *subdev_p;
};

struct ports_t {
    dev_t major;
    int sfp_min;
    int sfp_max;
    int qsfp_min;
    int qsfp_max;

    int port_total;
    int subdev_total;
    struct subdev_t **subdev_p;
};

#endif /* _PORTS_H_ */
