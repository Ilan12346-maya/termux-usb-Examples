# USB Serial Communication

This directory contains a C program and a shell script for reading data from a USB serial device (tested with Arduino Leonardo) in Termux.

## Files

-   **`read_serial.c`**: A C program that reads data from a USB serial device using `libusb`.
-   **`read_serial.sh`**: A wrapper script that executes the `read_serial` program with the file descriptor provided by `termux-usb`.

## How It Works

These scripts rely on `libusb` and the `termux-usb` utility to access the USB device. The process involves the following steps:

1.  **Device Access**: The `termux-usb` command provides a file descriptor for the specified USB device to the `read_serial.sh` script.

2.  **Device Wrapping**: `read_serial.c` uses `libusb_wrap_sys_device` to wrap the system device associated with the file descriptor. This allows `libusb` to work with the device without needing to perform device discovery.

3.  **Kernel Driver**: The program checks if a kernel driver is already attached to the device. If so, it detaches the kernel driver to allow `libusb` to claim the interface.

4.  **Claiming Interface**: The program claims the USB interface to begin communication.

5.  **Bulk Transfer**: The program uses `libusb_bulk_transfer` to read data from the serial device's bulk endpoint.

6.  **Driver Re-attachment**: After reading the data, the program re-attaches the kernel driver so that the operating system can resume normal operation with the device.

## Usage

1.  Identify the device path of your USB serial device using `../../util/list_all_usb_info.sh`.

2.  Use `termux-usb` to execute the `read_serial.sh` script with the device path:

    ```bash
    termux-usb -e ./read_serial /dev/bus/usb/001/004
    ```

    (Replace `/dev/bus/usb/001/004` with the actual device path of your USB serial device.)

The program will then read and print any data sent from the serial device.
