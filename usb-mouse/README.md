# USB Mouse Communication

This directory contains C programs and a header file for interacting with USB mice in Termux.

## Files

-   **`mouse_decode.h`**: This header file defines the structure and bitmasks required to decode the data reports typically sent by USB mice.

-   **`read_mouse_raw.c`**: This C program reads raw interrupt data directly from a USB mouse. It takes a file descriptor (provided by `termux-usb`) and continuously polls the mouse's interrupt IN endpoint. It prints the received raw hexadecimal bytes to `stderr`, allowing developers to see the exact data stream from the device.

-   **`read_mouse.c`**: This C program builds upon `read_mouse_raw.c` by incorporating the decoding logic from `mouse_decode.h`. It reads the same raw interrupt data but then parses and interprets it into a human-readable format. This includes:
    *   Decoding button presses.
    *   Interpreting X and Y coordinates for mouse movement.
    *   It provides a dynamic, updating display of the mouse's state directly in the terminal.

## How It Works (Common to C Programs)

Both `read_mouse_raw.c` and `read_mouse.c` utilize the `libusb` library to interact with USB mice within Termux. Their operational steps are:
1.  **Device Access & Wrapping**:
    *   They receive a file descriptor for the USB device from `termux-usb -e`.
    *   `libusb` is initialized, and `libusb_wrap_sys_device` is used to take control of the device handle via this file descriptor, bypassing `libusb`'s usual device discovery process.

2.  **Kernel Driver Management**:
    *   The programs check if the Android kernel has claimed an active driver for the mouse's interface (typically interface 0 for HID devices).
    *   If a driver is active, `libusb_detach_kernel_driver` is used to temporarily detach it, ensuring the program can claim exclusive access to the interface without conflict.

3.  **Interface Claiming**:
    *   The mouse's interface is claimed using `libusb_claim_interface` to enable data transfer.

4.  **Data Transfer (Interrupt)**:
    *   Data is read from the mouse using `libusb_interrupt_transfer` on its interrupt IN endpoint. Mice typically use interrupt transfers for their event-driven nature (button presses, movement).

5.  **Cleanup & Driver Re-attachment**:
    *   Upon program termination or error, the claimed interface is released (`libusb_release_interface`).
    *   Any previously detached kernel drivers are re-attached (`libusb_attach_kernel_driver`) to restore the operating system's control over the device.

## Usage

1.  Identify the device path of your USB mouse using a tool like `termux-usb -l`.

2.  Use `termux-usb` to execute the desired program with the device path:

    *   **To read raw mouse data:**

        ```bash
        termux-usb -e ./read_mouse_raw /dev/bus/usb/001/002
        ```

    *   **To read decoded mouse data:**

        ```bash
        termux-usb -e ./read_mouse /dev/bus/usb/001/002
        ```

    (Replace `/dev/bus/usb/001/002` with the actual device path of your USB mouse.)
