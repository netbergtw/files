#include <fcntl.h>
#include <inc/nb_ports_ctrl.h>
#include <nb_fpga_utility/inc/util.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

extern uint32_t port_i2c_chl_mask[];
static ports_cache_info_t ports_cache_info;

int nb_ports_cache_init() {
    fpga_ut_debug("cache_init");

    memset(&ports_cache_info, 0, sizeof(ports_cache_info));

    if (nb_ports_ctrl_lpmode_get(&ports_cache_info.cache_lpmode.u32lpmode) != NB_SUCCESS) {
        fpga_ut_err("Get all ports' lp mode fail");
        return -NB_FAIL;
    }
    fpga_ut_info("Update lpmode cache:%x\n", ports_cache_info.cache_lpmode.u32lpmode);
    if (nb_ports_ctrl_reset_get(&ports_cache_info.cache_reset.u32reset) != NB_SUCCESS) {
        fpga_ut_err("Get all ports' reset fail");
        return -NB_FAIL;
    }
    fpga_ut_info("Update reset cache:%x", ports_cache_info.cache_reset.u32reset);
    for (int port = 1; port < PORT_NUM; port += 4) {
        if (nb_ports_ctrl_port_led_get(port, &ports_cache_info.cache_leddata[(port / 4)].u32leddata) != NB_SUCCESS) {
            fpga_ut_err("[NB_FPGA] LED get error: set %d", port);
            return -NB_FAIL;
        }
    }

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_init                                         */
/* Description  : Initialize port ctrl IO interface                          */
/* Input        : None                                                       */
/* Output       : None                                                       */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_init() {
    int ret;
    uint32_t data = 0;
    int fd;
    system("echo 1 > /sys/bus/pci/devices/0000:66:00.0/enable");
    system("chmod 777 /sys/bus/pci/devices/0000:66:00.0/resource0");
    setenv("nb_fpga_dev", "/sys/bus/pci/devices/0000:66:00.0/resource0", 1);
    nb_lock_init();

    fd = shm_open("qsfp_init", O_RDWR, 0666);
    if (fd < 0) {
        fpga_ut_debug("qsfp init start");
        ret = nb_ports_ctrl_run_cmd("nb_fpga QSFP_init", &data);
        if (ret < 0) {
            fpga_ut_err("[NB_FPGA] QSFP INIT error");
            return ret;
        }
        fd = shm_open("qsfp_init", O_CREAT, 0666);
        if (fd < 0) {
            fpga_ut_err("qsfp inti tag create fail");
            return -NB_FAIL;
        }
        fpga_ut_debug("qsfp init tag create done");
    }
    fpga_ut_debug("qsfp init bypass");
    return NB_SUCCESS;
}

int nb_ports_ctrl_deinit() {
    return nb_lock_del();
}
/*****************************************************************************/
/* Function Name: nb_ports_ctrl_presnece_get                                 */
/* Description  : Get all ports presence status                              */
/* Input        : None                                                       */
/* Output       : u32presence                                                */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_presnece_get(uint32_t *u32presence) {
    int ret;
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd("nb_fpga RQPST", u32presence);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] PRESENT all port get error");
        nb_unlock();
        return ret;
    }

    nb_unlock();
    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_presnece_get                            */
/* Description  : Get specific ports presence status                         */
/* Input        : port                                                       */
/* Output       : bpresence                                                  */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_presnece_get(uint8_t port, bool *bpresence) {
    int ret;
    uint32_t data = 0;
    addr_t addr;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);

    // get present i2c address
    ret = nb_ports_ctrl_get_read_present_addr(port, &addr);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] PRESENT read addr get error");
        return ret;
    }

    // process cmd
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", PRESENT_I2C_CHL, addr.i2c_addr, addr.reg_addr);
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] PRESENT port get error");
        nb_unlock();
        return ret;
    }
    
    nb_unlock();

    *bpresence = (data & port_i2c_chl_mask[port]) ? true : false;

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_lpmode_get                                   */
/* Description  : Get all ports lpmode status                                */
/* Input        : None                                                       */
/* Output       : u32lpmode                                                  */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_lpmode_get(uint32_t *u32lpmode) {
    int ret;
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd("nb_fpga RQLPM", u32lpmode);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LPMODE all port get error");
        nb_unlock();
        return ret;
    }

    nb_unlock();
    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_lpmode_get                              */
