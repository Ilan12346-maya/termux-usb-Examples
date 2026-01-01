#!/bin/bash

echo "Listing information for all connected USB devices:"
echo "--------------------------------------------------"

# Get the list of USB device paths from termux-usb -l
# We need to parse the JSON output to extract the paths.
# jq is a common tool for JSON parsing in shell.
# If jq is not installed, this script will fail.
# For now, let's assume jq is available or handle its absence gracefully.
DEVICE_PATHS=$(termux-usb -l | jq -r '.[]')

if [ -z "$DEVICE_PATHS" ]; then
    echo "No USB devices detected or parsing failed. Please ensure devices are connected and jq is installed."
else
    for DEVICE_PATH in $DEVICE_PATHS; do
        echo ""
        echo "Device Path: $DEVICE_PATH"
        echo "--------------------"
        # Execute usb_info for each device path
        # The -r option ensures read access.
        # The -e option executes ./usb_info and passes the file descriptor as an argument.
        termux-usb -r -e ./usb_info "$DEVICE_PATH"
        if [ $? -ne 0 ]; then
            echo "Error getting info for $DEVICE_PATH. Check permissions or device status."
        fi
    done
fi

echo "--------------------------------------------------"
echo "Finished listing USB device information."
