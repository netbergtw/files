- [OS installation](#os-installation)
- [Ubuntu 24.04](#ubuntu-2404-installation)
- [Ubuntu 20.04](#ubuntu-2004-installation)
- [Model dependent steps](#model-dependent-steps)

# OS installation

We are going through a standard ONIE procedure.

Please refer to https://netbergtw.com/top-support/articles/onie-recovery-on-x86-enabled-netberg-aurora-switches/ if ONIE is missing.

# Ubuntu 24.04 installation

You can a get ready to use Ubuntu 24.04 [ONIE image](http://www.netbergtw.com/wp-content/uploads/Files/ubuntu-noble-amd64-mini-ONIE.bin)
and [user-data](user-data), [meta-data](meta-data), and
[vendor-data](vendor-data) for the autoinstall proccess.

Install the OS with the `onie-nos-install` command from a remote
or local source (remember to put the data files and Ubuntu installation
[ISO](https://releases.ubuntu.com/24.04/ubuntu-24.04.1-live-server-amd64.iso)
**near the image**).

```bash
 onie-nos-install http://192.168.0.4:8000/ubuntu-noble-amd64-mini-ONIE.bin
```

Also, you can redefine the ubuntu iso name and location with
the following command:

```bash
 export os_iso_url=http://your.local.server/ubuntu-24.04.1-live-server-amd64.iso
```

## Post-intallation steps:

Do not load the ast module until you know what you are doing.
```bash
echo "blacklist ast" >> /etc/modprobe.d/blacklist.conf
```


### Update repos:
```bash
 sudo apt update
 sudo apt upgrade
 sudo reboot
```

### Install additional components:
```bash
 sudo apt install openssh-server build-essential git i2c-tools
```

### Build optoe driver
```bash
 git clone https://github.com/netbergtw/oom.git --branch kern-68 --single-branch
 cd oom/optoe
 echo "ccflags-y := -DLATEST_KERNEL68" > Makefile
 echo "obj-m := optoe.o" >> Makefile
 make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
 sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
 sudo depmod
```

# Ubuntu 20.04 installation

You can [make your own ONIE image](https://github.com/opencomputeproject/onie/blob/master/contrib/debian-iso/README.md) 
with the preseed file or get a ready to use Ubuntu 20.04 [image](http://www.netbergtw.com/wp-content/uploads/Files/ubuntu-focal-amd64-mini-ONIE.bin) 
and [preseed](debian-preseed.txt):

Install OS with the `onie-nos-install` command from a remote or local source (don’t forget to put debian-preseed.txt near the image).
```bash
 onie-nos-install http://192.168.0.4:8000/ubuntu-focal-amd64-mini-ONIE.bin
```

## During installation

**Please unplug the USB drive if inserted** or follow debian-preseed.txt

```
 # Due notably to potential USB sticks, the location of the MBR can not be
 # determined safely in general, so this needs to be specified:
 #d-i grub-installer/bootdev  string /dev/sdb
 # To install to the first device (assuming it is not a USB stick):
 d-i grub-installer/bootdev  string default
```

Select the proper network device: e.g. enp8s0: Intel Corporation I210 Gigabit Network Connection.
Agree to a weak password (onie/onie is the default user/password).

## Post-intallation steps:

### Update repos and install the latest kernel (tested with 5.15.0-91):
```bash
 sudo apt update
 sudo apt upgrade
 sudo apt install --install-recommends linux-generic-hwe-20.04
 sudo reboot
```

### Install additional components:
```bash
 sudo apt install openssh-server build-essential git i2c-tools
```
### Build optoe driver
```bash
 git clone https://github.com/opencomputeproject/oom.git
 cd oom/optoe
 echo "ccflags-y := -DLATEST_KERNEL" > Makefile
 echo "obj-m := optoe.o" >> Makefile
 make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
 sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules_install
 sudo depmod
```

# Model dependent steps

See the appropriate folder for instructions for specific models:

- [Aurora 710](aurora-710/README.md)
- [Aurora 750](aurora-750/README.md)
- [Aurora 610](aurora-610/README.md)
- [Aurora 810](aurora-810/README.md)
- [Aurora 830](aurora-830/README.md)
- [Aurora 721](aurora-721/README.md)
- [Aurora 621](aurora-621/README.md)
- [Aurora 221](aurora-221/README.md)
