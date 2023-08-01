/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform drivers for NBA810 32x400G switch.
 */

#include "plat_port.h"
#include "plat_conf.h"


static int flag_ports_scanner = 0;
static port_fsm_t *port_fsm_p = NULL;
static void plat_port_scan_worker(struct work_struct *work);
static DECLARE_DELAYED_WORK(ports_scanner, plat_port_scan_worker);

static uint64_t port_valid_input_data_width = 0xffffffff;  // NBA810 total 32 ports


int plat_port_type_get(int port_num)
{
    if ((port_num >= PLAT_PORT_SFP_MIN) && (port_num <= PLAT_PORT_SFP_MAX)) {
        return PORT_TYPE_SFP;
    }
    if ((port_num >= PLAT_PORT_QSFP_MIN) && (port_num <= PLAT_PORT_QSFP_MAX)) {
        return PORT_TYPE_QSFP;
    }
    return -EINVAL;
}


int plat_port_first_num(void)
{
    return PLAT_PORT_NUM_START;
}


static int _is_port_present(int port_num)
{
    if (port_fsm_p) {
        return ((port_fsm_p->port_present >> (port_num - PLAT_PORT_NUM_START)) & 0x1);
    }
    return 0;
}

static int _port_ioexp_get_all(ioexp_t *ioexp_p, uint64_t *data)
{
    int i = 0;
    int err = 0;
    uint16_t ret = 0;
    uint64_t tmp = 0;
    plat_mutex_t *mtx_p = ioexp_p->mutex_p;
    
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    *data = 0;
    for (i=0; i<PLAT_IOEXP_DATA_WIDTH; i++) {
        err = nb_fpga_i2c_read(ioexp_p->in_bus[i], 
                               ioexp_p->in_addr[i], 
                               ioexp_p->in_reg[i], 
                               &ret);
        if (err != 0) {
            PLAT_DBG("nb_fpga_i2c_read fail! <bus>:0x%x <addr>:0x%x <reg>:0x%x\n",
                      ioexp_p->in_bus[i], ioexp_p->in_addr[i], ioexp_p->in_reg[i]);
            mtx_p->unlock_dev(mtx_p);
            return -EIO;
        }
        tmp = ((uint64_t)ret << (8*i));
        *data = (*data | tmp);
    }
    mtx_p->unlock_dev(mtx_p);
    return 0;
}


static uint64_t _nba810_ioexp_bitswap_get(uint64_t raw_data)
{
    uint64_t bitswap1 = (raw_data << 1) & (0xa0aaaa00);
    uint64_t bitswap2 = (raw_data >> 1) & (0x50555500);
    uint64_t retval = (raw_data & 0x0f0000ff) + bitswap1 + bitswap2;
    retval &= port_valid_input_data_width;
    return retval;
}


int fsm_plat_port_present_get_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int i = 0;
    int err = 0;
    int retry = 5;
    uint64_t raw_data;

    for (i=0; i<retry; i++) {
        err = _port_ioexp_get_all(plat_p->ioexp_present_p, &raw_data);
        if (err == 0) {
            break;
        }
    }
    if (err < 0) {
        PLAT_ERR("_port_ioexp_get_all fail! <err>:%d\n", err);
    } else {
        *data = _nba810_ioexp_bitswap_get(raw_data);
    }
    return err;
}


int plat_port_present_get_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    *data = port_fsm_p->port_present;
    return 0;
}


int plat_port_lpmod_get_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int err;
    uint64_t raw_data;

    err = _port_ioexp_get_all(plat_p->ioexp_lpmod_p, &raw_data);
    if (err < 0) {
        PLAT_ERR("_port_ioexp_get_all fail! <err>:%d\n", err);
        return err;
    }
    *data = _nba810_ioexp_bitswap_get(raw_data);
    return 0;
}


int plat_port_reset_get_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int err;
    uint64_t raw_data;

    err = _port_ioexp_get_all(plat_p->ioexp_reset_p, &raw_data);
    if (err < 0) {
        PLAT_ERR("_port_ioexp_get_all fail! <err>:%d\n", err);
        return err;
    }
    *data = _nba810_ioexp_bitswap_get(raw_data);
    return 0;
}


