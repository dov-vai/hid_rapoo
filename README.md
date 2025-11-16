# hid_rapoo

Battery reporting for Rapoo VT3 MAX Gen-2. 

Tested on Fedora Linux 43.

## Prerequisites

Install kernel headers and DKMS:

**Fedora/RHEL:**
```bash
sudo dnf install kernel-devel kernel-headers dkms
```

**Ubuntu/Debian:**
```bash
sudo apt install linux-headers-$(uname -r) dkms
```

**Arch:**
```bash
sudo pacman -S linux-headers dkms
```

## Installation (Recommended - DKMS)

DKMS will automatically rebuild and load the module after kernel updates and on boot.

```bash
sudo ./install-dkms.sh
```

To verify installation:
```bash
lsmod | grep hid_rapoo
modinfo hid_rapoo
```

## Uninstalling (DKMS)

```bash
sudo ./uninstall-dkms.sh
```

## Manual Installation (Temporary)

If you want to build and test without DKMS:

```bash
make
sudo insmod hid_rapoo.ko
```

To remove:
```bash
sudo rmmod hid_rapoo
```

# Testing

Might work with other Rapoo devices (you try). Should be safe since it's only reading the information.

Modify:
```c
// hid_rapoo.c
// ...

#define USB_VENDOR_ID_RAPOO  0x24ae
#define USB_DEVICE_ID_RAPOO_VT3_MAX_GEN2 0x1417 // add your device ID

// ...

static const struct hid_device_id rapoo_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAPOO, USB_DEVICE_ID_RAPOO_VT3_MAX_GEN2) },
    // add your device here
    { }
};

// ...
```