/* Description  : get specific port lpmode                                   */
/* Input        : u8port, bassert                                            */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_lpmode_get(uint8_t port, bool *blpmode) {
    int ret;
    uint32_t data = 0;
    addr_t addr;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);

    // get present i2c address
    ret = nb_ports_ctrl_get_read_lpmode_addr(port, &addr);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LPMODE read addr get error");
        return ret;
    }

    // process cmd
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", LP_MODE_I2C_CHL, addr.i2c_addr, addr.reg_addr);
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LPMODE port get error");
        nb_unlock();
        return ret;
    }
    nb_unlock();
    
    *blpmode = (data & port_i2c_chl_mask[port]) ? true : false;

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_lpmode_set                              */
/* Description  : set specific port lpmode                                   */
/* Input        : u8port, bassert                                            */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_lpmode_set(uint8_t port, bool bassert) {
    int ret;
    uint32_t data = 0;
    uint8_t *cached;
    addr_t addr;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);

    // get lpmode writing i2c address
    ret = nb_ports_ctrl_get_write_lpmode_addr(port, &addr);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LPMODE write addr get error");
        return ret;
    }
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    cached = &(ports_cache_info.cache_lpmode.lpmode_chl[(port - 1) / 8]);
    *cached = bassert ? *cached | port_i2c_chl_mask[port] : *cached & ~(port_i2c_chl_mask[port]);

    // process cmd
    sprintf(cmd, "nb_fpga i2c_w 0x%x 0x%x 0x%x 0x%x", LP_MODE_I2C_CHL, addr.i2c_addr, addr.reg_addr, *cached);

    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LPMODE write error");
        nb_unlock();
        return ret;
    }
    nb_unlock();

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_reset_set                                    */
/* Description  : get all port  reset                                        */
/* Input        : u8port, *u32reset                                          */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_reset_get(uint32_t *u32reset) {
    int ret;
    uint32_t data = 0;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);

    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    // get all port reset data from 4 channels.
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", RESET_I2C_CHL, RESET_I2C_ADDR_1, IO_EXPANDER_OUTPUT_PORT0_ADDR_0);
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET get:1 error");
        goto exec_fail;
    }
    *u32reset = data;
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", RESET_I2C_CHL, RESET_I2C_ADDR_1, IO_EXPANDER_OUTPUT_PORT1_ADDR_0);
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET get:2 error");
        goto exec_fail;
    }
    *u32reset |= data << 8;
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", RESET_I2C_CHL, RESET_I2C_ADDR_2, IO_EXPANDER_OUTPUT_PORT0_ADDR_0);
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET get:3 error");
        goto exec_fail;
    }
    *u32reset |= data << 16;
    sprintf(cmd, "nb_fpga i2c_r 0x%x 0x%x 0x%x", RESET_I2C_CHL, RESET_I2C_ADDR_2, IO_EXPANDER_OUTPUT_PORT1_ADDR_0);
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET get:4 error");
        goto exec_fail;
    }
    *u32reset |= data << 24;
    nb_unlock();
    return NB_SUCCESS;
    
exec_fail:
    nb_unlock();
    return -NB_FAIL;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_reset_set                               */
