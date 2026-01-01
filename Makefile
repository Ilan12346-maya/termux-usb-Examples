CC = gcc
CFLAGS = -Wall -Wextra -g

TARGETS = util/get_device_descriptors usb-gamepad/read_gamepad_raw util/usb_info usb-serial/read_serial usb-gamepad/read_gamepad

all: $(TARGETS)

util/get_device_descriptors: util/get_device_descriptors.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

usb-gamepad/read_gamepad_raw: usb-gamepad/read_gamepad_raw.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

util/usb_info: util/usb_info.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

usb-serial/read_serial: usb-serial/read_serial.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

usb-gamepad/read_gamepad: usb-gamepad/read_gamepad.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

clean:
	rm -f $(TARGETS) *.o
