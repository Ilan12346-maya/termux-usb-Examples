#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#define ARDUINO_CONTROL_INTERFACE 0
#define ARDUINO_DATA_INTERFACE 1
#define ARDUINO_ENDPOINT_IN 0x83
#define ARDUINO_ENDPOINT_OUT 0x02
#define ARDUINO_MAX_PACKET_SIZE 64

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    libusb_context *context = NULL;
    libusb_device_handle *handle = NULL;
    int fd = -1;
    int r = 0;
    int kernel_driver_detached_control = 0;
    int kernel_driver_detached_data = 0;

    fprintf(stderr, "DEBUG: Starting read_serial...\n");

    if (argc < 2 || sscanf(argv[1], "%d", &fd) != 1) {
        fprintf(stderr, "Usage: %s <file_descriptor>\n", argv[0]);
        return 1;
    }
    fprintf(stderr, "DEBUG: File descriptor from argument: %d\n", fd);

    libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    fprintf(stderr, "DEBUG: Calling libusb_init()...\n");
    r = libusb_init(&context);
    if (r < 0) {
        fprintf(stderr, "ERROR: libusb_init failed: %s\n", libusb_error_name(r));
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_init() successful.\n");

    fprintf(stderr, "DEBUG: Calling libusb_wrap_sys_device()...\n");
    r = libusb_wrap_sys_device(context, (intptr_t)fd, &handle);
    if (r < 0) {
        fprintf(stderr, "ERROR: libusb_wrap_sys_device failed: %s\n", libusb_error_name(r));
        libusb_exit(context);
        return 1;
    }
    if (!handle) {
        fprintf(stderr, "ERROR: libusb_wrap_sys_device returned a null handle.\n");
        libusb_exit(context);
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_wrap_sys_device() successful. Handle is not NULL.\n");

    // Detach kernel drivers if active
    if (libusb_kernel_driver_active(handle, ARDUINO_CONTROL_INTERFACE) == 1) {
        fprintf(stderr, "DEBUG: Detaching kernel driver from control interface %d\n", ARDUINO_CONTROL_INTERFACE);
        r = libusb_detach_kernel_driver(handle, ARDUINO_CONTROL_INTERFACE);
        if (r == 0) {
            kernel_driver_detached_control = 1;
        } else {
             fprintf(stderr, "ERROR: Failed to detach kernel driver from control interface: %s\n", libusb_error_name(r));
        }
    }
    if (libusb_kernel_driver_active(handle, ARDUINO_DATA_INTERFACE) == 1) {
        fprintf(stderr, "DEBUG: Detaching kernel driver from data interface %d\n", ARDUINO_DATA_INTERFACE);
        r = libusb_detach_kernel_driver(handle, ARDUINO_DATA_INTERFACE);
        if (r == 0) {
            kernel_driver_detached_data = 1;
        } else {
            fprintf(stderr, "ERROR: Failed to detach kernel driver from data interface: %s\n", libusb_error_name(r));
        }
    }

    fprintf(stderr, "DEBUG: Claiming control interface %d...\n", ARDUINO_CONTROL_INTERFACE);
    r = libusb_claim_interface(handle, ARDUINO_CONTROL_INTERFACE);
    if (r < 0) {
        fprintf(stderr, "ERROR: libusb_claim_interface (control) failed: %s\n", libusb_error_name(r));
        goto cleanup_and_exit;
    }

    // CDC-ACM line coding setup (baud rate, etc.)
    unsigned char line_coding[7] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 }; // 9600 baud, 8-N-1
    fprintf(stderr, "DEBUG: Calling libusb_control_transfer(SET_LINE_CODING) on interface %d...\n", ARDUINO_CONTROL_INTERFACE);
    r = libusb_control_transfer(handle, 0x21, 0x20, 0, ARDUINO_CONTROL_INTERFACE, line_coding, sizeof(line_coding), 100);
    if (r < 0) {
        fprintf(stderr, "WARN: libusb_control_transfer(SET_LINE_CODING) failed: %s\n", libusb_error_name(r));
    } else {
        fprintf(stderr, "DEBUG: libusb_control_transfer(SET_LINE_CODING) successful (sent %d bytes).\n", r);
    }

    // Set DTR (Data Terminal Ready) and RTS (Request to Send)
    fprintf(stderr, "DEBUG: Calling libusb_control_transfer(SET_CONTROL_LINE_STATE) on interface %d...\n", ARDUINO_CONTROL_INTERFACE);
    r = libusb_control_transfer(handle, 0x21, 0x22, 0x03, ARDUINO_CONTROL_INTERFACE, NULL, 0, 100);
    if (r < 0) {
        fprintf(stderr, "WARN: libusb_control_transfer(SET_CONTROL_LINE_STATE) failed: %s\n", libusb_error_name(r));
    } else {
        fprintf(stderr, "DEBUG: libusb_control_transfer(SET_CONTROL_LINE_STATE) successful.\n");
    }

    fprintf(stderr, "DEBUG: Claiming data interface %d...\n", ARDUINO_DATA_INTERFACE);
    r = libusb_claim_interface(handle, ARDUINO_DATA_INTERFACE);
    if (r < 0) {
        fprintf(stderr, "ERROR: libusb_claim_interface (data) failed: %s\n", libusb_error_name(r));
        goto cleanup_and_exit;
    }
    fprintf(stderr, "DEBUG: Successfully claimed data interface %d.\n", ARDUINO_DATA_INTERFACE);

    // Give device a moment
    usleep(50000); // 50ms should be enough

    unsigned char buffer[ARDUINO_MAX_PACKET_SIZE + 1];
    int actual_length;
    int timeout_errors = 0;

    fprintf(stderr, "DEBUG: Entering read loop...\n");
    while (1) {
        r = libusb_bulk_transfer(
            handle, ARDUINO_ENDPOINT_IN, buffer, ARDUINO_MAX_PACKET_SIZE, &actual_length, 2000
        );

        if (r == LIBUSB_SUCCESS) {
            timeout_errors = 0; // Reset counter on success
            if (actual_length > 0) {
                buffer[actual_length] = '\0';
                fprintf(stderr, "%s", buffer); // Print to stderr to bypass stdout buffering
            }
        } else if (r == LIBUSB_ERROR_TIMEOUT) {
            timeout_errors++;
            fprintf(stderr, ".\n"); // Print a dot for timeout
            if (timeout_errors >= 3) {
                fprintf(stderr, "ERROR: Exiting after 3 consecutive timeouts.\n");
                break;
            }
        } else {
            fprintf(stderr, "ERROR: libusb_bulk_transfer failed: %s\n", libusb_error_name(r));
            if (r == LIBUSB_ERROR_PIPE) {
                fprintf(stderr, "DEBUG: Pipe error detected. Clearing halt on endpoint %02x...\n", ARDUINO_ENDPOINT_IN);
                int rh = libusb_clear_halt(handle, ARDUINO_ENDPOINT_IN);
                if (rh == 0) {
                    fprintf(stderr, "DEBUG: Halt cleared successfully. Retrying transfer.\n");
                    continue;
                } else {
                    fprintf(stderr, "ERROR: Could not clear halt: %s\n", libusb_error_name(rh));
                }
            }
            if (r == LIBUSB_ERROR_NO_DEVICE) {
                fprintf(stderr, "ERROR: Device disconnected. Exiting loop.\n");
                break;
            }
            // Break on other errors too
            break;
        }
    }

cleanup_and_exit:
    fprintf(stderr, "\nDEBUG: Cleaning up and exiting...\n");
    libusb_release_interface(handle, ARDUINO_CONTROL_INTERFACE);
    libusb_release_interface(handle, ARDUINO_DATA_INTERFACE);

    if (kernel_driver_detached_control) {
        libusb_attach_kernel_driver(handle, ARDUINO_CONTROL_INTERFACE);
    }
    if (kernel_driver_detached_data) {
        libusb_attach_kernel_driver(handle, ARDUINO_DATA_INTERFACE);
    }

    fprintf(stderr, "DEBUG: Closing device handle.\n");
    libusb_close(handle);
    fprintf(stderr, "DEBUG: Exiting libusb.\n");
    libusb_exit(context);
    fprintf(stderr, "DEBUG: End of program.\n");
    return 0;
}
