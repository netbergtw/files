#include "platform.h"

#include <linux/errno.h>

int platform_probe(uint32_t pci_addr) {
    ssize_t ret;
    PRINT_INFO("Netberg PCI MEM address : %#x \n", pci_addr);
    sn_fpga_mmap_addr = ioremap(pci_addr, SN_FPGA_MEM_SIZE);

    if (sn_fpga_mmap_addr == NULL) {
        PRINT_ERR("FPGA PCI BAR0 mmap failed \n");
        return SN_FPGA_MMAP_ALLC_FAIL;
    }
    PRINT_INFO("FPGA PCI BAR0 mmapped %#llX size %#x \n", (u64)sn_fpga_mmap_addr,
               SN_FPGA_MEM_SIZE);

    PRINT_INFO("Set FPGA delay time\n");
    sn_fpga_write_mem32(0x00000114, 0x00000200);
    mdelay(500);
    sn_fpga_write_mem32(0x00000118, 0x00000064);
    mdelay(500);
    ret = sn_i2c_io_init();
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return ret;
    }
    return SN_SUCCESS;
}
int platform_init() {
    ssize_t ret;
    PRINT_INFO("Mutex lock init");

    ret = sn_init_lock();
    if (ret != SN_SUCCESS) {
        PRINT_ERR("%zd", ret);
        return ret;
    }
    return SN_SUCCESS;
}
int platform_exit() {
    if (sn_fpga_mmap_addr) {
        iounmap(sn_fpga_mmap_addr);
    }
    return SN_SUCCESS;
}

// I2C common utils
int sn_i2c_io_init() {
    uint8_t i2c_addr, i2c_io_expader_addr;
    ssize_t ret;
    // init lpmode
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    for (i2c_addr = LP_MODE_I2C_ADDR_1; i2c_addr < LP_MODE_I2C_ADDR_1 + LP_MODE_I2C_ADDR_NUM; i2c_addr++) {
        for (i2c_io_expader_addr = IO_EXPANDER_PO_INVERSION_PORT0_ADDR_0; i2c_io_expader_addr < IO_EXPANDER_CFG_PORT0_ADDR_0 + LP_MODE_IO_NUM; i2c_io_expader_addr++) {
            ret = netberg_fpga_write_i2c(LP_MODE_I2C_CHL, i2c_addr,
                                            i2c_io_expader_addr, IO_OUPUT);
            if (ret != SN_SUCCESS) {
                goto exit_unlock;
            }
        }
    }
    // init reset
    for (i2c_addr = RESET_I2C_ADDR_1; i2c_addr < RESET_I2C_ADDR_1 + RESET_I2C_ADDR_NUM; i2c_addr++) {
        for (i2c_io_expader_addr = IO_EXPANDER_CFG_PORT0_ADDR_0; i2c_io_expader_addr < IO_EXPANDER_CFG_PORT0_ADDR_0 + RESET_IO_NUM; i2c_io_expader_addr++) {
            ret = netberg_fpga_write_i2c(RESET_I2C_CHL, i2c_addr,
                                            i2c_io_expader_addr, IO_OUPUT);
            if (ret != SN_SUCCESS) {
                goto exit_unlock;
            }
        }
    }
    // init present
    for (i2c_addr = PRESENT_I2C_ADDR_1; i2c_addr < PRESENT_I2C_ADDR_1 + PRESENT_I2C_ADDR_NUM; i2c_addr++) {
        for (i2c_io_expader_addr = IO_EXPANDER_PO_INVERSION_PORT0_ADDR_0; i2c_io_expader_addr < IO_EXPANDER_PO_INVERSION_PORT0_ADDR_0 + PRESENT_IO_NUM; i2c_io_expader_addr++) {
            ret = netberg_fpga_write_i2c(PRESENT_I2C_CHL, i2c_addr,
                                            i2c_io_expader_addr, IO_INPUT);
            if (ret != SN_SUCCESS) {
                goto exit_unlock;
            }
        }
    }
    // init rx loss
    for (i2c_addr = RX_LOSS_I2C_ADDR_1; i2c_addr < RX_LOSS_I2C_ADDR_1 + RX_LOSS_I2C_ADDR_NUM; i2c_addr++) {
        for (i2c_io_expader_addr = IO_EXPANDER_CFG_PORT0_ADDR_0; i2c_io_expader_addr < IO_EXPANDER_CFG_PORT0_ADDR_0 + RX_LOSS_IO_NUM; i2c_io_expader_addr++) {
            ret = netberg_fpga_write_i2c(RX_LOSS_I2C_CHL, i2c_addr,
                                            i2c_io_expader_addr, IO_INPUT);
            if (ret != SN_SUCCESS) {
                goto exit_unlock;
            }
        }
    }
    sn_unlock();
    return SN_SUCCESS;
exit_unlock:
    sn_unlock();
exit_fail:
    return EINVAL;
}