int plat_port_rxlos_get_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int err;
    uint64_t raw_data;

    err = _port_ioexp_get_all(plat_p->ioexp_rxlos_p, &raw_data);
    if (err < 0) {
        PLAT_ERR("_port_ioexp_get_all fail! <err>:%d\n", err);
        return err;
    }
    *data = _nba810_ioexp_bitswap_get(raw_data);
    return 0;
}


port_t* get_port_obj(nb_platdrv_t *plat_p, int port_num)
{
    int i = port_num - PLAT_PORT_NUM_START;
    if (plat_port_type_get(port_num) <0) {
        return NULL;
    }
    return plat_p->port_list[i];
}


int plat_port_i2c_read(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg) 
{
     PLAT_ERR("Function not ready\n");
     return -1;
}


int plat_port_i2c_write(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t addr, uint16_t reg) 
{
     PLAT_ERR("Function not ready\n");
     return -1;
}


int plat_port_xcvr_read(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg)
{
    int err;
    uint8_t tmp;
    plat_mutex_t *mtx_p; 
    port_t *port_p = get_port_obj(plat_p, port_num);

    if (!port_p) {
        PLAT_ERR("Can't get port_obj! <num>:%d\n", port_num);
        return -ENXIO;
    }
    if (!_is_port_present(port_num)) {
        return -ENODEV;
    }
    mtx_p = port_p->mutex_p;
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    // TBD
    //  -> Change to object base IO
    //
    err = nb_fpga_qsfp_read(port_num, reg, &tmp);
    mtx_p->unlock_dev(mtx_p);
    if (err != 0) {
        PLAT_DBG("nb_fpga_qsfp_read() fail!\n");
        return -EIO;
    }
    *buf = (uint16_t)tmp;
     return 0;
}


int plat_port_xcvr_write(nb_platdrv_t *plat_p, uint16_t *buf, int port_num, uint16_t reg)
{
    int err;
    plat_mutex_t *mtx_p; 
    port_t *port_p = get_port_obj(plat_p, port_num);

    if (!port_p) {
        PLAT_ERR("Can't get port_obj! <num>:%d\n", port_num);
        return -ENXIO;
    }
    if (!_is_port_present(port_num)) {
        return -ENODEV;
    }
    mtx_p = port_p->mutex_p;
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    // TBD
    //  -> Change to object base IO
    //
    err = nb_fpga_qsfp_write(port_num, reg, *buf);
    mtx_p->unlock_dev(mtx_p);
    if (err != 0) {
        PLAT_DBG("nb_fpga_qsfp_read() fail!\n");
        return -EIO;
    }
     return 0;
}


static int _port_ioexp_set_all(ioexp_t *ioexp_p, uint64_t *data)
{
    int i = 0;
    int err = 0;
    uint16_t val = 0;
    uint64_t tmp = *data;
    plat_mutex_t *mtx_p = ioexp_p->mutex_p;
    
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    for (i=0; i<PLAT_IOEXP_DATA_WIDTH; i++) {
        val = (uint16_t)((tmp >> (8*i)) & 0xff);
        err = nb_fpga_i2c_write(ioexp_p->out_bus[i], 
                                ioexp_p->out_addr[i], 
                                ioexp_p->out_reg[i], 
                                val);
        if (err != 0) {
            PLAT_DBG("nb_fpga_i2c_write fail! <bus>:0x%x <addr>:0x%x <reg>:0x%x <val>:0x%x\n",
                      ioexp_p->out_bus[i], ioexp_p->out_addr[i], ioexp_p->out_reg[i], val);
            mtx_p->unlock_dev(mtx_p);
            return -EIO;
        }
    }
    mtx_p->unlock_dev(mtx_p);
    return 0;
}


static uint64_t _nba810_ioexp_bitswap_set(uint64_t sorted_data)
{
    uint64_t bitswap1 = (sorted_data << 1) & (0xa0aaaa00);
    uint64_t bitswap2 = (sorted_data >> 1) & (0x50555500);
    uint64_t retval = (sorted_data & 0x0f0000ff) + bitswap1 + bitswap2;
    retval &= port_valid_input_data_width;
    return retval;
}


