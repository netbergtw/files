/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform drivers for NBA810 32x400G switch.
 */

#include "plat_fpga.h"


struct mutex fpga_lock;

uint32_t port_i2c_chl_mask[PORT_NUM + 1] = {0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000020, 0x00000010, 0x00000080, 0x00000040};
volatile nb_fpga_regs_t *nb_fpga_mmap_addr;

int nb_fpga_i2c_check(int channel) {
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while (nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status != NB_FPGA_STATUS_DONE && time_count) {
        if (nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == NB_FPGA_STATUS_TIMEOUT) {
            nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel].control = NB_FPGA_MEDIA_CLEAR;
        }
        NB_FPGA_ACCESS_DELAY;
        time_count--;
    }

    if (!time_count) {
        if (nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == NB_FPGA_STATUS_BUSY) {
            PLAT_ERR("channel (%d) busy", channel);
            return NB_FPGA_STATUS_BUSY;
        } else if (nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == NB_FPGA_STATUS_TIMEOUT) {
            PLAT_ERR("channel (%d) timeout", channel);
            return NB_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return NB_SUCCESS;
}

int led_status_check(int channel) {
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while (nb_fpga_mmap_addr->led_i2c_data[channel].status != NB_FPGA_STATUS_DONE && time_count) {
        if (nb_fpga_mmap_addr->led_i2c_data[channel].status == NB_FPGA_STATUS_TIMEOUT) {
            nb_fpga_mmap_addr->led_i2c_data[channel].control = NB_FPGA_LED_CLEAR;
            PLAT_ERR("channel (%d) time out,clear status and retry", channel);
        }
        NB_FPGA_ACCESS_DELAY;
        time_count--;
    }
    if (!time_count) {
        if (nb_fpga_mmap_addr->led_i2c_data[channel].status == NB_FPGA_STATUS_BUSY) {
            PLAT_ERR("channel (%d) busy", channel);
            return NB_FPGA_STATUS_BUSY;
        } else if (nb_fpga_mmap_addr->led_i2c_data[channel].status == NB_FPGA_STATUS_TIMEOUT) {
            PLAT_ERR("channel (%d) timeout", channel);
            return NB_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return NB_SUCCESS;
}

// change reuturn value for error detecting.
int nb_fpga_mem32_write(uint32_t offset, uint32_t data) {
    uint32_t *addr;

    NB_FPGA_CMD_DELAY;
    addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + offset);
    if (!nb_fpga_mmap_addr || !addr) {
        return -EIO;
    }
    NB_FPGA_CMD_DELAY;
    *addr = data;
    return 0;
}
// add @param uint32_t* data for passing data and change return value for error detecting.
int nb_fpga_mem32_read(uint32_t offset, uint32_t *data) {
    volatile uint32_t *addr;
    uint32_t tmp;

    NB_FPGA_CMD_DELAY;
    addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + offset);
    if (!nb_fpga_mmap_addr || !addr) {
        return -EIO;
    }
    NB_FPGA_CMD_DELAY;
    tmp = *addr;
    *data = *addr;
    return 0;
}

int nb_fpga_i2c_write(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t data) {
    // 1.read 0x0008  check status
    int ret;
    ret = nb_fpga_i2c_check(channel - 1);
    if (ret == NB_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        uint32_t i2c_addr_control = (i2c_addr << 16) + 0x00000002;
        uint32_t *addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + (NB_MEDIA_I2C_BASE + (NB_MEDIA_I2C_FORMAT_SIZE * (channel - 1))));
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].reg_addr = reg_addr;
        NB_FPGA_CMD_DELAY;
        // 3.write 0x000c Write data
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].w_data = data;
        NB_FPGA_CMD_DELAY;
        *addr = i2c_addr_control;
    } else {
        goto exit_fail;
    }

    // 5.read  0x0008  check status
    ret = nb_fpga_i2c_check(channel - 1);
    if (ret == NB_FPGA_STATUS_DONE) {
        return 0;
    } else {
        goto exit_fail;
    }

exit_fail:
    return ret;
}

