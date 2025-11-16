# hid_rapoo

Battery reporting for Rapoo VT3 MAX Gen-2. 

Tested on Fedora Linux 43.


# Building

Kernel headers must be installed.

```bash
make
```

# Installing

Temporary (DKMS soon):
```bash
sudo insmod hid_rapoo.ko
```

# Removing

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