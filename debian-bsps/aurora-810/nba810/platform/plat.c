/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform drivers.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "plat.h"

int loglv = 0x6;
int hotswap = 0;

nb_platdrv_t *plat_p;


int nb_port_present_get(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: 
     *   => -1: get all ports info
     *   =>  #: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_present_get(plat_p, buf, port_num);
}


int nb_port_lpmod_get(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: 
     *   => -1: get all ports info
     *   =>  #: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_lpmod_get(plat_p, buf, port_num);
}


int nb_port_lpmod_set(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for input data
     * (2) port_num: 
     *   => -1: set all ports info
     *   =>  #: set no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_lpmod_set(plat_p, buf, port_num);
}


int nb_port_reset_get(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: 
     *   => -1: get all ports info
     *   =>  #: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_reset_get(plat_p, buf, port_num);
}


int nb_port_reset_set(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for input data
     * (2) port_num: 
     *   => -1: set all ports info
     *   =>  #: set no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_reset_set(plat_p, buf, port_num);
}


int nb_port_rxlos_get(uint64_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: 
     *   => -1: get all ports info
     *   =>  #: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_rxlos_get(plat_p, buf, port_num);
}


int nb_port_i2c_get(uint16_t *buf, int port_num, uint16_t addr, uint16_t reg)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: get no.# port 
     * (3) addr: I2C device address
     * (4) reg: Register offset
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_i2c_get(plat_p, buf, port_num, addr, reg);
}


int nb_port_i2c_set(uint16_t *buf, int port_num, uint16_t addr, uint16_t reg)
{
    /* Args
     * (1) *buf: for input data
     * (2) port_num: get no.# port 
     * (3) addr: I2C device address
     * (4) reg: Register offset
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_port_i2c_set(plat_p, buf, port_num, addr, reg);
}


int nb_port_xcvr_get(uint16_t *buf, int port_num, uint16_t reg)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: get no.# port 
     * (3) reg: Transceiver EEPROM register offset
     * 
     * TBD: transaction handler for request handling
     */
    int i, err;
    int rty = 3;
    int dly = 100;
    if (!plat_p->ready) return -EAGAIN;
    for (i=0; i<rty; i++) {
        err = plat_p->plat_port_xcvr_get(plat_p, buf, port_num, reg);
        if (err < 0) {
            msleep(dly);
            continue;
        }
        break;
    }
    return err;
}


int nb_port_xcvr_set(uint16_t *buf, int port_num, uint16_t reg)
{
    /* Args
     * (1) *buf: for input data
     * (2) port_num: get no.# port 
     * (3) reg: Transceiver EEPROM register offset
     * 
     * TBD: transaction handler for request handling
     */
    int i, err;
    int rty = 3;
    int dly = 100;
    if (!plat_p->ready) return -EAGAIN;
    for (i=0; i<rty; i++) {
        err = plat_p->plat_port_xcvr_set(plat_p, buf, port_num, reg);
        if (err < 0) {
            msleep(dly);
            continue;
        }
        break;
    }
    return err;
}


int nb_led_port_get(uint32_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for return data
     * (2) port_num: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_led_port_get(plat_p, buf, port_num);
}


int nb_led_port_set(uint32_t *buf, int port_num)
{
    /* Args
     * (1) *buf: for setup data
     * (2) port_num: get no.# port 
     * 
     * TBD: transaction handler for request handling
     */
    if (!plat_p->ready) return -EAGAIN;
    return plat_p->plat_led_port_set(plat_p, buf, port_num);
}


static void destory_platdrv_obj(void)
{
    if (plat_p) {
        plat_p->plat_free(plat_p);
        mutex_destroy(&plat_p->fpga_mutex->dev_mutex);
        kfree(plat_p->fpga_mutex);
        kfree(plat_p);
    }
}

static int create_platdrv_obj(void)
{
    int err = 0;

    plat_p = kzalloc(sizeof(*plat_p), GFP_KERNEL);
    if (!plat_p) {
        PLAT_ERR("kzalloc plat_p fail\n");
        return -ENOMEM;
    }
    plat_p->fpga_mutex = kzalloc(sizeof(plat_mutex_t), GFP_KERNEL);
    if (!plat_p->fpga_mutex) {
        PLAT_ERR("kzalloc fpga_mutex fail\n");
        return -ENOMEM;
    }
    plat_p->ready = false;
    plat_p->log_lv = loglv;
    err = get_platform_drv(plat_p);
    if (err < 0) {
        PLAT_ERR("get_platform_drv fail\n");
        return err;
    }
    return 0;
}


static void _show_ioexp_settings(char *item_str, int item_num, ioexp_t *io_p)
{
    int i;

    PLAT_DBG("------------------\n");
    PLAT_DBG("[IO Expander]: %s \n", item_str);
    PLAT_DBG(" item   bus   addr   reg \n");
    for (i=0; i<item_num; i++) {
        PLAT_DBG(" IN-%d   0x%02x  0x%02x  0x%02x\n",
                   i, io_p->in_bus[i], io_p->in_addr[i], io_p->in_reg[i]);
    }
    for (i=0; i<item_num; i++) {
        PLAT_DBG(" Out-%d  0x%02x  0x%02x  0x%02x\n",
                   i, io_p->in_bus[i], io_p->in_addr[i], io_p->in_reg[i]);
    }
}


