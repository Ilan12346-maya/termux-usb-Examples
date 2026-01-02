# USB Gamepad Communication

This directory contains C programs and a header file for interacting with USB gamepads in Termux.

![Read Gamepad](https://github.com/Ilan12346-maya/termux-usb-examples/blob/main/usb-gamepad/read_gamepad.png)


## Files

-   **`gamepad_decode.h`**: This header file defines the structure and bitmasks required to decode the 20-byte data reports typically sent by USB gamepads. It provides constants for identifying D-pad states, button presses (e.g., A, B, X, Y, L1, R1, Home, Start, Back, L3, R3), and defines the `GamepadReport` struct for a byte-level mapping of the raw data, including analog stick and trigger values.

-   **`read_gamepad_raw.c`**: This C program reads raw interrupt data directly from a USB gamepad. It takes a file descriptor (provided by `termux-usb`) and continuously polls the gamepad's interrupt IN endpoint. It prints the received raw hexadecimal bytes to `stderr`, allowing developers to see the exact data stream from the device.

-   **`read_gamepad.c`**: This C program builds upon `read_gamepad_raw.c` by incorporating the decoding logic from `gamepad_decode.h`. It reads the same raw interrupt data but then parses and interprets it into a human-readable format. This includes:
    *   Decoding D-pad states and various button presses.
    *   Extracting analog values for triggers.
    *   Interpreting X and Y coordinates for left and right analog sticks (signed 16-bit values).
    *   It provides a dynamic, updating display of the gamepad's state directly in the terminal, including graphical representations for analog triggers.

## How It Works (Common to C Programs)

Both `read_gamepad_raw.c` and `read_gamepad.c` utilize the `libusb` library to interact with USB gamepads within Termux. Their operational steps are:
1.  **Device Access & Wrapping**:
    *   They receive a file descriptor for the USB device from `termux-usb -e`.
    *   `libusb` is initialized, and `libusb_wrap_sys_device` is used to take control of the device handle via this file descriptor, bypassing `libusb`'s usual device discovery process.

2.  **Kernel Driver Management**:
    *   The programs check if the Android kernel has claimed an active driver for the gamepad's interface (typically interface 0 for HID devices).
    *   If a driver is active, `libusb_detach_kernel_driver` is used to temporarily detach it, ensuring the program can claim exclusive access to the interface without conflict.

3.  **Interface Claiming**:
    *   The gamepad's interface is claimed using `libusb_claim_interface` to enable data transfer.

4.  **Data Transfer (Interrupt)**:
    *   Data is read from the gamepad using `libusb_interrupt_transfer` on its interrupt IN endpoint. Gamepads typically use interrupt transfers for their event-driven nature (button presses, stick movements).

5.  **Cleanup & Driver Re-attachment**:
    *   Upon program termination or error, the claimed interface is released (`libusb_release_interface`).
    *   Any previously detached kernel drivers are re-attached (`libusb_attach_kernel_driver`) to restore the operating system's control over the device.

## Usage

1.  Identify the device path of your USB gamepad using a tool like `termux-usb -l`.

2.  Use `termux-usb` to execute the desired program with the device path:

    *   **To read raw gamepad data:**

        ```bash
        termux-usb -e ./read_gamepad_raw /dev/bus/usb/001/005
        ```

    *   **To read decoded gamepad data:**

        ```bash
        termux-usb -e ./read_gamepad /dev/bus/usb/001/005
        ```

    (Replace `/dev/bus/usb/001/005` with the actual device path of your USB gamepad.)
