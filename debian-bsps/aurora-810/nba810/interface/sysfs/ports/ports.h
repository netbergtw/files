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
