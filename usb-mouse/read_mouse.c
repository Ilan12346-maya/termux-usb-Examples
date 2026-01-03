#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <stdint.h>

#include "mouse_decode.h"

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define POLLING_RATE_HZ 30
#define POLLING_INTERVAL_US (1000000 / POLLING_RATE_HZ)

// UI and mouse state
char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
int mouse_x = SCREEN_WIDTH / 2;
int mouse_y = SCREEN_HEIGHT / 2;
uint8_t mouse_buttons = 0;
int8_t mouse_wheel = 0;

int prev_mouse_x = -1;
int prev_mouse_y = -1;
uint8_t prev_mouse_buttons = -1;
int8_t prev_mouse_wheel = 0;




void draw_ui() {
    fprintf(stderr, "\033[u"); // Restore cursor to saved position

    // Determine cursor string
    const char* cursor_str;
    if (mouse_buttons & 0x01) { // Left button
        cursor_str = "L";
    } else if (mouse_buttons & 0x02) { // Right button
        cursor_str = "R";
    } else if (mouse_buttons & 0x04) { // Middle button
        cursor_str = "M";
    } else {
        cursor_str = "⇖"; // Default cursor
    }

    // Prepare screen buffer (fills with spaces, cursor will be printed directly)
    memset(screen, ' ', sizeof(screen));

    // Print screen buffer with border
    fprintf(stderr, "╭");
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        fprintf(stderr, "─");
    }
    fprintf(stderr, "╮\n");

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        fprintf(stderr, "│");
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (y == mouse_y && x == mouse_x) {
                fprintf(stderr, "%s", cursor_str); // Print the cursor string
            } else {
                fputc(screen[y][x], stderr); // Print space from buffer
            }
        }
        fprintf(stderr, "│\n");
    }

    fprintf(stderr, "╰");
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        fprintf(stderr, "─");
    }
    fprintf(stderr, "╯\n");

    // Print status
    const char* wheel_status_str = "---";
    if (mouse_wheel < 0) {
        wheel_status_str = "Down ";
    } else if (mouse_wheel > 0) {
        wheel_status_str = " Up ";
    }

    fprintf(stderr, "  Buttons: L=%d R=%d M=%d | Wheel: %s \n",
           (mouse_buttons & 0x01) ? 1 : 0,
           (mouse_buttons >> 1) & 0x01 ? 1 : 0,
           (mouse_buttons >> 2) & 0x01 ? 1 : 0,
           wheel_status_str);


    fflush(stderr);
}


int main(int argc, char **argv) {
    libusb_context *context = NULL;
    libusb_device_handle *handle = NULL;
    int fd = -1;
    int r;

    if (argc < 2 || sscanf(argv[1], "%d", &fd) != 1) {
        fprintf(stderr, "Usage: %s <file_descriptor>\n", argv[0]);
        return 1;
    }

    fprintf(stderr, "\033[?25l"); // Hide cursor
    fprintf(stderr, "\033[s"); // Save cursor position

    libusb_set_option(NULL, LIBUSB_OPTION_NO_DEVICE_DISCOVERY);
    r = libusb_init(&context);
    if (r < 0) {
        fprintf(stderr, "libusb_init failed: %s\n", libusb_error_name(r));
        goto cleanup_cursor;
    }

    r = libusb_wrap_sys_device(context, (intptr_t)fd, &handle);
    if (r < 0) {
        fprintf(stderr, "libusb_wrap_sys_device failed: %s\n", libusb_error_name(r));
        libusb_exit(context);
        goto cleanup_cursor;
    }

    int interface_number = 1; // From original working file
    int kernel_driver_active = 0;
    if (libusb_kernel_driver_active(handle, interface_number) == 1) {
        r = libusb_detach_kernel_driver(handle, interface_number);
        if (r < 0) {
            fprintf(stderr, "libusb_detach_kernel_driver failed: %s\n", libusb_error_name(r));
            goto cleanup_libusb;
        }
        kernel_driver_active = 1;
    }

    r = libusb_claim_interface(handle, interface_number);
    if (r < 0) {
        fprintf(stderr, "libusb_claim_interface failed: %s\n", libusb_error_name(r));
        goto cleanup_driver;
    }

    int endpoint_address = 0x82; // From original working file
    unsigned char data[8];
    int actual_length;

    draw_ui(); // Initial draw

    while (1) {
        r = libusb_interrupt_transfer(handle, endpoint_address, data, sizeof(data), &actual_length, 34); // ~30Hz timeout

        if (r == 0 && actual_length > 0) {
            MouseReport report = interpret_mouse_report(data, actual_length);

            mouse_buttons = report.buttons;
            mouse_x += report.x;
            mouse_y += report.y;
            mouse_wheel = report.wheel;

            // Clamp coordinates
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;
        } else if (r != LIBUSB_ERROR_TIMEOUT) {
            fprintf(stderr, "\nlibusb_interrupt_transfer error: %s\n", libusb_error_name(r));
            break;
        }

        if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y || mouse_buttons != prev_mouse_buttons || mouse_wheel != prev_mouse_wheel) {
            draw_ui();
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
            prev_mouse_buttons = mouse_buttons;
            prev_mouse_wheel = mouse_wheel;
        }


    }

    libusb_release_interface(handle, interface_number);
cleanup_driver:
    if (kernel_driver_active) {
        libusb_attach_kernel_driver(handle, interface_number);
    }
cleanup_libusb:
    libusb_close(handle);
    libusb_exit(context);
cleanup_cursor:
    fprintf(stderr, "\n"); // Move to a new line to not overwrite the UI
    fprintf(stderr, "\033[?25h"); // Show cursor again
    return r;
}