// FPGA
int sn_fpga_version_read(uint32_t *version) {
    ssize_t ret;
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = sn_fpga_read_mem32(SN_FPGA_VERSION_MEM_OFFSET, version);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        ret = SN_MEM_ERROR;
        goto exit_fail;
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}

/*
There are 32 ports on platform and divide into 4 group, 8 ports for each group.
port_i2c_chl_mask array is for extracting the port "presnt" "lpmode" "reset"
and "rxloss" from the 8 bits value of netberg_fpga_read_i2c.
*/
// Present
int sn_port_read_present(port_t port, bool *present) {
    ssize_t ret;
    uint16_t val;
    addr_t addr;
    ret = get_port_present_addr(port.num, &addr);
    if (ret != SN_SUCCESS) {
        ret = EINVAL;
        goto exit_fail;
    }
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = netberg_fpga_read_i2c(PRESENT_I2C_CHL, addr.i2c_addr,
                                   addr.reg_addr, &val);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    *present = (val & port_i2c_chl_mask[port.num]) ? true : false;
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int sn_port_read_all_present(uint32_t *all_present) {
    ssize_t ret;
    uint32_t port, data;
    uint32_t bitswap1, bitswap2;
    // only need to read the first port of each mux. it will return a 8 bit
    // value contain 8 ports' present info.
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    for (port = 1; port < PORT_NUM; port += 8) {
        addr_t addr;
        ret = get_port_present_addr(port, &addr);
        if (ret != SN_SUCCESS) {
            goto exit_unlock;
        }

        ret = netberg_fpga_read_i2c(PRESENT_I2C_CHL, addr.i2c_addr,
                                       addr.reg_addr, (uint16_t *)&data);
        if (ret != SN_SUCCESS) {
            goto exit_unlock;
        }

        *all_present |= (data & 0x000000ff) << (port - 1);
        // swap bit order for correct portmapping which refferences to port_i2c_chl_mask[].
    }
    sn_unlock();
    bitswap1 = (*all_present << 1) & (0xa0aaaa00);
    bitswap2 = (*all_present >> 1) & (0x50555500);
    *all_present = (*all_present & 0x0f0000ff) + bitswap1 + bitswap2;
    return SN_SUCCESS;
exit_unlock:
    sn_unlock();
exit_fail:
    return ret;
}
int get_port_present_addr(int port, addr_t *addr) {
    switch ((port - 1) / 8) {
        case 0:
            addr->i2c_addr = PRESENT_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr->i2c_addr = PRESENT_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr->i2c_addr = PRESENT_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr->i2c_addr = PRESENT_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        default:
            return EINVAL;
            break;
    }

    return SN_SUCCESS;
}

// LED
/*
RBG=0:None,1:Green,2:Blue,3:Cyan-blue,RBG=4:Red,5:Yellow,6:Purple,7:White
one port
*/
int sn_port_read_led(port_t port, uint8_t *data) {
    ssize_t ret;
    int shift_bit;
    uint32_t port_group_addr, leddata;
    port_group_addr = get_led_addr(port.num);

    ret = get_port_led_value(port_group_addr, &leddata);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    shift_bit = ((port.num - 1) % 4) * 8;
    *data = (leddata >> shift_bit) & 0xf;
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int sn_port_write_led(port_t port, uint32_t data) {
    ssize_t ret;
    uint32_t leddata, port_group_addr;
    uint8_t shift_bit;

    if (data > LED_RGB_NUM || data < 0) {
        return EINVAL;
    }

    port_group_addr = get_led_addr(port.num);

    ret = get_port_led_value(port_group_addr, &leddata);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    /*
    get_port_led_value() will return 32 bit data, 0xff|ff|ff|ff,
    which correspond to port4|port3|port2|por1 .
    */
    shift_bit = ((port.num - 1) % 4) * 8;
    /*
    the port has two LED, both of them will be set together. 4 bits for
    each LED.
    */
    data = (data << 4) | data;

    // to preserve other port's LED
    leddata &= (0xff << shift_bit) ^ 0xffffffff;

    leddata |= data << shift_bit;
    ret = write_port_led_value(port_group_addr, leddata);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }

    return SN_SUCCESS;
exit_fail:
    return ret;
}
int get_port_led_value(uint32_t port_group_addr, uint32_t *data) {
    ssize_t ret;
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = sn_fpga_read_mem32(port_group_addr, data);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    sn_unlock();
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int write_port_led_value(uint32_t port_group_addr, uint32_t data) {
    ssize_t ret;
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;

    ret = sn_fpga_write_mem32(port_group_addr, data);
    if (ret != SN_SUCCESS) {
        goto exit_unlock;
    }
    // to enable led write on fpga.
    ret = sn_fpga_write_mem32(LED_CTL_I2C_ADDR, 0x01);
    if (ret != SN_SUCCESS) {
        goto exit_unlock;
    }
    sn_unlock();
    return SN_SUCCESS;
exit_unlock:
    sn_unlock();
exit_fail:
    return ret;
}
uint32_t get_led_addr(int port_num) {
    uint32_t port_group_addr;
    port_group_addr = LED_RGB_I2C_ADDR + ((port_num - 1) / 4) * 4;
    return port_group_addr;
}

// EEPROM
int sn_port_read_eeprom(port_t port, uint8_t *buff, uint16_t offset, uint16_t len) {
    ssize_t ret;
    uint16_t eeprom_byte;

    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    if (offset + len > 256) {
        ret = -EINVAL;
        goto exit_unlock;
    }
    for (eeprom_byte = offset; eeprom_byte < offset + len; eeprom_byte++) {
        ret = netberg_fpga_QSFP_Read(port.num, eeprom_byte, (buff + eeprom_byte));
        if (ret != SN_SUCCESS) {
            goto exit_unlock;
        }
    }

    sn_unlock();
    return SN_SUCCESS;

exit_unlock:
    sn_unlock();
exit_fail:
    return ret;
}
int sn_port_write_eeprom(port_t port, uint8_t *buff, uint16_t offset, uint16_t len) {
    ssize_t ret;
    uint16_t eeprom_byte, write_data;
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    for (eeprom_byte = offset; eeprom_byte < offset + len; eeprom_byte++) {
        write_data = buff[eeprom_byte];

        ret = netberg_fpga_QSFP_Write(port.num, eeprom_byte, write_data);
        if (ret != SN_SUCCESS) {
            goto exit_unlock;
        }
    }
    sn_unlock();
    return SN_SUCCESS;

exit_unlock:
    sn_unlock();
exit_fail:
    return ret;
}
// LPMode
int sn_port_read_lpmode(port_t port, bool *lpmode) {
    ssize_t ret;
    uint16_t data, port_mask;
    port_mask = port_i2c_chl_mask[port.num];
    ret = read_lpmode(port.num, &data);

    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    *lpmode = (data & port_mask) ? true : false;
    return SN_SUCCESS;

exit_fail:
    return ret;
}
int sn_port_write_lpmode(port_t port, bool lpmode) {
    ssize_t ret;
    uint16_t data, port_mask;
    ret = read_lpmode(port.num, &data);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    port_mask = port_i2c_chl_mask[port.num];
    data = lpmode ? data | port_mask : data & (~port_mask);
    ret = write_lpmode(port.num, data);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int read_lpmode(int port_num, uint16_t *data) {
    ssize_t ret = 0;
    addr_t addr;

    switch ((port_num - 1) / 8) {
        case 0:
            addr.i2c_addr = LP_MODE_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr.i2c_addr = LP_MODE_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr.i2c_addr = LP_MODE_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr.i2c_addr = LP_MODE_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        default:
            ret = EINVAL;
            goto exit_fail;
            break;
    }

    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = netberg_fpga_read_i2c(LP_MODE_I2C_CHL, addr.i2c_addr, addr.reg_addr, data);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int write_lpmode(int port_num, uint16_t data) {
    ssize_t ret = 0;
    addr_t addr;
    switch ((port_num - 1) / 8) {
        case 0:
            addr.i2c_addr = LP_MODE_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr.i2c_addr = LP_MODE_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr.i2c_addr = LP_MODE_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr.i2c_addr = LP_MODE_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        default:
            ret = EINVAL;
            goto exit_fail;
            break;
    }

    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = netberg_fpga_write_i2c(LP_MODE_I2C_CHL, addr.i2c_addr, addr.reg_addr, data);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}

// Reset
int sn_port_write_reset(port_t port, bool reset) {
    ssize_t ret;
    uint16_t data;
    ret = read_reset(port.num, &data);
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    if (reset) {
        ret = write_reset(port.num, data);
        if (ret != SN_SUCCESS) {
            goto exit_fail;
        }
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int read_reset(int port_num, uint16_t *data) {
    ssize_t ret = 0;
    addr_t addr;

    switch ((port_num - 1) / 8) {
        case 0:
            addr.i2c_addr = RESET_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr.i2c_addr = RESET_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr.i2c_addr = RESET_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr.i2c_addr = RESET_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        default:
            ret = EINVAL;
            goto exit_fail;
            break;
    }
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = netberg_fpga_read_i2c(RESET_I2C_CHL, addr.i2c_addr, addr.reg_addr, data);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    return SN_SUCCESS;
exit_fail:
    return ret;
}
int write_reset(int port_num, uint16_t data) {
    ssize_t ret = 0;
    uint16_t port_mask;
    addr_t addr;
    // LPMode has 2 I2C channels and each of them handl 16 ports. Most significant 16 bits
    // represent 17~32 ports and least significant 16 bits represent 1~16 ports.
    switch ((port_num - 1) / 8) {
        case 0:
            addr.i2c_addr = RESET_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr.i2c_addr = RESET_I2C_ADDR_1;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr.i2c_addr = RESET_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr.i2c_addr = RESET_I2C_ADDR_2;
            addr.reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        default:
            ret = EINVAL;
            goto exit_fail;
            break;
    }
    port_mask = port_i2c_chl_mask[port_num];
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    // set reset
    data |= port_mask; 
    ret = netberg_fpga_write_i2c(RESET_I2C_CHL, addr.i2c_addr, addr.reg_addr, data);
    if (ret != SN_SUCCESS) {
        goto exit_unlock;
    }
    // recover reset
    data &= (~port_mask);
    ret = netberg_fpga_write_i2c(RESET_I2C_CHL, addr.i2c_addr, addr.reg_addr, data);
    if (ret != SN_SUCCESS) {
        goto exit_unlock;
    }
    sn_unlock();

    return SN_SUCCESS;
exit_unlock:
    sn_unlock();
exit_fail:
    return ret;
}

// Temperature
int sn_port_temp_read(port_t port, int8_t *temp) {
    ssize_t ret;
    uint8_t u_temp;
    ret = sn_lock();
    if (ret != SN_SUCCESS) goto exit_fail;
    ret = netberg_fpga_QSFP_Read(port.num, QSFP_TEMP_MSB_EEPROM_ADDR, &u_temp);
    sn_unlock();
    if (ret != SN_SUCCESS) {
        goto exit_fail;
    }
    *temp = u_temp;
    return SN_SUCCESS;
exit_fail:
    return ret;
}