int plat_port_lpmod_set_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int err = 0;
    uint64_t raw_data = 0;

    *data &= port_valid_input_data_width;
    raw_data = _nba810_ioexp_bitswap_set(*data);
    err = _port_ioexp_set_all(plat_p->ioexp_lpmod_p, &raw_data);
    if (err < 0) {
        PLAT_ERR("_port_ioexp_set_all fail! <err>:%d\n", err);
        return err;
    }
    return 0;
}


int plat_port_reset_set_all(nb_platdrv_t *plat_p, uint64_t *data)
{
    int err = 0;
    uint64_t raw_data = 0;

    *data &= port_valid_input_data_width;
    raw_data = _nba810_ioexp_bitswap_set(*data);
    err = _port_ioexp_set_all(plat_p->ioexp_reset_p, &raw_data);
    if (err < 0) {
        PLAT_ERR("_port_ioexp_set_all fail! <err>:%d\n", err);
        return err;
    }
    return 0;
}


int plat_port_led_rgb_get(nb_platdrv_t *plat_p, uint32_t *buf, int port_num)
{
    int err;
    uint32_t tmp;
    plat_mutex_t *mtx_p; 
    port_t *port_p = get_port_obj(plat_p, port_num);

    if (!port_p) {
        PLAT_ERR("Can't get port_obj! <num>:%d\n", port_num);
        return -ENXIO;
    }
    mtx_p = port_p->mutex_p;
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    err = nb_fpga_mem32_read(port_p->led_rgb_addr, &tmp);
    mtx_p->unlock_dev(mtx_p);
    if (err != 0) {
        PLAT_DBG("nb_fpga_qsfp_read() fail!\n");
        return -EIO;
    }
    port_p->cache_led = (tmp >> port_p->led_rgb_offs) & 0xff;
    (*buf) = port_p->cache_led;
    
    return 0;
}


int plat_port_led_rgb_set(nb_platdrv_t *plat_p, uint32_t *buf, int port_num)
{
    int err;
    uint32_t tmp_set;
    uint32_t tmp_curr;
    uint32_t valid_mask = 0x77; // 8-Bit: x R B G x R B G
    plat_mutex_t *mtx_p; 
    port_t *port_p = get_port_obj(plat_p, port_num);

    if (!port_p) {
        PLAT_ERR("Can't get port_obj! <port>:%d\n", port_num);
        return -ENXIO;
    }
    if (*buf & ~valid_mask) {
        PLAT_ERR("Input Invalid. <port>:%d <val>:0x%x\n", port_num, *buf);
        return -EINVAL;
    }
    if (*buf == port_p->cache_led) {
        PLAT_DBG("Set the same. <port>:%d <val>:0x%x\n", port_num, *buf);
        return 0;
    }
    mtx_p = port_p->mutex_p;
    if (!mtx_p->lock_dev(mtx_p)) {
        PLAT_DBG("lock_dev() fail\n");
        return -EBUSY;
    }
    err = nb_fpga_mem32_read(port_p->led_rgb_addr, &tmp_curr);
    if (err < 0) {
        mtx_p->unlock_dev(mtx_p);
        return err;
    }
    tmp_curr &= ~(0xff << port_p->led_rgb_offs);
    tmp_set = tmp_curr | (*buf << port_p->led_rgb_offs);
    err = nb_fpga_mem32_write(port_p->led_rgb_addr, tmp_set);
    nb_fpga_mem32_write(port_p->led_ctl_addr, NB_FPGA_LED_MEM_CTL_ENABLE);
    mtx_p->unlock_dev(mtx_p);
    if (err != 0) {
        PLAT_DBG("nb_fpga_qsfp_read() fail!\n");
        return -EIO;
    }
    port_p->cache_led = (*buf);
    return 0;
}


static int plat_port_led_rgb_set_all(nb_platdrv_t *plat_p, uint32_t *buf)
{
    int err = 0;
    int index = 0;
    int port_num = PLAT_PORT_NUM_START;

    for (index=0; index<PLAT_PORT_NUM_TOTAL; index++) {
        err = plat_port_led_rgb_set(plat_p, buf, port_num);
        if (err < 0) {
            PLAT_DBG("Fail! <port>:%d <err>:%d\n", port_num, err);
            return err;
        }
        port_num++;
    }
    return 0;
}


