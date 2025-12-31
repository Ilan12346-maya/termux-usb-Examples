# termux-usb-xinput


Termux-usb Xinput Test Script example, by Gemini-Cli in Yolo-Mode 🫣 

# Termux USB Test Scripts

This repository contains a collection of shell scripts and C programs designed for interacting with USB devices, specifically focusing on gamepads, within a Termux environment on Android. These scripts can serve as a foundation for exploring USB device information and raw input from gamepads.

## Contents

-   `get_device_descriptors/`: Contains scripts and C code to retrieve detailed USB device descriptors.
    -   `get_device_descriptors.c`: C source code for fetching USB device descriptors.
    -   `get_device_descriptors.sh`: Shell script to compile and run the C code to display device descriptors.
-   `list_all_usb_info.sh`: A shell script to list general information about all connected USB devices.
-   `read_gamepad_raw/`: Contains scripts and C code for reading raw input from a gamepad.
    -   `read_gamepad_raw.c`: C source code for reading raw gamepad input.
    -   `read_gamepad_raw.sh`: Shell script to compile and run the C code to display raw gamepad input.
-   `usb_info/`: Contains scripts and C code to display basic USB device information.
    -   `usb_info.c`: C source code for fetching basic USB information.
    -   `usb_info.sh`: Shell script to compile and run the C code to display USB information.

## Purpose

The primary goal of these scripts is to provide a starting point for developers and enthusiasts who want to:

-   Understand how USB devices are enumerated and described on an Android device via Termux.
-   Access raw input data from USB gamepads (e.g., Xbox gamepads) in a command-line environment.
-   Develop custom applications or drivers that interact with USB hardware on Android.

## Usage in Termux

To use these scripts in Termux:

1.  **Install necessary tools**: You'll likely need `clang` for compiling C code and `make` if a `Makefile` is present (though currently, compilation is handled directly in `.sh` files).
    ```bash
    pkg install clang make
    ```
2.  **Navigate to the script directory**:
    ```bash
    cd /data/data/com.termux/files/home/adbshellrun/usb-test/
    ```
3.  **Run the desired script**: For example, to list all USB info:
    ```bash
    ./list_all_usb_info.sh
    ```
    Or to get device descriptors:
    ```bash
    ./get_device_descriptors/get_device_descriptors.sh
    ```

## Contribution

Feel free to fork this repository, submit pull requests, or open issues if you find bugs or have suggestions for improvements. This project is intended as a foundation for further development.
