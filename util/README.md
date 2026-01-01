# Utilities for USB Device Inspection

This directory contains a collection of scripts and programs for listing and inspecting connected USB devices.

## Files

### `list_all_usb_info.sh`

This script iterates through all connected USB devices and prints general information for each one, such as the device path, vendor ID, product ID, manufacturer, and product name.

**Usage:**

```bash
./list_all_usb_info.sh
```

### `usb_info.c` and `usb_info.sh`

`usb_info.c` is a C program that displays general information about a single USB device, given its file descriptor. `usb_info.sh` is a wrapper script to be used with `termux-usb`.

**Usage:**

1.  Find the device path using `list_all_usb_info.sh`.
2.  Execute the script with `termux-usb`:

    ```bash
    termux-usb -e ./usb_info /dev/bus/usb/001/003
    ```

### `get_device_descriptors.c` and `get_device_descriptors.sh`

`get_device_descriptors.c` is a C program that prints detailed USB device and configuration descriptors for a given device. `get_device_descriptors.sh` is a wrapper script for use with `termux-usb`.

This is useful for in-depth USB device analysis, allowing you to see all endpoints, interfaces, and their properties.

**Usage:**

1.  Find the device path using `list_all_usb_info.sh`.
2.  Execute the program with `termux-usb`:

    ```bash
    termux-usb -e ./get_device_descriptors /dev/bus/usb/001/003
    ```
