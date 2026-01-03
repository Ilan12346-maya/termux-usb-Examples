#ifndef MOUSE_DECODE_H
#define MOUSE_DECODE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Struct for a USB HID mouse report
typedef struct {
    uint8_t buttons;    // buttons bitmap (Bit 0=Left, Bit 1=Right, Bit 2=Middle)
    int8_t  x;          // delta X
    int8_t  y;          // delta Y
    int8_t  wheel;      // vertical scroll wheel
} MouseReport;

// Decode and print a single report
static inline MouseReport interpret_mouse_report(const uint8_t* data, size_t len) {
    MouseReport report = {0};

    if (len >= 7) { // Expect at least 7 bytes
        report.buttons = data[1];
        report.x = (int8_t)data[2];
        report.y = (int8_t)data[4];
        report.wheel = (int8_t)data[6];
    } else {
        fprintf(stderr, "Warning: Expected at least 7 bytes, but received %zu bytes for interpretation.\n", len);
    }
    
    return report;
}

#endif // MOUSE_DECODE_H