# Installing kernel modules

```bash
git clone https://github.com/netbergtw/files.git
cd files/debian-bsps/aurora-830/nba830
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
depmod -a
```

# Installing init example

## Init hardware

```bash
sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-830/i2c_init.sh -O /usr/sbin/i2c_init.sh
sudo chmod +x /usr/sbin/i2c_init.sh

sudo wget https://github.com/netbergtw/files/raw/master/debian-bsps/aurora-221/aurora-sw.service -O /etc/systemd/system/aurora-sw.service
sudo systemctl enable aurora-sw
``` 

## Periodic transceiver temperature update

```bash
sudo wget https://github.com/netbergtw/files/raw/refs/heads/master/debian-bsps/aurora-830/update_optics_temp.sh -U /usr/sbin/update_optics_temp.sh
sudo chmod +x /usr/sbin/update_optics_temp.sh

sudo wget https://github.com/netbergtw/files/raw/refs/heads/master/debian-bsps/aurora-830/update_optics_temp.service -O /etc/systemd/system/update_optics_temp.service

sudo wget https://github.com/netbergtw/files/raw/refs/heads/master/debian-bsps/aurora-830/update_optics_temp.timer -O /etc/systemd/system/update_optics_temp.timer

sudo systemctl daemon-reload

sudo systemctl enable --now update_optics_temp.timer

```



