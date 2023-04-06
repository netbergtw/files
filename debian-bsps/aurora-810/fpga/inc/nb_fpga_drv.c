#include "nb_fpga.h"

uint32_t port_i2c_chl_mask[PORT_NUM + 1] = {0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000020, 0x00000010, 0x00000080, 0x00000040};
volatile sn_fpga_regs_t *sn_fpga_mmap_addr;

int netberg_fpga_i2c_status_check(int channel) {
    int time_count = SN_FPGA_ACCESS_TIME_COUNT;
    while (sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status != SN_FPGA_STATUS_DONE && time_count) {
        if (sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel].control = SN_FPGA_MEDIA_CLEAR;
        }
        SN_FPGA_ACCESS_DELAY;
        time_count--;
    }

    if (!time_count) {
        if (sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == SN_FPGA_STATUS_BUSY) {
            sn_fpga_err("channel (%d) busy", channel);
            return SN_FPGA_STATUS_BUSY;
        } else if (sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel].status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_err("channel (%d) timeout", channel);
            return SN_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return SN_SUCCESS;
}

int led_status_check(int channel) {
    int time_count = SN_FPGA_ACCESS_TIME_COUNT;
    while (sn_fpga_mmap_addr->led_i2c_data[channel].status != SN_FPGA_STATUS_DONE && time_count) {
        if (sn_fpga_mmap_addr->led_i2c_data[channel].status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_mmap_addr->led_i2c_data[channel].control = SN_FPGA_LED_CLEAR;
            sn_fpga_err("channel (%d) time out,clear status and retry", channel);
        }
        SN_FPGA_ACCESS_DELAY;
        time_count--;
    }
    if (!time_count) {
        if (sn_fpga_mmap_addr->led_i2c_data[channel].status == SN_FPGA_STATUS_BUSY) {
            sn_fpga_err("channel (%d) busy", channel);
            return SN_FPGA_STATUS_BUSY;
        } else if (sn_fpga_mmap_addr->led_i2c_data[channel].status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_err("channel (%d) timeout", channel);
            return SN_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return SN_SUCCESS;
}

// change reuturn value for error detecting.
int sn_fpga_write_mem32(uint32_t offset, uint32_t data) {
    uint32_t *addr;
    addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + offset);
    if (!sn_fpga_mmap_addr || !addr) {
        return -SN_MEM_ERROR;
    }
    SN_FPGA_CMD_DELAY;
    *addr = data;
    return SN_SUCCESS;
}
// add @param uint32_t* data for passing data and change return value for error detecting.
int sn_fpga_read_mem32(uint32_t offset, uint32_t *data) {
    volatile uint32_t *addr;
    uint32_t tmp;
    addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + offset);
    if (!sn_fpga_mmap_addr || !addr) {
        return -SN_MEM_ERROR;
    }
    SN_FPGA_CMD_DELAY;
    tmp = *addr;
    *data = *addr;
    return SN_SUCCESS;
}

int netberg_fpga_write_i2c(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t data) {
    // 1.read 0x0008  check status
    int ret;
    ret = netberg_fpga_i2c_status_check(channel - 1);
    if (ret == SN_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        uint32_t i2c_addr_control = (i2c_addr << 16) + 0x00000002;
        uint32_t *addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + (SN_MEDIA_I2C_BASE + (SN_MEDIA_I2C_FORMAT_SIZE * (channel - 1))));
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].reg_addr = reg_addr;
        SN_FPGA_CMD_DELAY;
        // 3.write 0x000c Write data
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].w_data = data;
        SN_FPGA_CMD_DELAY;
        *addr = i2c_addr_control;
    } else {
        goto exit_fail;
    }

    // 5.read  0x0008  check status
    ret = netberg_fpga_i2c_status_check(channel - 1);
    if (ret == SN_FPGA_STATUS_DONE) {
        return SN_SUCCESS;
    } else {
        goto exit_fail;
    }

exit_fail:
    return ret;
}

int netberg_fpga_read_i2c(int channel, uint16_t i2c_addr, uint16_t reg_addr, uint16_t *data) {
    int ret;
    // 1.read 0x0008  check status
    ret = netberg_fpga_i2c_status_check(channel - 1);
    if (ret == SN_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        uint32_t i2c_addr_control = (i2c_addr << 16) + 0x00000001;
        uint32_t *addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + (SN_MEDIA_I2C_BASE + (SN_MEDIA_I2C_FORMAT_SIZE * (channel - 1))));

        sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].reg_addr = reg_addr;
        SN_FPGA_CMD_DELAY;
        // 3.write 0x0000 i2c_addr and Control
        *addr = i2c_addr_control;
        SN_FPGA_CMD_DELAY;
    } else {
        goto exit_fail;
    }

    // 4.read  0x0008 check status
    ret = netberg_fpga_i2c_status_check(channel - 1);
    if (ret == SN_FPGA_STATUS_DONE) {
        // 5.read  0x0010 Register data
        // bug: it needs to read twice.
        *data = sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].r_data;
        *data = sn_fpga_mmap_addr->netberg_fpga_i2c_data[channel - 1].r_data;
    } else {
        goto exit_fail;
    }
    return SN_SUCCESS;

exit_fail:
    return ret;
}

