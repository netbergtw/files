# Installing lm-sensors
```bash
sudo apt install lm-sensors
```

# Installing kernel modules

```bash
git clone https://github.com/netbergtw/files.git
cd files/debian-bsps/aurora-221/nba221
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
sudo depmod -a
```

# Installing init example
```bash
sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-221/i2c_init.sh -O /usr/sbin/i2c_init.sh
sudo chmod +x /usr/sbin/i2c_init.sh

sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-221/aurora-sw.service -O /etc/systemd/system/aurora-sw.service
sudo systemctl enable aurora-sw
``` 
