# Termux USB Hardware test

This repository contains a collection of shell scripts and C programs designed for interacting with USB devices within a Termux environment on Android. These scripts can serve as a foundation for exploring USB device information and raw input from gamepads or usb-Serial

## Contents

-   `get_device_descriptors.sh`: Shell script to run to display device descriptors.
-   `list_all_usb_info.sh`: A shell script to list general information about all connected USB devices.
-   `read_gamepad_raw.sh`: Shell script to run and display raw gamepad input.
-   `usb_info.sh`: Shell script to run to display USB information.
-   `read_serial.sh`: USB Serial example tested with Arduino Leonardo 

## Purpose

The primary goal of these scripts is to provide a starting point for developers and enthusiasts who want to:

-   Understand how USB devices are enumerated and described on an Android device via Termux.
-   Access raw input data from USB gamepads (e.g., Xbox gamepads) in a command-line environment.
-   Develop custom applications or drivers that interact with USB hardware in termux.

## Contribution

Feel free to fork this repository, submit pull requests, or open issues if you find bugs or have suggestions for improvements. This project is intended as a foundation for further development.