static void nb_fpga_ioexp_free(nb_platdrv_t *plat_p)
{
    // For Fast-boot considition, IOEXP doesn't needs to free.
    return; 
}

static int nb_fpga_ioexp_init(void)
{
    int i = 0;
    int err = 0;

    for (i=0; i<PLAT_IOEXP_CONF_NUM; i++) {
        err = nb_fpga_i2c_write(ioexp_config_bus[i],
                                ioexp_config_addr[i],
                                ioexp_config_reg[i],
                                ioexp_config_data[i]);
        if (err < 0) {
            PLAT_ERR("fail! <bus>:0x%x <addr>:0x%x <reg>:0x%x <data>:0x%x\n",
                      ioexp_config_bus[i], ioexp_config_addr[i], 
                      ioexp_config_reg[i], ioexp_config_data[i]);
            return err;
        }
    }
    return 0;
}


static void nb_fpga_free(nb_platdrv_t *plat_p)
{
    nb_fpga_lock_destroy(plat_p);
    if (plat_p->pcidev) {
        pci_dev_put(plat_p->pcidev);
    }
}


static int nb_fpga_init(nb_platdrv_t *plat_p)
{
    int err = 0;
    uint32_t pci_addr;
     
    plat_p->pcidev = pci_get_device(NB_FPGA_PCI_VENDOR_ID_XILINX, 
                                   NB_FPGA_PCI_DEVICE_ID_XILINX, 
                                   NULL);
    if (!plat_p->pcidev) {
        PLAT_ERR("pci_get_device fail\n");
        err = -ENODEV;
        goto err_fpga_init_1;
    }
    if (pci_enable_device(plat_p->pcidev)) {
        PLAT_ERR("pci_enable_device fail\n");
        err = -ENODEV;
        goto err_fpga_init_2;
    }
    if (pci_read_config_dword(plat_p->pcidev, PCI_BASE_ADDRESS_0, &pci_addr)) {
        PLAT_ERR("pci_read_config_dword fail\n");
        err = -ENODEV;
        goto err_fpga_init_2;
    }
    PLAT_INFO("FPGA PCI MEM addr: %#x \n", pci_addr);

    nb_fpga_mmap_addr = ioremap(pci_addr, NB_FPGA_MEM_SIZE);

    if (nb_fpga_mmap_addr == NULL) {
        PLAT_ERR("FPGA PCI BAR0 mmap failed \n");
        err = -ENOMEM;
        goto err_fpga_init_2;
    }
    PLAT_INFO("FPGA PCI BAR0 mmapped %#llX size %#x \n", 
               (u64)nb_fpga_mmap_addr,
               NB_FPGA_MEM_SIZE);
    nb_fpga_mem32_write(0x00000114, 0x00000200);
    mdelay(500);
    nb_fpga_mem32_write(0x00000118, 0x00000064);
    mdelay(500);

    nb_fpga_lock_init(plat_p);
    plat_p->fpga_mutex->lock_dev = nb_fpga_lock;
    plat_p->fpga_mutex->unlock_dev = nb_fpga_unlock;

    PLAT_DBG("... OK\n");
    return 0;

err_fpga_init_2:
    pci_dev_put(plat_p->pcidev);
err_fpga_init_1:
    return err;
}


static void plat_ioexp_obj_free(nb_platdrv_t *plat_p)
{
    if (plat_p->ioexp_present_p) {
        kfree(plat_p->ioexp_present_p);
    }
    if (plat_p->ioexp_lpmod_p) {
        kfree(plat_p->ioexp_lpmod_p);
    }
    if (plat_p->ioexp_reset_p) {
        kfree(plat_p->ioexp_reset_p);
    }
    if (plat_p->ioexp_rxlos_p) {
        kfree(plat_p->ioexp_rxlos_p);
    }
}


