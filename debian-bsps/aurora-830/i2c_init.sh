#!/bin/bash
# Netberg Aurora 830 basic init 

#Load modules
modprobe i2c-mux-pca954x
modprobe optoe

##SYSEEPROM
echo 24c32 0x57 > /sys/bus/i2c/devices/i2c-0/new_device
sleep 1
hexdump -n 256 -C /sys/bus/i2c/devices/0-0057/eeprom

#Init CPU_HOST_I2C
echo pca9548 0x73 > /sys/bus/i2c/devices/i2c-0/new_device 
echo '-2' > /sys/bus/i2c/devices/0-0073/idle_state


#Init CPU_LEG_I2C
## SFP root
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-0/new_device
echo '-2' > /sys/bus/i2c/devices/0-0072/idle_state

##QSFPDD leafs
for I in {9..12}; 
do 
	echo pca9548 0x76 > /sys/bus/i2c/devices/i2c-$I/new_device
	echo '-2' > /sys/bus/i2c/devices/$I-0076/idle_state
done

##QSFPDD EEPROMs
echo "QSFPDD EEPROMs"
for I in {17..48}; 
do 
    echo optoe3 0x50 > /sys/bus/i2c/devices/i2c-$I/new_device
    echo port$(($I-18)) > /sys/bus/i2c/devices/$I-0050/port_name
    cat /sys/bus/i2c/devices/$I-0050/port_name
done
echo "done"

echo "SFP+ EEPROMs"
echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-14/new_device
echo sfp2 > /sys/bus/i2c/devices/14-0050/port_name
cat /sys/bus/i2c/devices/14-0050/port_name

echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-15/new_device
echo sfp3 > /sys/bus/i2c/devices/15-0050/port_name
cat /sys/bus/i2c/devices/15-0050/port_name

modprobe x86_64_netberg_aurora_830_cpld
echo aurora_830_cpld1 0x30 > /sys/bus/i2c/devices/i2c-2/new_device
echo aurora_830_cpld2 0x31 > /sys/bus/i2c/devices/i2c-2/new_device
echo aurora_830_cpld3 0x32 > /sys/bus/i2c/devices/i2c-2/new_device