int netberg_fpga_led_status_check(void) {
    int time_count = SN_FPGA_ACCESS_TIME_COUNT;
    while (sn_fpga_mmap_addr->netberg_fpga_led_data.status != SN_FPGA_STATUS_DONE && time_count) {
        if (sn_fpga_mmap_addr->netberg_fpga_led_data.status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_mmap_addr->netberg_fpga_led_data.control = SN_FPGA_MEDIA_CLEAR;
            sn_fpga_err("NETBERG_FPGA LED time out,clear status and retry");
        }
        SN_FPGA_ACCESS_DELAY;
        time_count--;
    }

    if (!time_count) {
        if (sn_fpga_mmap_addr->netberg_fpga_led_data.status == SN_FPGA_STATUS_BUSY) {
            sn_fpga_err("NETBERG_FPGA LED busy");
            return SN_FPGA_STATUS_BUSY;
        } else if (sn_fpga_mmap_addr->netberg_fpga_led_data.status == SN_FPGA_STATUS_TIMEOUT) {
            sn_fpga_err("NETBERG_FPGA LED Time out");
            return SN_FPGA_STATUS_TIMEOUT;
        }
        return -1;
    }
    return SN_SUCCESS;
}

int netberg_fpga_QSFP_Read(int QSFP_channel, uint16_t Reg_addr, uint8_t *R_data) {
    int ret;
    uint32_t channel;
    uint16_t i2c_mux_addr;
    volatile uint8_t tmp;
    int i2c_channel;
    uint32_t *i2c_addr;
    if ((QSFP_channel > 0) && (QSFP_channel <= 8)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP1_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP1_MUX_ADDR;
    } else if ((QSFP_channel > 8) && (QSFP_channel <= 16)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP2_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP2_MUX_ADDR;
    } else if ((QSFP_channel > 16) && (QSFP_channel <= 24)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP3_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP3_MUX_ADDR;
    } else if ((QSFP_channel > 24) && (QSFP_channel <= 32)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP4_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP4_MUX_ADDR;
    } else {
        ret = -EINVAL;
        goto exit_fail;
    }

    i2c_addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + SN_MEDIA_I2C_BASE + (SN_MEDIA_I2C_FORMAT_SIZE * i2c_channel));
    ret = netberg_fpga_i2c_status_check(i2c_channel);
    if (ret == SN_FPGA_STATUS_DONE) {
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = channel;
        *i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
        ret = netberg_fpga_i2c_status_check(i2c_channel);
        if (ret != SN_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }

    ret = netberg_fpga_i2c_status_check(i2c_channel);
    if (ret == SN_FPGA_STATUS_DONE) {
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = Reg_addr;
        *i2c_addr = 0x00500001;
        // sn_fpga_mmap_addr->media_i2c_data[channel].status = SN_FPGA_STATUS_BUSY;
        ret = netberg_fpga_i2c_status_check(i2c_channel);
        if (ret != SN_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }
    ret = netberg_fpga_i2c_status_check(i2c_channel);
    if (ret == SN_FPGA_STATUS_DONE) {
        // 2.write 0x0004 Register address
        tmp = sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].r_data & 0xff;
        tmp = sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].r_data & 0xff;

        *R_data = tmp;
        return SN_SUCCESS;
    } else {
        goto exit_fail;
    }
exit_fail:
    return ret;
}

int netberg_fpga_QSFP_Write(int QSFP_channel, uint16_t Reg_addr, uint16_t w_data) {
    int ret;
    uint32_t channel;
    uint16_t i2c_mux_addr;
    int i2c_channel;
    uint32_t *i2c_addr;
    if ((QSFP_channel > 0) && (QSFP_channel <= 8)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP1_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP1_MUX_ADDR;
    } else if ((QSFP_channel > 8) && (QSFP_channel <= 16)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP2_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP2_MUX_ADDR;
    } else if ((QSFP_channel > 16) && (QSFP_channel <= 24)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP3_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP3_MUX_ADDR;
    } else if ((QSFP_channel > 24) && (QSFP_channel <= 32)) {
        channel = port_i2c_chl_mask[QSFP_channel];
        i2c_channel = QSFP4_I2C_CHL - 1;
        i2c_mux_addr = SN_FPGA_QSFP4_MUX_ADDR;
    } else {
        ret = -EINVAL;

        goto exit_fail;
    }

    i2c_addr = (uint32_t *)((uint8_t *)sn_fpga_mmap_addr + SN_MEDIA_I2C_BASE + (SN_MEDIA_I2C_FORMAT_SIZE * i2c_channel));
    ret = netberg_fpga_i2c_status_check(i2c_channel);
    if (ret == SN_FPGA_STATUS_DONE) {
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = channel;
        *i2c_addr = (i2c_mux_addr << 16) + 0x00000004;
        ret = netberg_fpga_i2c_status_check(i2c_channel);
        if (ret != SN_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
    } else {
        goto exit_fail;
    }

    ret = netberg_fpga_i2c_status_check(i2c_channel);
    if (ret == SN_FPGA_STATUS_DONE) {
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].w_data = w_data;
        sn_fpga_mmap_addr->netberg_fpga_i2c_data[i2c_channel].reg_addr = Reg_addr;
        ret = netberg_fpga_i2c_status_check(i2c_channel);
        if (ret != SN_FPGA_STATUS_DONE) {
            goto exit_fail;
        }
        *i2c_addr = 0x00500002;
        ret = netberg_fpga_i2c_status_check(i2c_channel);
        if (ret != SN_FPGA_STATUS_DONE) {
            goto exit_fail;
        } else {
            return SN_SUCCESS;
        }
    } else {
        goto exit_fail;
    }
exit_fail:
    return ret;
}