static int _plat_ioexp_obj_create(ioexp_t **ioexp_p, plat_mutex_t *mutex_p,
                                  uint16_t *in_bus, uint16_t *in_addr, uint16_t *in_reg,
                                  uint16_t *out_bus, uint16_t *out_addr, uint16_t *out_reg)
{
    (*ioexp_p) = kzalloc(sizeof(ioexp_t), GFP_KERNEL);
    if (!ioexp_p) {
        PLAT_ERR("kzalloc fail!\n");
        return -ENOMEM;
    }
    if (in_bus)   memcpy((*ioexp_p)->in_bus   ,in_bus,   (sizeof(*in_bus)*PLAT_IOEXP_DATA_WIDTH) );
    if (in_addr)  memcpy((*ioexp_p)->in_addr  ,in_addr,  (sizeof(*in_addr)*PLAT_IOEXP_DATA_WIDTH) );
    if (in_reg)   memcpy((*ioexp_p)->in_reg   ,in_reg,   (sizeof(*in_reg)*PLAT_IOEXP_DATA_WIDTH) );
    if (out_bus)  memcpy((*ioexp_p)->out_bus  ,out_bus,  (sizeof(*out_bus)*PLAT_IOEXP_DATA_WIDTH) );
    if (out_addr) memcpy((*ioexp_p)->out_addr ,out_addr, (sizeof(*out_addr)*PLAT_IOEXP_DATA_WIDTH) );
    if (out_reg)  memcpy((*ioexp_p)->out_reg  ,out_reg,  (sizeof(*out_reg)*PLAT_IOEXP_DATA_WIDTH) );
    (*ioexp_p)->mutex_p = mutex_p;
    (*ioexp_p)->data_width = PLAT_IOEXP_DATA_WIDTH;
    return 0;
}


static int plat_ioexp_obj_init(nb_platdrv_t *plat_p)
{
    if (_plat_ioexp_obj_create(&(plat_p->ioexp_present_p), plat_p->fpga_mutex, 
                               ioexp_in_present_bus, ioexp_in_present_addr, ioexp_in_present_reg,
                               NULL, NULL, NULL) < 0) {
        PLAT_ERR("Create Present obj fail\n");
        goto err_plat_ioexp_obj_init;
    }
    if (_plat_ioexp_obj_create(&(plat_p->ioexp_lpmod_p), plat_p->fpga_mutex, 
                               ioexp_in_lpmod_bus, ioexp_in_lpmod_addr, ioexp_in_lpmod_reg,
                               ioexp_out_lpmod_bus, ioexp_out_lpmod_addr, ioexp_out_lpmod_reg) < 0) {
        PLAT_ERR("Create LPMod obj fail\n");
        goto err_plat_ioexp_obj_init;
    }
    if (_plat_ioexp_obj_create(&(plat_p->ioexp_reset_p), plat_p->fpga_mutex, 
                               ioexp_in_reset_bus, ioexp_in_reset_addr, ioexp_in_reset_reg,
                               ioexp_out_reset_bus, ioexp_out_reset_addr, ioexp_out_reset_reg) < 0) {
        PLAT_ERR("Create REST obj fail\n");
        goto err_plat_ioexp_obj_init;
    }
    if (_plat_ioexp_obj_create(&(plat_p->ioexp_rxlos_p), plat_p->fpga_mutex, 
                               ioexp_in_rxlos_bus, ioexp_in_rxlos_addr, ioexp_in_rxlos_reg,
                               NULL, NULL, NULL) < 0) {
        PLAT_ERR("Create RX Los obj fail\n");
        goto err_plat_ioexp_obj_init;
    }

    PLAT_DBG("... OK\n");
    return 0;

err_plat_ioexp_obj_init:
    plat_ioexp_obj_free(plat_p);
    return -ENOMEM;
}


static void plat_port_dev_free(nb_platdrv_t *plat_p)
{
    nb_fpga_ioexp_free(plat_p);
    nb_fpga_free(plat_p);
}


static int plat_port_dev_init(nb_platdrv_t *plat_p) 
{
    int err;

    PLAT_DBG("... GO\n");
    err = nb_fpga_init(plat_p);
    if (err < 0) {
        PLAT_ERR("nb_fpga_init fail <err>:%d\n", err);
        goto err_plat_port_dev_init_1;
    }
    err = nb_fpga_ioexp_init();
    if (err < 0) {
        PLAT_ERR("nb_fpga_ioexp_init fail\n.");
        goto err_plat_port_dev_init_2;
    }
    PLAT_DBG("... OK\n");
    return 0;

err_plat_port_dev_init_2:
    nb_fpga_free(plat_p);
err_plat_port_dev_init_1:    
    return err;
}


