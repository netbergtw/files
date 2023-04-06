#include <nb_fpga_utility/inc/util.h>

uint32_t port_i2c_chl_mask[PORT_NUM + 1] = {0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000002, 0x00000001, 0x00000008, 0x00000004, 0x00000020, 0x00000010, 0x00000080, 0x00000040,
                                            0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000020, 0x00000010, 0x00000080, 0x00000040};

pthread_mutex_t lock;

int nb_lock_init() {
    if (pthread_mutex_init(&lock, 0) < 0) {
        fpga_ut_err("Lock init fail");
        return -NB_FAIL;
    }
    return NB_SUCCESS;
}

int nb_lock_del() {
    if (pthread_mutex_destroy(&lock) != 0) {
        fpga_ut_err("Lock destroy fail");
        return -NB_FAIL;
    }
    return NB_SUCCESS;
}

int nb_lock() {
    return pthread_mutex_lock(&lock);
}

int nb_unlock() {
    return pthread_mutex_unlock(&lock);
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_run_cmd                                      */
/* Description  : run command with popen and parse the data                  */
/* Input        : cmd   : command string                                     */
/*                *data : unsigned 32 bit pointer                            */
/* Output       : None                                                       */
/* Return       : NB_SUCCESS / -NB_FAIL                                      */
/*****************************************************************************/
int nb_ports_ctrl_run_cmd(char *cmd, uint32_t *data) {
    FILE *fp;
    char output[24];
    memset(output, 0, sizeof(char) * 24);
    int8_t ret;
    bool result;

#ifdef TIME_MEAS
    clock_t start = clock();
#endif
    // run command
    fp = popen(cmd, "r");
    if (!fp) {
        fpga_ut_debug("Fail to run command %s\n", cmd);
        return -NB_FAIL;
    }
#ifdef TIME_MEAS
    clock_t finish = clock();
    if (EXEC_TIME) fpga_ut_debug("%s : exec_time:%f", __func__, (double)(finish - start) / CLOCKS_PER_SEC);
#endif
    // read result with fgets(read until a newline)
    while (fgets(output, sizeof(output), fp) != NULL) {
        // fpga_ut_debug("%s", output);
    }
    pclose(fp);
    fpga_ut_info("%s->%s", cmd, output);
    // parse output
    ret = nb_ports_ctrl_output_parser(output, &result, data);
    if (ret < 0) {  // parsing error
        fpga_ut_debug("parsing error\n");
        return ret;
    }
    if (result) {
        return NB_SUCCESS;
    }
    return -NB_FAIL;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_output_parser                                */
/* Description  : only prccess data format : <result>:<u32 data>             */
/* Input        : output  : stout of system call                             */
/*                *result : pointer of result which contains 1 for success   */
/*                         0 for fail                                        */
/*                *data   : unsigned 32 bit pointer                          */
/* Output       : None                                                       */
/* Return       : NB_SUCCESS / -NB_FAIL                                      */
/*****************************************************************************/
int nb_ports_ctrl_output_parser(char *output, bool *result, uint32_t *data) {
    char *token;
    int8_t ret;

    token = strtok(output, ":");

    // error handling
    if (!token) {
        fpga_ut_err("string parsing error : null token\n");
        return -NB_FAIL;
    } else {
        // process result
        ret = nb_ports_ctrl_result_parser(&output[0]);
        if (ret == 0) {
            *result = ret;
            return NB_SUCCESS;
        } else if (ret == 1) {
            *result = ret;
        } else {
            fpga_ut_err("string parsing error : format error\n");
            return -NB_FAIL;
        }

        // process :data
        token = strtok(NULL, ":");
        if (token) {
            *data = strtoul(token, NULL, 0);
            return NB_SUCCESS;
        }
        return NB_SUCCESS;
    }

    return -NB_FAIL;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_result_parser                                */
/* Description  : parse result Success/Fail                                  */
/* Input        : None                                                       */
/* Output       : None                                                       */
/* Return       : 1 for success, 0 for fail, -1 for parsing error            */
/*****************************************************************************/
int nb_ports_ctrl_result_parser(char *result) {
    // strip \n
    result[strcspn(result, "\n")] = 0;

    if (strcmp(result, "Success") == 0) {
        return 1;
    } else if (strcmp(result, "Fail") == 0) {
        return 0;
    } else {
        return -NB_FAIL;
    }

    return -NB_FAIL;
}

/*****************************************************************************/
/* Function Name: nb_ports_ctrl_get_read_present_addr                             */
/* Input        : port    : required port num                                */
/*                *addr   : addr_t struct with i2c_addr and reg_addr         */
/* Output       : None                                                       */
/* Return       : NB_SUCCESS / -NB_FAIL                                      */
/*****************************************************************************/
int nb_ports_ctrl_get_read_present_addr(uint8_t port, addr_t *addr) {
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
            return -NB_FAIL;
            break;
    }

    return NB_SUCCESS;
}

int nb_ports_ctrl_get_read_lpmode_addr(uint8_t port, addr_t *addr) {
    switch ((port - 1) / 8) {
        case 0:
            addr->i2c_addr = LP_MODE_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr->i2c_addr = LP_MODE_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr->i2c_addr = LP_MODE_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr->i2c_addr = LP_MODE_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        default:
            return -NB_FAIL;
            break;
    }

    return NB_SUCCESS;
}

int nb_ports_ctrl_get_write_lpmode_addr(uint8_t port, addr_t *addr) {
    switch ((port - 1) / 8) {
        case 0:
            addr->i2c_addr = LP_MODE_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr->i2c_addr = LP_MODE_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr->i2c_addr = LP_MODE_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr->i2c_addr = LP_MODE_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        default:
            return -NB_FAIL;
            break;
    }

    return NB_SUCCESS;
}

int nb_ports_ctrl_get_read_reset_addr(uint8_t port, addr_t *addr) {
    switch ((port - 1) / 8) {
        case 0:
            addr->i2c_addr = RESET_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr->i2c_addr = RESET_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr->i2c_addr = RESET_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr->i2c_addr = RESET_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_INPUT_PORT1_ADDR_0;
            break;
        default:
            return -NB_FAIL;
            break;
    }

    return NB_SUCCESS;
}

int nb_ports_ctrl_get_write_reset_addr(uint8_t port, addr_t *addr) {
    switch ((port - 1) / 8) {
        case 0:
            addr->i2c_addr = RESET_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 1:
            addr->i2c_addr = RESET_I2C_ADDR_1;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        case 2:
            addr->i2c_addr = RESET_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT0_ADDR_0;
            break;
        case 3:
            addr->i2c_addr = RESET_I2C_ADDR_2;
            addr->reg_addr = IO_EXPANDER_OUTPUT_PORT1_ADDR_0;
            break;
        default:
            return -NB_FAIL;
            break;
    }

    return NB_SUCCESS;
}
