# USB Serial Communication

This directory contains a C program for reading data from a USB serial device (e.g., an Arduino) in Termux.

![Read Serial](https://github.com/Ilan12346-maya/termux-usb-examples/blob/main/usb-serial/read_serial.jpg)

## Files

-   **`read_serial.c`**: A C program that reads data from a USB serial device using `libusb`.

## How It Works

The `read_serial.c` program is specifically designed to interact with CDC-ACM (Communication Device Class - Abstract Control Model) compatible serial devices, such as Arduinos, within the Termux environment. It has been tested with an Arduino Leonardo continuously sending a text string every second at a baud rate of 9600. It uses `libusb` and the `termux-usb` utility to access the device. The process involves several key steps:

1.  **Device Access & Wrapping**:
    *   The program receives a file descriptor for the USB device, provided by `termux-usb -e`.
    *   It initializes `libusb` and uses `libusb_wrap_sys_device` to take control of the device via this file descriptor, bypassing standard device discovery.

2.  **Kernel Driver Management**:
    *   The program identifies the control and data interfaces (typically interface 0 and 1 for CDC-ACM devices).
    *   It checks if the Android kernel has active drivers on these interfaces. If so, it temporarily detaches them using `libusb_detach_kernel_driver` to gain exclusive access. This is crucial for preventing "Resource Busy" errors.

3.  **Interface Claiming**:
    *   It claims both the control and data interfaces using `libusb_claim_interface`, allowing the program to send and receive data.

4.  **Serial Port Configuration (CDC-ACM)**:
    *   The program sends control transfers (`libusb_control_transfer`) to configure the serial port parameters, such as the baud rate (e.g., 9600), data bits, parity, and stop bits (`SET_LINE_CODING`).
    *   It also sets the DTR (Data Terminal Ready) and RTS (Request To Send) states (`SET_CONTROL_LINE_STATE`), which are often necessary for establishing communication with serial devices.

5.  **Data Reading**:
    *   It enters a continuous loop, using `libusb_bulk_transfer` to read incoming data from the device's bulk IN endpoint.
    *   Received data is printed to `stderr`. The loop includes error handling for timeouts, pipe errors (attempting to clear them), and device disconnection.

6.  **Cleanup & Driver Re-attachment**:
    *   Upon exiting the loop or encountering a critical error, the program releases the claimed interfaces (`libusb_release_interface`).
    *   Crucially, it re-attaches any kernel drivers that were previously detached (`libusb_attach_kernel_driver`), returning control of the device to the operating system.

## Usage

1.  Identify the device path of your USB serial device using a tool like `termux-usb -l`.

2.  Use `termux-usb` to execute the `read_serial` program with the device path:

    ```bash
    termux-usb -e ./read_serial /dev/bus/usb/001/004
    ```

    (Replace `/dev/bus/usb/001/004` with the actual device path of your USB serial device.)

The program will then read and print any data sent from the serial device.