bool port_obj_lock(plat_mutex_t *mtx_p)
{
    mutex_lock(&(mtx_p->dev_mutex));
    return true;
}

// bool port_obj_lock(plat_mutex_t *mtx_p)
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


void port_obj_unlock(plat_mutex_t *mtx_p)
{
    mutex_unlock(&(mtx_p->dev_mutex));
}


static void plat_port_obj_free(nb_platdrv_t *plat_p)
{
    int i = 0;
    port_t *port_p = NULL;

    for (i=0; i<PLAT_PORT_NUM_TOTAL; i++) {
        if (plat_p->port_list[i]) {
            port_p = plat_p->port_list[i];
            if (port_p->cache) kfree(port_p->cache);
            kfree(port_p);
        }
    }
}


static int _plat_port_obj_create(port_t **port_p, int port_num, int port_type, int ledcfg_index, plat_mutex_t *mutex_p)
{
    int i;

    (*port_p) = kzalloc(sizeof(port_t), GFP_KERNEL);
    if (!(*port_p)) {
        PLAT_ERR("kzalloc port_p fail! <num>:%d <type>:%d\n", port_num, port_type);
        return -ENOMEM;
    }
    (*port_p)->num = port_num;
    (*port_p)->type = port_type;
    (*port_p)->cache = NULL;
    (*port_p)->mutex_p = mutex_p;
    (*port_p)->led_ctl_addr = PLAT_FPGA_LED_CTL_BASE_ADDR;
    (*port_p)->led_rgb_addr = PLAT_FPGA_LED_CTL_BASE_ADDR + led_rgb_offset_addr[ledcfg_index];

    // PLAT_ERR("Port-%d Base=0x%x RGB_conf=0x%x ")

    (*port_p)->led_rgb_offs = led_rgb_offset_bit[ledcfg_index];
    for (i=0; i<PLAT_PORT_LED_PERPORT_NUM; i++) {
        (*port_p)->led_grn_bit[i] = led_rgb_offset_bit_green[i];
        (*port_p)->led_blu_bit[i] = led_rgb_offset_bit_blue[i];
        (*port_p)->led_red_bit[i] = led_rgb_offset_bit_red[i];
    }
    return 0;
}


static int plat_port_obj_init(nb_platdrv_t *plat_p)
{
    int err = 0;
    int index = 0;
    int port_num = PLAT_PORT_NUM_START;
    uint32_t led_white = 0x77;
    uint32_t led_off = 0x0;
    uint32_t buf = 0;

    // 1. Create objects
    for (index=0; index<PLAT_PORT_NUM_TOTAL; index++) {
        if ((port_num >= PLAT_PORT_SFP_MIN) && (port_num <= PLAT_PORT_SFP_MAX)) {
            err = _plat_port_obj_create(&(plat_p->port_list[index]), port_num, PORT_TYPE_SFP, index, plat_p->fpga_mutex);
            if (err < 0) {
                goto err_plat_port_obj_init;
            }
        }
        else if ((port_num >= PLAT_PORT_QSFP_MIN) && (port_num <= PLAT_PORT_QSFP_MAX)) {
            err = _plat_port_obj_create(&(plat_p->port_list[index]), port_num, PORT_TYPE_QSFP, index, plat_p->fpga_mutex);
            if (err < 0) {
                goto err_plat_port_obj_init;
            }
        } 
        else {
            PLAT_ERR("un-defined port number:%d\n", port_num);
            err = -EPERM;
            goto err_plat_port_obj_init;
        }
        port_num++;
    }
    plat_p->port_total = PLAT_PORT_NUM_TOTAL;

    // Set default Port LED
    if (hotswap) {
        // Hotswap: Update caches
        port_num = PLAT_PORT_NUM_START;
        for (index=0; index<PLAT_PORT_NUM_TOTAL; index++) {
            plat_port_led_rgb_get(plat_p, &buf, port_num);
            port_num++;
        }
    } else {
        // Non-hotswap: Set LED= white -> off and update caches
        plat_port_led_rgb_set_all(plat_p, &led_white);
        msleep(500);
        plat_port_led_rgb_set_all(plat_p, &led_off);
    }

    PLAT_DBG("init %d ports ... OK\n", PLAT_PORT_NUM_TOTAL);
    return 0;

err_plat_port_obj_init:
    plat_port_obj_free(plat_p);
    return err;
}


