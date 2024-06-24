#!/bin/bash
# Netberg Aurora 721 basic init 

#Load modules
modprobe i2c-mux-pca954x
modprobe optoe

#Init CPU_HOST_I2C
echo pca9548 0x70 > /sys/bus/i2c/devices/i2c-0/new_device 
echo '-2' > /sys/bus/i2c/devices/0-0070/idle_state

##SYSEEPROM
echo 24c32 0x57 > /sys/bus/i2c/devices/i2c-0/new_device
sleep 1
hexdump -n 256 -C /sys/bus/i2c/devices/0-0057/eeprom


#Init CPU_LEG_I2C
## SFP root
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-1/new_device
echo '-2' > /sys/bus/i2c/devices/1-0072/idle_state

##QSFP28 leafs
for I in {10..13}; 
do 
	echo pca9548 0x73 > /sys/bus/i2c/devices/i2c-$I/new_device
	echo '-2' > /sys/bus/i2c/devices/$I-0073/idle_state
done

##QSFP28 EEPROMs
echo "QSFP28 EEPROMs"
for I in {18..49}; 
do 
    echo optoe1 0x50 > /sys/bus/i2c/devices/i2c-$I/new_device
    echo port$(($I-17)) > /sys/bus/i2c/devices/$I-0050/port_name
    cat /sys/bus/i2c/devices/$I-0050/port_name
done
echo "done"

modprobe x86_64_netberg_aurora_721_cpld
echo aurora_721_cpld1 0x30 > /sys/bus/i2c/devices/i2c-2/new_device
echo aurora_721_cpld2 0x31 > /sys/bus/i2c/devices/i2c-2/new_device
