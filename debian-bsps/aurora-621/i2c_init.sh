#!/bin/bash
# Netberg Aurora 621 basic init 

GPIO_OFFSET=0

function set_gpio_offset {
    GPIO_OFFSET=0
    gpio_base_min=`cat /sys/class/gpio/gpiochip*/base | sort -n | head -1`
    GPIO_OFFSET=$(($gpio_base_min))
    echo "set GPIO_OFFSET=${GPIO_OFFSET}"
}

#Load modules
modprobe i2c-mux-pca954x
modprobe optoe

#Init CPU_HOST_I2C
echo pca9548 0x70 > /sys/bus/i2c/devices/i2c-0/new_device 
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-0/new_device

echo '-2' > /sys/bus/i2c/devices/0-0070/idle_state
echo '-2' > /sys/bus/i2c/devices/0-0071/idle_state

##SYSEEPROM
echo 24c32 0x53 > /sys/bus/i2c/devices/i2c-5/new_device

#Init CPU_LEG_I2C
## SFP root
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-1/new_device
echo '-2' > /sys/bus/i2c/devices/1-0072/idle_state

##SFP leafs
for I in {18..24}; 
do 
	echo pca9548 0x73 > /sys/bus/i2c/devices/i2c-$I/new_device
	echo '-2' > /sys/bus/i2c/devices/$I-0073/idle_state
done

##SFP EEPROMs
echo -n "SFP28 EEPROMs"
for I in {26..73}; 
do 
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-$I/new_device
    echo -n '.'
done
echo "done"

echo -n "QSFP28 EEPROMs"
for I in {74..79}; 
do 
    echo optoe1 0x50 > /sys/bus/i2c/devices/i2c-$I/new_device
    echo -n '.'
done
echo "done"

modprobe gpio-pca953x

#6424_RXRS_2,  SFP 0-23  rx rate select
echo tca6424 0x22 > /sys/bus/i2c/devices/i2c-7/new_device

#6424_RXRS_1, SFP 24-47 rx rate select
echo tca6424 0x22 > /sys/bus/i2c/devices/i2c-6/new_device

#6424_TXRS_2, SFP 0-23  tx rate select
echo tca6424 0x23 > /sys/bus/i2c/devices/i2c-7/new_device

#6424_TXRS_1,  SFP 24-47 tx rate select
echo tca6424 0x23 > /sys/bus/i2c/devices/i2c-6/new_device

set_gpio_offset

for((i=${GPIO_OFFSET};i<=${GPIO_OFFSET}+95;i++));
do
    echo $i > /sys/class/gpio/export
    echo high > /sys/class/gpio/gpio${i}/direction
done

modprobe x86_64_netberg_aurora_621_cpld
echo aurora_621_cpld1 0x30 > /sys/bus/i2c/devices/i2c-2/new_device
echo aurora_621_cpld2 0x31 > /sys/bus/i2c/devices/i2c-2/new_device
