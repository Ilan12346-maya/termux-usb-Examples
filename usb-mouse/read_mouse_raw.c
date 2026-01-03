#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <unistd.h> // For close
#include <string.h> // For memset
#include <time.h> // For time functions

// VENDOR_ID and PRODUCT_ID are not strictly necessary when using wrap_sys_device,
// but can be used for identification or specific device handling if needed.
// For now, using generic placeholders or removing them.
// #define VENDOR_ID 0xXXXX
// #define PRODUCT_ID 0xXXXX

// Helper function to convert transfer type to string for better readability
const char* libusb_transfer_type_to_string(enum libusb_transfer_type type) {
    switch (type) {
        case LIBUSB_TRANSFER_TYPE_CONTROL: return "Control";
        case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS: return "Isochronous";
        case LIBUSB_TRANSFER_TYPE_BULK: return "Bulk";
        case LIBUSB_TRANSFER_TYPE_INTERRUPT: return "Interrupt";
        default: return "Unknown";
    }
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    libusb_context *context = NULL;
    libusb_device_handle *handle = NULL;
    int fd = -1;
    int r = 0;
    unsigned char data[8]; // Typical mouse HID reports are 3-8 bytes
    int actual_length;
    int interface_number = 1; // Common for mouse HID, from device descriptors
    int endpoint_address = 0x82; // Common Interrupt IN endpoint for mouse HID, from device descriptors
    int max_packet_size = sizeof(data); // Use the buffer size


    if (argc < 2 || sscanf(argv[1], "%d", &fd) != 1) {
        fprintf(stderr, "Usage: %s <file_descriptor>\n", argv[0]);
        return 1;
    }

    libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    r = libusb_init(&context);
    if (r < 0) {
        fprintf(stderr, "libusb_init failed: %s\n", libusb_error_name(r));
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_init successful.\n");

    r = libusb_wrap_sys_device(context, (intptr_t)fd, &handle);
    if (r < 0) {
        fprintf(stderr, "libusb_wrap_sys_device failed: %s\n", libusb_error_name(r));
        libusb_exit(context);
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_wrap_sys_device successful. Handle obtained.\n");

    // Try to detach kernel driver if one is active for Interface 0
    int kernel_driver_active = 0; 
    fprintf(stderr, "DEBUG: Checking for active kernel driver on interface %d.\n", interface_number);
    if (libusb_kernel_driver_active(handle, interface_number) == 1) {
        fprintf(stderr, "DEBUG: Kernel driver active. Attempting to detach.\n");
        r = libusb_detach_kernel_driver(handle, interface_number);
        if (r < 0) {
            fprintf(stderr, "libusb_detach_kernel_driver failed (interface %d): %s\n", interface_number, libusb_error_name(r));
            goto error_exit_with_handle;
        }
        kernel_driver_active = 1;
        fprintf(stderr, "DEBUG: Kernel driver detached.\n");
    } else {
        fprintf(stderr, "DEBUG: No active kernel driver found or already detached.\n");
    }

    fprintf(stderr, "DEBUG: Attempting to claim interface %d.\n", interface_number);
    r = libusb_claim_interface(handle, interface_number);
    if (r < 0) {
        fprintf(stderr, "libusb_claim_interface failed (interface %d): %s\n", interface_number, libusb_error_name(r));
        if (kernel_driver_active) {
            fprintf(stderr, "DEBUG: Reattaching kernel driver.\n");
            libusb_attach_kernel_driver(handle, interface_number); // Reattach if we detached it
        }
        goto error_exit_with_handle;
    }
    fprintf(stderr, "DEBUG: Interface %d claimed successfully.\n", interface_number);

    fprintf(stderr, "Reading raw HID input from USB mouse (Press Ctrl+C to stop):\n");
    fprintf(stderr, "Endpoint Address: 0x%02x, Interface: %d, Max Packet Size: %d\n", endpoint_address, interface_number, max_packet_size);
    fprintf(stderr, "DEBUG: Entering polling loop.\n");

    while (1) {
        r = libusb_interrupt_transfer(handle, endpoint_address, data, max_packet_size, &actual_length, 100);
        if (r == LIBUSB_ERROR_TIMEOUT) {
            continue; // No data received yet, continue polling
        } else if (r < 0) {
            if (r == LIBUSB_ERROR_NO_DEVICE) {
                fprintf(stderr, "\nERROR: Device disconnected. Exiting.\n");
                break; // Exit the loop
            } else if (r == LIBUSB_ERROR_PIPE) {
                 fprintf(stderr, "libusb_interrupt_transfer error: LIBUSB_ERROR_PIPE (endpoint halted). Retrying...\n");
                 libusb_clear_halt(handle, endpoint_address);
                 usleep(100000); // Wait 100ms before retrying
                 continue;
            }
            fprintf(stderr, "libusb_interrupt_transfer failed: %s\n", libusb_error_name(r));
            break;
        }

        if (actual_length > 0) {
            fprintf(stderr, "Received %d bytes: ", actual_length);
            for (int i = 0; i < actual_length; ++i) {
                fprintf(stderr, "%02x ", data[i]);
            }
            fprintf(stderr, "\n");
        }
    }

    // Cleanup upon successful exit or break from loop
    libusb_release_interface(handle, interface_number);
    if (kernel_driver_active) {
        libusb_attach_kernel_driver(handle, interface_number); // Reattach if we detached it
    }
    
    libusb_close(handle);
    libusb_exit(context);
    return 0;

error_exit_with_handle:
    if (handle) { // Only close handle if it was successfully opened
        libusb_close(handle);
    }
    // Fall through to common context exit
// error_exit_no_config: // Label removed as it's not needed for this simplified code
    if (context) { // Only exit context if it was successfully initialized
        libusb_exit(context);
    }
    return 1;
}