int nb_fpga_i2c_read(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t *data) {
    int ret;
    // 1.read 0x0008  check status
    ret = nb_fpga_i2c_check(channel - 1);
    if (ret == NB_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        uint32_t i2c_addr_control = (i2c_addr << 16) + 0x00000001;
        uint32_t *addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + (NB_MEDIA_I2C_BASE + (NB_MEDIA_I2C_FORMAT_SIZE * (channel - 1))));

        nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].reg_addr = reg_addr;
        NB_FPGA_CMD_DELAY;
        // 3.write 0x0000 i2c_addr and Control
        *addr = i2c_addr_control;
        NB_FPGA_CMD_DELAY;
    } else {
        goto exit_fail;
    }

    // 4.read  0x0008 check status
    ret = nb_fpga_i2c_check(channel - 1);
    if (ret == NB_FPGA_STATUS_DONE) {
        // 5.read  0x0010 Register data
        // bug: it needs to read twice.
        *data = nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].r_data;
        *data = nb_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].r_data;
    } else {
        goto exit_fail;
    }
    return NB_SUCCESS;

exit_fail:
    return ret;
}

int nb_fpga_led_check(void) {
    int time_count = NB_FPGA_ACCESS_TIME_COUNT;
    while (nb_fpga_mmap_addr->netberg_fpga_led_data.status != NB_FPGA_STATUS_DONE && time_count) {
        if (nb_fpga_mmap_addr->netberg_fpga_led_data.status == NB_FPGA_STATUS_TIMEOUT) {
            nb_fpga_mmap_addr->netberg_fpga_led_data.control = NB_FPGA_MEDIA_CLEAR;
            PLAT_ERR("NETBERG_FPGA LED time out,clear status and retry");
        }
        NB_FPGA_ACCESS_DELAY;
        time_count--;
    }

    if (!time_count) {
        if (nb_fpga_mmap_addr->netberg_fpga_led_data.status == NB_FPGA_STATUS_BUSY) {
            PLAT_ERR("NETBERG_FPGA LED busy");
            return NB_FPGA_STATUS_BUSY;
        } else if (nb_fpga_mmap_addr->netberg_fpga_led_data.status == NB_FPGA_STATUS_TIMEOUT) {
            PLAT_ERR("NETBERG_FPGA LED Time out");
            return NB_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return NB_SUCCESS;
}


int nb_fpga_led_read(int i2c_bus, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t *data)
{
    // 1.read 0x0008  check status
    if( led_status_check(i2c_bus) != NB_SUCCESS) {
        return -1;
    }
    // 2.write 0x0004 Register address
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].reg_addr = reg_addr;
    NB_FPGA_CMD_DELAY;
    // 3.write 0x0000 i2c_addr and Control
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].i2c_addr = i2c_addr;
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].control  = NB_FPGA_LED_READ;
    NB_FPGA_CMD_DELAY;
    // 4.read  0x0008 check status
    if( led_status_check(i2c_bus) != NB_SUCCESS) {
        return -1;
    }
    *data = nb_fpga_mmap_addr->led_i2c_data[i2c_bus].r_data;
    return 0;
}


int nb_fpga_led_write(int i2c_bus, u_int16_t i2c_addr, u_int16_t reg_addr, u_int16_t data)
{
    // 1.read 0x0008  check status
    if( led_status_check(i2c_bus) != NB_SUCCESS) {
        return -1;
    }
    // 2.write 0x0004 Register address
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].reg_addr = reg_addr;
    NB_FPGA_CMD_DELAY;
    // 3.write 0x000c Write data
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].w_data = data;  	
    NB_FPGA_CMD_DELAY;
    // 4.write 0x0000 i2c_addr and Control
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].i2c_addr = i2c_addr;
    nb_fpga_mmap_addr->led_i2c_data[i2c_bus].control = NB_FPGA_LED_WRITE;
    NB_FPGA_CMD_DELAY;
    // 5.read  0x0008  check status
    if( led_status_check(i2c_bus) != NB_SUCCESS) {
        return -1;
    }
    return 0;
}


