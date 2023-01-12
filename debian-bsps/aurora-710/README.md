# Installing i2c_utils.sh
```bash
 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/i2c_utils.sh -O /usr/sbin/i2c_utils.sh
 sudo chmod +x /usr/sbin/i2c_utils.sh
```
# Init the switch
With this, you can initialize the switch i2c tree:
```bash
 sudo i2c_utils.sh i2c_init
```
There are a lot of useful i2c_utils.sh options. 
Just execute it without parameters to show all of them. 

# Example of a simple systemd monitoring services
```bash
 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/aurora-710-monitor.service -O /etc/systemd/system/aurora-710-monitor.service

 sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-710/aurora_710_monitor.sh -O /usr/sbin/aurora_710_monitor.sh
 sudo chmod +x /usr/sbin/aurora_710_monitor.sh

 sudo systemctl enable aurora-710-monitor
```
This script will automatically run during the system boot, 
start the i2c_init function, and control fans/LEDs.

