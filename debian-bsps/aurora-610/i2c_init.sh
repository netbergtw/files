#!/bin/bash

echo "Setting up board... " >> /dev/kmsg
INIT_ERROR="FALSE"

depmod -a

# Insert modules by order
rmmod lpc_ich
rmmod gpio_ich

modprobe gpio_ich gpiobase=0
modprobe lpc_ich
modprobe i2c-gpio
modprobe i2c-mux-pca954x
modprobe i2c-dev
modprobe net_platform
modprobe net_psoc
modprobe net_cpld
modprobe optoe

# wait net_platform init complete
# wait hwmon0 (coretemp) load complete to make sure psoc and cpld
PLATFORM_WAITING_COUNT=0
while [ $PLATFORM_WAITING_COUNT -le 10 ]
do
    if [ -d /sys/bus/i2c/devices/i2c-0/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-2/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-3/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-4/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-5/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-6/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-7/ ] &&
    [ -d /sys/bus/i2c/devices/i2c-8/ ] &&
    [ -d /sys/class/hwmon/hwmon0/ ]
    then
        break
    else
        #200ms for each step
        sleep 0.2
        let "PLATFORM_WAITING_COUNT++"
    fi
done


if [ -d "/sys/bus/i2c/devices/i2c-0/" ]
then
    echo net_cpld 0x77 > /sys/bus/i2c/devices/i2c-0/new_device
else
    echo "i2c-0 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

# IPMI support
modprobe ipmi_devintf

# Attach 48 instances of EEPROM driver SFP ports on IO module
#eeprom can dump data using below command
if [ -d "/sys/bus/i2c/devices/i2c-2/" ]
then
    for ((i=10;i<=17;i++));
    do
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-2/i2c-$i/new_device
    done
else
    echo "i2c-2 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

if [ -d "/sys/bus/i2c/devices/i2c-3/" ]
then
    for ((i=18;i<=25;i++));
    do
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-3/i2c-$i/new_device
    done
else
    echo "i2c-3 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi


if [ -d "/sys/bus/i2c/devices/i2c-4/" ]
then
    for ((i=26;i<=33;i++));
    do
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-4/i2c-$i/new_device
    done
else
    echo "i2c-4 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

if [ -d "/sys/bus/i2c/devices/i2c-5/" ]
then
  for ((i=34;i<=41;i++));
  do
  echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-5/i2c-$i/new_device
  done
else
    echo "i2c-5 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

if [ -d "/sys/bus/i2c/devices/i2c-6/" ]
then
    for ((i=42;i<=49;i++));
    do
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-6/i2c-$i/new_device
    done
else
    echo "i2c-6 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

if [ -d "/sys/bus/i2c/devices/i2c-7/" ]
then
    for ((i=50;i<=57;i++));
    do
    echo optoe2 0x50 > /sys/bus/i2c/devices/i2c-7/i2c-$i/new_device
    done
else
    echo "i2c-7 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

for ((i=1;i<=48;i++));
do
    echo port$i > /sys/bus/i2c/devices/$(($i+9))-0050/port_name
done

# Attach 6 instances of EEPROM driver QSFP ports on IO module
#eeprom can dump data using below command
if [ -d "/sys/bus/i2c/devices/i2c-8/" ]
then
    for ((i=58;i<=65;i++));
    do
    echo optoe1 0x50 > /sys/bus/i2c/devices/i2c-8/i2c-$i/new_device
    done
else
    echo "i2c-8 error" >> /dev/kmsg
    INIT_ERROR="TRUE"
fi

echo port50 > /sys/bus/i2c/devices/58-0050/port_name
echo port49 > /sys/bus/i2c/devices/59-0050/port_name
echo port52 > /sys/bus/i2c/devices/60-0050/port_name
echo port51 > /sys/bus/i2c/devices/61-0050/port_name
echo port54 > /sys/bus/i2c/devices/62-0050/port_name
echo port53 > /sys/bus/i2c/devices/63-0050/port_name
echo port56 > /sys/bus/i2c/devices/64-0050/port_name
echo port55 > /sys/bus/i2c/devices/65-0050/port_name

if [ $INIT_ERROR != "TRUE" ]
then
    case "$(cat /proc/cmdline)" in
      *fast-reboot*)
        modprobe swps io_no_init=1
        ;;
      # warm-reboot will be set to fastfast in barefoot
      *fastfast*)
        modprobe swps io_no_init=1
        ;;
      *)
        modprobe swps
        ;;
    esac
    modprobe at24
    echo 24c64 0x53 > /sys/bus/i2c/devices/i2c-0/new_device
else
    echo " AS: nba610 init fail" >> /dev/kmsg
fi

# Setting ARP garbage collection threshold values
echo "Setting IPv4 Neighbor GC threshold values."
sysctl -w net.ipv4.neigh.default.gc_thresh1=16384
sysctl -w net.ipv4.neigh.default.gc_thresh2=32768
sysctl -w net.ipv4.neigh.default.gc_thresh3=32768

echo "done."