static int _get_task_period_ms(void)
{
    int ret = 0;

    if (PLAT_PORT_TASK_PERIOD_MS == 0) {
        return 0;
    }
    ret = ((PLAT_PORT_TASK_PERIOD_MS * HZ) / 1000);
    if (ret == 0) {
        return 1;
    }
    return ret;
}


static void plat_port_scan_worker(struct work_struct *work)
{
    int err;
    uint64_t all_present;

    if (!flag_ports_scanner) {
        PLAT_INFO("...STOP");
        return;
    }
    err = fsm_plat_port_present_get_all(port_fsm_p->plat_p, &all_present);
    if (err < 0) {
        goto next_round;
    }
    // TBD: FSM by port
    //
    if (all_present != port_fsm_p->port_present) {
        port_fsm_p->state = FSM_PORT_STATE_CHECKING;
        port_fsm_p->state_retry += 1;
        if (port_fsm_p->state_retry >= PLAT_PORT_FSM_RETRY_COUNT) {
            // Re-check finish (FSM status changes)
            port_fsm_p->port_present = all_present;
            port_fsm_p->state_retry = 0;
            port_fsm_p->state = FSM_PORT_STATE_RUNNING;
        }
    } else {
        port_fsm_p->state = FSM_PORT_STATE_RUNNING;
        port_fsm_p->state_retry = 0;
    }
    
next_round:
    schedule_delayed_work(&ports_scanner, _get_task_period_ms());
}


static void plat_port_task_free(void)
{
    int period_ms = _get_task_period_ms();
    flag_ports_scanner = 0;
    cancel_delayed_work_sync(&ports_scanner);
    if (port_fsm_p) {
        msleep(period_ms);
        kfree(port_fsm_p);
    }
}


static int plat_port_task_init(nb_platdrv_t *plat_p)
{
    int period_ms = _get_task_period_ms();

    if (!PLAT_PORT_TASK_ENABLE) {
        PLAT_DBG("Task is disabled.\n");
        return 0;
    }
    port_fsm_p = kzalloc(sizeof(port_fsm_t), GFP_KERNEL);
    if (!port_fsm_p) {
        PLAT_ERR("kzalloc fail!\n");
        return -ENOMEM;
    }
    port_fsm_p->plat_p = plat_p;
    port_fsm_p->state = FSM_PORT_STATE_RUNNING;
    port_fsm_p->state_retry = 0;
    port_fsm_p->port_present = 0;
    flag_ports_scanner = 1;
    schedule_delayed_work(&ports_scanner, period_ms);
    PLAT_DBG("Task is inited. <Period>:%d ms\n", period_ms);
    return 0;
}


void plat_port_free(nb_platdrv_t *plat_p)
{
    plat_port_task_free();
    plat_port_obj_free(plat_p);
    plat_ioexp_obj_free(plat_p);
    plat_port_dev_free(plat_p);
}


int plat_port_init(nb_platdrv_t *plat_p)
{
    int err = 0;

    PLAT_DBG("... GO\n");
    err = plat_port_dev_init(plat_p);
    if (err < 0) {
        PLAT_ERR("plat_port_dev_init fail\n.");
        goto err_plat_port_init_1;
    }
    err = plat_ioexp_obj_init(plat_p);
    if (err < 0) {
        PLAT_ERR("plat_ioexp_obj_init fail\n.");
        goto err_plat_port_init_2;
    }
    err = plat_port_obj_init(plat_p);
    if (err < 0) {
        PLAT_ERR("plat_port_obj_init fail\n.");
        goto err_plat_port_init_3;
    }
    err = plat_port_task_init(plat_p);
    if (err < 0) {
        PLAT_ERR("plat_port_task_init fail\n.");
        goto err_plat_port_init_4;
    }
    PLAT_INFO("... OK\n");
    return 0;

err_plat_port_init_4:
    plat_port_obj_free(plat_p);
err_plat_port_init_3:
    plat_ioexp_obj_free(plat_p);
err_plat_port_init_2:
    plat_port_dev_free(plat_p);
err_plat_port_init_1:
    return err;
}
