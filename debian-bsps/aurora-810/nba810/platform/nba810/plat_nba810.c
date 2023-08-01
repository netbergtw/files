/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform drivers for NBA810 32x400G switch.
 */

#include "plat_port.h"
#include "../plat.h"


static int nba810_port_present_get(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err;
    uint64_t tmp;

    // Case-1: Get all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_present_get_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Get one
    // <TBD> Implement real get one functions
    err = plat_port_present_get_all(plat_p, &tmp);
    if (err < 0) {
        return err;
    }
    *buf = 0;
    *buf = ((tmp >> (port_num - plat_port_first_num())) & 0x1);
    return 0;
}


static int nba810_port_lpmod_get(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err;
    uint64_t tmp;

    // Case-1: Get all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_lpmod_get_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Get one
    // <TBD> Implement real get one functions
    err = plat_port_lpmod_get_all(plat_p, &tmp);
    if (err < 0) {
        return err;
    }
    *buf = 0;
    *buf = ((tmp >> (port_num - plat_port_first_num())) & 0x1);
    return 0;
}


static int nba810_port_lpmod_set(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err = 0;
    int shift = 0;
    uint64_t ori_val = 0;
    uint64_t tmp_val = 0;
    uint64_t new_val = 0;

    // Case-1: Set all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_lpmod_set_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Set one
    // <TBD> Implement real set one functions
    err = plat_port_lpmod_get_all(plat_p, &ori_val);
    if (err < 0) {
        PLAT_ERR("plat_port_lpmod_get_all fail\n");
        return err;
    }
    shift = port_num - plat_port_first_num();
    tmp_val = ori_val;
    if (BITCHK((*buf), 0)) {
        new_val = BITSET(tmp_val, shift);
    } else {
        new_val = BITCLEAR(tmp_val, shift);
    }
    // Case-3.1: Setup the same value
    if (new_val == ori_val) {
        return 0;
    }
    // Case-3.2: Setup different value
    return plat_port_lpmod_set_all(plat_p, &new_val);
}


static int nba810_port_reset_get(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err;
    uint64_t tmp;

    // Case-1: Get all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_reset_get_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Get one
    // <TBD> Implement real get one functions
    err = plat_port_reset_get_all(plat_p, &tmp);
    if (err < 0) {
        return err;
    }
    *buf = 0;
    *buf = ((tmp >> (port_num - plat_port_first_num())) & 0x1);
    return 0;
}


static int nba810_port_reset_set(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err = 0;
    int shift = 0;
    uint64_t ori_val = 0;
    uint64_t tmp_val = 0;
    uint64_t new_val = 0;

    // Case-1: Set all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_reset_set_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Set one
    // <TBD> Implement real set one functions
    err = plat_port_reset_get_all(plat_p, &ori_val);
    if (err < 0) {
        PLAT_ERR("plat_port_lpmod_get_all fail\n");
        return err;
    }
    shift = port_num - plat_port_first_num();
    tmp_val = ori_val;
    if (BITCHK((*buf), 0)) {
        new_val = BITSET(tmp_val, shift);
    } else {
        new_val = BITCLEAR(tmp_val, shift);
    }
    // Case-3.1: Setup the same value
    if (new_val == ori_val) {
        return 0;
    }
    // Case-3.2: Setup different value
    return plat_port_reset_set_all(plat_p, &new_val);
}


static int nba810_port_rxlos_get(nb_platdrv_t *plat_p, uint64_t *buf, int port_num)
{
    int err;
    uint64_t tmp;

    // Case-1: Get all
    if (port_num == PLAT_CLIENT_REQ_ALL) {
        return plat_port_rxlos_get_all(plat_p, buf);
    }
    // Case-2: Input invalid
    err = plat_port_type_get(port_num);
    if (err < 0) {
        return err;
    }
    // Case-3: Get one
    // <TBD> Implement real get one functions
    err = plat_port_rxlos_get_all(plat_p, &tmp);
    if (err < 0) {
        return err;
    }
    *buf = 0;
    *buf = ((tmp >> (port_num - plat_port_first_num())) & 0x1);
    return 0;
}


static int nba810_port_i2c_get(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg)
{
    return plat_port_i2c_read(plat_p, buf, port_num, addr, reg);
}


static int nba810_port_i2c_set(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg)
{
    return plat_port_i2c_read(plat_p, buf, port_num, addr, reg);
}


static int nba810_port_xcvr_get(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg)
{
    return plat_port_xcvr_read(plat_p, buf, port_num, reg);
}


static int nba810_port_xcvr_set(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg)
{
    return plat_port_xcvr_write(plat_p, buf, port_num, reg);
}


static int nba810_led_port_get(nb_platdrv_t *plat_p, uint32_t *buf, int port_num)
{
    return plat_port_led_rgb_get(plat_p, buf, port_num);
}


static int nba810_led_port_set(nb_platdrv_t *plat_p, uint32_t *buf, int port_num)
{
    return plat_port_led_rgb_set(plat_p, buf, port_num);
}


void nba810_platform_free(nb_platdrv_t *plat_p)
{
    PLAT_DBG("... GO\n");
    plat_port_free(plat_p);
    PLAT_INFO("... OK\n");
}


int nba810_platform_init(nb_platdrv_t *plat_p)
{
    int err = 0;

    PLAT_DBG("... GO\n");

    err = plat_port_init(plat_p);
    if (err < 0) {
        PLAT_ERR("plat_port_init fail\n");
        goto err_plat_init_1;
    }
    // TBD: Get from plat_board sub-module
    snprintf(plat_p->plat_name, sizeof(plat_p->plat_name), "%s", "NBA810");

    PLAT_INFO("... OK\n");
    return 0;

err_plat_init_1:
    return err;
}


int get_platform_drv(nb_platdrv_t *drv)
{
    drv->plat_port_present_get = nba810_port_present_get;
    drv->plat_port_lpmod_get   = nba810_port_lpmod_get;
    drv->plat_port_lpmod_set   = nba810_port_lpmod_set;
    drv->plat_port_reset_get   = nba810_port_reset_get;
    drv->plat_port_reset_set   = nba810_port_reset_set;
    drv->plat_port_rxlos_get   = nba810_port_rxlos_get;
    drv->plat_port_i2c_get     = nba810_port_i2c_get;
    drv->plat_port_i2c_set     = nba810_port_i2c_set;
    drv->plat_port_xcvr_get    = nba810_port_xcvr_get;
    drv->plat_port_xcvr_set    = nba810_port_xcvr_set;
    drv->plat_led_port_get     = nba810_led_port_get;
    drv->plat_led_port_set     = nba810_led_port_set;
    drv->plat_init             = nba810_platform_init;
    drv->plat_free             = nba810_platform_free;
    
    return 0;
}