/* Description  : assert or deassert specific port reset                     */
/* Input        : u8port, bassert                                            */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_reset_set(uint8_t port, bool bassert) {
    int ret;
    uint32_t data = 0;
    uint8_t *cached;
    addr_t addr;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);

    ret = nb_ports_ctrl_get_write_reset_addr(port, &addr);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET write addr get error");
        return ret;
    }

    // proccess writing data
    // set 0  to assert reset , 1 to deassert
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    cached = &(ports_cache_info.cache_reset.reset_chl[(port - 1) / 8]);
    *cached = bassert ? *cached & ~(port_i2c_chl_mask[port]) : *cached | port_i2c_chl_mask[port];

    // process cmd
    sprintf(cmd, "nb_fpga i2c_w 0x%x 0x%x 0x%x 0x%x", RESET_I2C_CHL, addr.i2c_addr, addr.reg_addr, *cached);

    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] RESET set error");
        nb_unlock();
        return ret;
    }
    nb_unlock();

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_led_get                                 */
/* Description  : set specific port led mode                                 */
/* Input        : u8port, u8mode                                             */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_led_get(uint8_t port, uint32_t *leddata) {
    int ret;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);
    uint32_t port_group_addr;

    port_group_addr = LED_RGB_I2C_ADDR + ((port - 1) / 4) * 4;
    // process cmd
    sprintf(cmd, "nb_fpga rmem32 %d", port_group_addr);
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd(cmd, leddata);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LED get error");
        nb_unlock();
        return ret;
    }
    nb_unlock();

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_led_set                                 */
/* Description  : set specific port led mode                                 */
/* Input        : u8port, u8mode                                             */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_led_set(uint8_t port, uint8_t u8mode) {
    int8_t ret, idx;
    uint32_t data = 0;
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);
    cache_leddata_t *cache_leddata;
    uint8_t integrated_led;
    
    // data process
    integrated_led = u8mode << 4 | u8mode;
    idx = ((port + 3) % 4);
    // get cache 
    cache_leddata = &(ports_cache_info.cache_leddata[(port - 1) / 4]);
    
    if(cache_leddata->leddata[idx] == integrated_led){
        return NB_SUCCESS;
    }else{
        cache_leddata->leddata[idx] = integrated_led;
    }
    
    // process cmd
    sprintf(cmd, "nb_fpga led_w %d %u", ((port - 1) / 4) + 1, cache_leddata->u32leddata);
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    ret = nb_ports_ctrl_run_cmd(cmd, &data);
    if (ret < 0) {
        fpga_ut_err("[NB_FPGA] LED set error");
        nb_unlock();
        return ret;
    }
    nb_unlock();

    return NB_SUCCESS;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_eeprom_read                             */
/* Description  : read port's eeprom information                             */
/* Input        : u8port, u8offset                                           */
/* Output       : u8length, u8data                                           */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_eeprom_get(uint8_t port, uint8_t u8offset, uint8_t u8length, uint8_t *u8data) {
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);
    int ret;
    uint32_t data = 0;
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    for (uint8_t idx = 0; idx < u8length; idx++) {
        sprintf(cmd, "nb_fpga QSFP_r 0x%x 0x%x", port, u8offset + idx);
        ret = nb_ports_ctrl_run_cmd(cmd, &data);
        if (ret < 0) {
            fpga_ut_err("[NB_FPGA] QSFP read error");
            goto exec_fail;
        }
        u8data[idx] = data & 0xff;
    }
    nb_unlock();
    return NB_SUCCESS;
exec_fail:
    nb_unlock();
    return -NB_FAIL;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_port_eeprom_set                              */
/* Description  : write port's eeprom information                            */
/* Input        : u8port, u8offset, u8length, u8data                         */
/* Output       :                                                            */
/* Return       : NB_SUCCESS / -NB_FAIL                                       */
/*****************************************************************************/
int nb_ports_ctrl_port_eeprom_set(uint8_t port, uint8_t u8offset, uint8_t u8length, uint8_t *u8data) {
    char cmd[CMD_LEN];
    memset(cmd, 0, sizeof(char) * CMD_LEN);
    int ret;
    uint32_t data = 0;
    ret = nb_lock();
    if (ret != 0) {
        fpga_ut_err("fpga_util lock fail : %d", ret);
        return -NB_FAIL;
    }
    for (int idx = 0; idx < u8length; idx++) {
        sprintf(cmd, "nb_fpga QSFP_w 0x%x 0x%x 0x%x", port, u8offset + idx, u8data[idx]);
        ret = nb_ports_ctrl_run_cmd(cmd, &data);
        if (ret < 0) {
            fpga_ut_err("[NB_FPGA] QSFP write error");
            goto exec_fail;
        }
    }
    nb_unlock();
    return NB_SUCCESS;
exec_fail:
    nb_unlock();
    return -NB_FAIL;
}