int nb_fpga_qsfp_read(int QSFP_channel, uint16_t Reg_addr, uint8_t *R_data) {
    int ret;
    uint32_t channel;
    uint16_t i2c_mux_addr;
    volatile uint8_t tmp;
    int i2c_channel;
    uint32_t *i2c_addr;
    if ((QSFP_channel > 0) && (QSFP_channel <= 8)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP1_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP1_MUX_ADDR;
    } else if ((QSFP_channel > 8) && (QSFP_channel <= 16)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP2_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP2_MUX_ADDR;
    } else if ((QSFP_channel > 16) && (QSFP_channel <= 24)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP3_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP3_MUX_ADDR;
    } else if ((QSFP_channel > 24) && (QSFP_channel <= 32)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP4_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP4_MUX_ADDR;
    } else {
        ret = -EINVAL;
        goto exit_fail;
    }

    i2c_addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + NB_MEDIA_I2C_BASE + (NB_MEDIA_I2C_FORMAT_SIZE * i2c_channel));
    ret = nb_fpga_i2c_check(i2c_channel);
    if (ret == NB_FPGA_STATUS_DONE) {
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = channel;
        *i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
        ret = nb_fpga_i2c_check(i2c_channel);
        if (ret != NB_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }

    ret = nb_fpga_i2c_check(i2c_channel);
    if (ret == NB_FPGA_STATUS_DONE) {
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = Reg_addr;
        *i2c_addr = 0x00500001;
        // nb_fpga_mmap_addr->media_i2c_data[channel].status = NB_FPGA_STATUS_BUSY;
        ret = nb_fpga_i2c_check(i2c_channel);
        if (ret != NB_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }
    ret = nb_fpga_i2c_check(i2c_channel);
    if (ret == NB_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        tmp = nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].r_data & 0xff;
        tmp = nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].r_data & 0xff;

        *R_data = tmp;
        return NB_SUCCESS;
    } else {
        goto exit_fail;
    }
exit_fail:
    return ret;
}

int nb_fpga_qsfp_write(int QSFP_channel, uint16_t Reg_addr, uint16_t w_data) {
    int ret;
    uint32_t channel;
    uint16_t i2c_mux_addr;
    int i2c_channel;
    uint32_t *i2c_addr;
    if ((QSFP_channel > 0) && (QSFP_channel <= 8)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP1_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP1_MUX_ADDR;
    } else if ((QSFP_channel > 8) && (QSFP_channel <= 16)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP2_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP2_MUX_ADDR;
    } else if ((QSFP_channel > 16) && (QSFP_channel <= 24)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP3_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP3_MUX_ADDR;
    } else if ((QSFP_channel > 24) && (QSFP_channel <= 32)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP4_I2C_CHL - 1;
        i2c_mux_addr = NB_FPGA_QSFP4_MUX_ADDR;
    } else {
        ret = -EINVAL;

        goto exit_fail;
    }

    i2c_addr = (uint32_t *)((uint8_t *)nb_fpga_mmap_addr + NB_MEDIA_I2C_BASE + (NB_MEDIA_I2C_FORMAT_SIZE * i2c_channel));
    ret = nb_fpga_i2c_check(i2c_channel);
    if (ret == NB_FPGA_STATUS_DONE) {
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = channel;
        *i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
        ret = nb_fpga_i2c_check(i2c_channel);
        if (ret != NB_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }

    ret = nb_fpga_i2c_check(i2c_channel);
    if (ret == NB_FPGA_STATUS_DONE) {
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].w_data = w_data;
        nb_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = Reg_addr;
        ret = nb_fpga_i2c_check(i2c_channel);
        if (ret != NB_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
        *i2c_addr = 0x00500002;
        ret = nb_fpga_i2c_check(i2c_channel);
        if (ret != NB_FPGA_STATUS_DONE) {
            goto exit_fail;
        } else {
            return NB_SUCCESS;
        }
    } else {
        goto exit_fail;
    }
exit_fail:
    return ret;
}

void nb_fpga_lock_init(nb_platdrv_t *plt_p) 
{
    mutex_init(&(plt_p->fpga_mutex->dev_mutex));
}


void nb_fpga_lock_destroy(nb_platdrv_t *plt_p)
{
    mutex_destroy(&(plt_p->fpga_mutex->dev_mutex));
}


bool nb_fpga_lock(plat_mutex_t *mtx_p)
{
    mutex_lock(&(mtx_p->dev_mutex));
    return true;
}

// bool nb_fpga_lock(plat_mutex_t *mtx_p)
// {
//     int retry = 300;
//     int wait_us = 10;

//     while (retry > 0) {
//         if (mutex_trylock(&(mtx_p->dev_mutex))) {
//             return true;
//         }
//         retry--;
//         udelay(wait_us);
//     }
//     PLAT_DBG("retry fail\n");
//     return false;
// }


void nb_fpga_unlock(plat_mutex_t *mtx_p)
{
    mutex_unlock(&(mtx_p->dev_mutex));
}