static void _show_port_settings(void)
{
    int i, j;
    char tmp_str[128], new_str[8];
    port_t *port;

    for (i=0; i<plat_p->port_total; i++) {
        port = plat_p->port_list[i];

        PLAT_DBG("------------------\n");
        PLAT_DBG("[Port-%d] type:%d\n", port->num, port->type);
        PLAT_DBG(" - LED CTL_ADDR: 0x%x\n", port->led_ctl_addr);
        PLAT_DBG(" - LED RGB_ADDR: 0x%x\n", port->led_rgb_addr);
        PLAT_DBG(" - LED RGB_OFFS: 0x%x\n", port->led_rgb_offs);
        memset(tmp_str, '\0', sizeof(new_str));
        for (j=0; j<PLAT_PORT_LED_PERPORT_NUM; j++) {
            memset(new_str, '\0', sizeof(new_str));
            snprintf(new_str, sizeof(new_str), "0x%x ", port->led_grn_bit[j]);
            strcat(tmp_str, new_str);
        }
        PLAT_DBG(" - LED BITS_GRN: %s\n", tmp_str);
        memset(tmp_str, '\0', sizeof(new_str));
        for (j=0; j<PLAT_PORT_LED_PERPORT_NUM; j++) {
            memset(new_str, '\0', sizeof(new_str));
            snprintf(new_str, sizeof(new_str), "0x%x ", port->led_blu_bit[j]);
            strcat(tmp_str, new_str);
        }
        PLAT_DBG(" - LED BITS_BLU: %s\n", tmp_str);
        memset(tmp_str, '\0', sizeof(new_str));
        for (j=0; j<PLAT_PORT_LED_PERPORT_NUM; j++) {
            memset(new_str, '\0', sizeof(new_str));
            snprintf(new_str, sizeof(new_str), "0x%x ", port->led_red_bit[j]);
            strcat(tmp_str, new_str);
        }
        PLAT_DBG(" - LED BITS_RED: %s\n", tmp_str);
    }
}


static void show_platform_settings(void)
{
    int dw_pst = plat_p->ioexp_present_p->data_width;
    int dw_lpm = plat_p->ioexp_lpmod_p->data_width;
    int dw_rst = plat_p->ioexp_reset_p->data_width;
    int dw_rxl = plat_p->ioexp_rxlos_p->data_width;

    PLAT_DBG("------------------\n");
    PLAT_DBG("[Platform Info]\n");
    PLAT_DBG(" - Platform Name : %s\n", plat_p->plat_name);
    PLAT_DBG(" - Port Number   : %d\n", plat_p->port_total);
    PLAT_DBG("\n");

    _show_ioexp_settings("Present", dw_pst, plat_p->ioexp_present_p);
    _show_ioexp_settings("LPMod", dw_lpm, plat_p->ioexp_lpmod_p);
    _show_ioexp_settings("REST", dw_rst, plat_p->ioexp_reset_p);
    _show_ioexp_settings("RXLos", dw_rxl, plat_p->ioexp_rxlos_p);

    _show_port_settings();
}


static int __init nb_platform_init(void) 
{
    int err = 0;

    PLAT_INFO("... GO\n");
    
    err = create_platdrv_obj();
    if (err < 0) {
      PLAT_ERR("create_platdrv_obj fail\n");
      return err;
    }
    err = plat_p->plat_init(plat_p);
    if (err < 0) {
      PLAT_ERR("plat_p.init fail <err>:%d\n", err);
      return err;
    }
    show_platform_settings();
    plat_p->ready = true;

    PLAT_INFO("Log level settings: ERR=%d INFO=%d DBG=%d\n", 
               (loglv & 0x4)>>2, (loglv & 0x2)>>1, (loglv & 0x1) );
    PLAT_INFO("Version: %s\n", PLAT_DRV_VERSION);
    PLAT_INFO("... OK\n");
    return 0;
}

static void __exit nb_platform_exit(void)
{
    destory_platdrv_obj();
    PLAT_INFO("... OK\n");
}

/* EXPORT_SYMBOL for upper layer interface modules
 */
EXPORT_SYMBOL(nb_port_present_get);
EXPORT_SYMBOL(nb_port_lpmod_get);
EXPORT_SYMBOL(nb_port_lpmod_set);
EXPORT_SYMBOL(nb_port_reset_get);
EXPORT_SYMBOL(nb_port_reset_set);
EXPORT_SYMBOL(nb_port_rxlos_get);
EXPORT_SYMBOL(nb_port_i2c_get);
EXPORT_SYMBOL(nb_port_i2c_set);
EXPORT_SYMBOL(nb_port_xcvr_get);
EXPORT_SYMBOL(nb_port_xcvr_set);
EXPORT_SYMBOL(nb_led_port_get);
EXPORT_SYMBOL(nb_led_port_set);

module_init(nb_platform_init);
module_exit(nb_platform_exit);

module_param(loglv, int, 0644);
MODULE_PARM_DESC(loglv, "Module log level (all=0xf, err=0x4, info=0x2, dbg=0x1, off=0x0)");

module_param(hotswap, int, 0644);
MODULE_PARM_DESC(hotswap, "Module runtime reload option");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netberg");
MODULE_DESCRIPTION("nba810 Platform driver");
