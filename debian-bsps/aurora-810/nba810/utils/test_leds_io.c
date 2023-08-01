/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Leds IOCTL test and userspace sample code.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#include "../includes/leds_io.h"

#define DEVFILE "/dev/"LEDS_IOCTL_DEV_NAME


void test_portled_allcolors(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    lio_args_port_led_t led = {};

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Show Off
    printf("[PortLED]: Show Off (0x%x)\n", LEDS_PORT_RGB_OFF);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_OFF;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show Green
    printf("[PortLED]: Show Green (0x%x)\n", LEDS_PORT_RGB_GREEN);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_GREEN;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show Blue
    printf("[PortLED]: Show Blue (0x%x)\n", LEDS_PORT_RGB_BLUE);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_BLUE;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show Tiffany
    printf("[PortLED]: Show Tiffany (0x%x)\n", LEDS_PORT_RGB_TIFFANY);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_TIFFANY;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show Red
    printf("[PortLED]: Show Red (0x%x)\n", LEDS_PORT_RGB_RED);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_RED;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show Yellow
    printf("[PortLED]: Show Yellow (0x%x)\n", LEDS_PORT_RGB_YELLOW);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_YELLOW;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show PURPLE
    printf("[PortLED]: Show Yellow (0x%x)\n", LEDS_PORT_RGB_PURPLE);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_PURPLE;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    // Show White
    printf("[PortLED]: Show White (0x%x)\n", LEDS_PORT_RGB_WHITE);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_WHITE;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }
    sleep(2);

    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}


void test_portled_get(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    lio_args_port_led_t led = {};

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }
   for (i=1; i<=port_total; i++) {
        led.port_num = i;
        err = ioctl(fd, LIO_IOCTL_PORT_GET, &led);
        if (err < 0) {
            printf("[PortLED]: Get Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Get LED(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    if (close(fd) != 0) {
        printf("close() fail\n");
    }

}

void test_portled_set(void)
{
    int fd;
    int i, j, err;
    int port_total = 32;
    lio_args_port_led_t led = {};

    fd = open(DEVFILE, O_RDWR);
    if (fd < 0) {
        perror("open() fail\n");
        return;
    }

    // Off
    printf("[PortLED]: All Off (0x%x)\n", LEDS_PORT_RGB_OFF);
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_OFF;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        }
    }

    // Test Green (LED-1: bit-0 / LED-2: Bit-3)
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_GREEN;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Set Green(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    // Test Blue (LED-1: bit-1 / LED-2: Bit-4)
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_BLUE;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Set Blue(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    // Test Red (LED-1: bit-2 / LED-2: Bit-5)
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_RED;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Set Red(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    // Test White (Set All Bit-0~5)
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = LEDS_PORT_RGB_WHITE;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Set White(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    // Test Off (Clear All Bit-0~5)
    for (i=1; i<=port_total; i++) {
        led.port_num = i;
        led.data = 0;
        err = ioctl(fd, LIO_IOCTL_PORT_SET, &led);
        if (err < 0) {
            printf("[PortLED]: Set Fail! <port>:%d\n", i);
        } else {
            printf("[PortLED]: Set Off(0x%x) <port>:%d\n", led.data, i);
        }
        sleep(1);
    }
    if (close(fd) != 0) {
        printf("close() fail\n");
    }
}

int main(void)
{
    test_portled_allcolors();
    test_portled_get();
    test_portled_set();
}
