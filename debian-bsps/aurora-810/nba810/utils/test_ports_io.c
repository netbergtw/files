/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ports IOCTL test and userspace sample code.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#include "../includes/ports_io.h"

#define DEVFILE "/dev/"PORTS_IOCTL_DEV_NAME


void test_xcvr_eeprom_read(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_xcvr_t xcvr = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    for (i=1; i<=port_total; i++) {

        xcvr.port_num = i;
        xcvr.bank_num = PIO_RW_XCVR_CTL_BYPASS;
        xcvr.page_num = PIO_RW_XCVR_CTL_BYPASS;
        xcvr.reg      = 0;
        xcvr.data_len = 0x4;

        err = ioctl(fd, PIO_IOCTL_XCVR_READ, &xcvr);
        if (err < 0) {
            printf("[Port-%d]: Read fail!\n", i);
            continue;
        }
        printf("[Port-%d]: Read OK\n", i);
        for (j=0; j<xcvr.data_len; j++) {
            printf(" - data[%d] = 0x%x\n", j, xcvr.data[j]);
        }
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_xcvr_eeprom_write(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_xcvr_t xcvr = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    for (i=1; i<=port_total; i++) {

        xcvr.port_num = i;
        xcvr.bank_num = PIO_RW_XCVR_CTL_BYPASS;
        xcvr.page_num = PIO_RW_XCVR_CTL_BYPASS;
        xcvr.reg      = 127;
        xcvr.data_len = 1;
        xcvr.data[0]  = 0x1;

        err = ioctl(fd, PIO_IOCTL_XCVR_WRITE, &xcvr);
        if (err < 0) {
            printf("[Port-%d]: Write fail!\n", i);
            continue;
        }
        printf("[Port-%d]: Write OK\n", i);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_port_present(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Get All
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_PRESENT_READ, &ioexp);
    if (err < 0) {
        printf("[Presen]: Get all fail!\n");
    } else {
        printf("[Presen]: All Present: %lx\n", ioexp.data);
    }

    // Get one
    for (i=1; i<=port_total; i++) {

        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_PRESENT_READ, &ioexp);
        if (err < 0) {
            printf("[Presen]: Get one fail! <num>:%d\n", i);
            continue;
        }
        printf("[Presen]: Port-%d: %lx\n", i, ioexp.data);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_port_lpmod_read(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Get All
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_LPMOD_READ, &ioexp);
    if (err < 0) {
        printf("[LPMod]: Get all fail!\n");
    } else {
        printf("[LPMod]: All LPMod: %lx\n", ioexp.data);
    }

    // Get one
    for (i=1; i<=port_total; i++) {

        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_LPMOD_READ, &ioexp);
        if (err < 0) {
            printf("[LPMod]: Get one fail! <num>:%d\n", i);
            continue;
        }
        printf("[LPMod]: Port-%d: %lx\n", i, ioexp.data);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_port_reset_read(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Get All
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_REST_READ, &ioexp);
    if (err < 0) {
        printf("[RESET]: Get all fail!\n");
    } else {
        printf("[RESET]: All RESET: %lx\n", ioexp.data);
    }

    // Get one
    for (i=1; i<=port_total; i++) {

        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_REST_READ, &ioexp);
        if (err < 0) {
            printf("[RESET]: Get one fail! <num>:%d\n", i);
            continue;
        }
        printf("[RESET]: Port-%d: %lx\n", i, ioexp.data);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_port_rxlos_read(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Get All
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_RXLOS_READ, &ioexp);
    if (err < 0) {
        printf("[RXLos]: Get all fail!\n");
    } else {
        printf("[RXLos]: All RESET: %lx\n", ioexp.data);
    }

    // Get one
    for (i=1; i<=port_total; i++) {

        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_RXLOS_READ, &ioexp);
        if (err < 0) {
            printf("[RXLos]: Get one fail! <num>:%d\n", i);
            continue;
        }
        printf("[RXLos]: Port-%d: %lx\n", i, ioexp.data);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_port_lpmod_write(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Set All: Low power mode
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0xffffffff;
    err = ioctl(fd, PIO_IOCTL_LPMOD_WRITE, &ioexp);
    if (err < 0) {
        printf("[LPMod]: Set all fail!\n");
    }

    sleep(1);

    // Set All: High power mode
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_LPMOD_WRITE, &ioexp);
    if (err < 0) {
        printf("[LPMod]: Set all fail!\n");
    }

    sleep(1);

    // Set one: Low power mode
    for (i=1; i<=port_total; i++) {

        sleep(1);
        ioexp.port_num = i;
        ioexp.data = 1;

        err = ioctl(fd, PIO_IOCTL_LPMOD_WRITE, &ioexp);
        if (err < 0) {
            printf("[LPMod]: <Low> Set one Fail! <num>:%d\n", i);
            continue;
        }
        printf("[LPMod]: <Low> Set one OK! <num>:%d\n", i);
    }

    // Set one: High power mode
    for (i=1; i<=port_total; i++) {

        sleep(1);
        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_LPMOD_WRITE, &ioexp);
        if (err < 0) {
            printf("[LPMod]: <High> Set one Fail! <num>:%d\n", i);
            continue;
        }
        printf("[LPMod]: <High> Set one OK! <num>:%d\n", i);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}



void test_port_reset_write(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    pio_args_ioexp_t ioexp = {};

    printf("===== %s =====\n",__FUNCTION__);

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Set All: Reset mode
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0;
    err = ioctl(fd, PIO_IOCTL_REST_WRITE, &ioexp);
    if (err < 0) {
        printf("[RESET]: <In-RESET> Set all fail!\n");
    }

    sleep(1);

    // Set All: non-Reset mode
    ioexp.port_num = PIO_RW_IOEXP_REQ_GET_ALL;
    ioexp.data = 0xffffffff;
    err = ioctl(fd, PIO_IOCTL_REST_WRITE, &ioexp);
    if (err < 0) {
        printf("[RESET]: <Normal> Set all fail!\n");
    }

    sleep(1);

    // Set one: Reset mode
    for (i=1; i<=port_total; i++) {

        sleep(1);        
        ioexp.port_num = i;
        ioexp.data = 0;

        err = ioctl(fd, PIO_IOCTL_REST_WRITE, &ioexp);
        if (err < 0) {
            printf("[RESET]: <In-RESET> Set one Fail! <num>:%d\n", i);
            continue;
        }
        printf("[RESET]: <In-RESET> Set one OK! <num>:%d\n", i);
    }

    // Set one: non-Reset mode
    for (i=1; i<=port_total; i++) {

        sleep(1);        
        ioexp.port_num = i;
        ioexp.data = 1;

        err = ioctl(fd, PIO_IOCTL_REST_WRITE, &ioexp);
        if (err < 0) {
            printf("[RESET]: <Normal> Set one Fail! <num>:%d\n", i);
            continue;
        }
        printf("[RESET]: <Normal> Set one OK! <num>:%d\n", i);
    }

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


int main(void)
{
    test_xcvr_eeprom_read();
    test_xcvr_eeprom_write();
    test_port_present();
    test_port_lpmod_read();
    test_port_reset_read();
    test_port_rxlos_read();
    test_port_lpmod_write();
    test_port_reset_write();
}
