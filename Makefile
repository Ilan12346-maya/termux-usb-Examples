CC = gcc
CFLAGS = -Wall -Wextra -g

TARGETS = get_device_descriptors read_gamepad_raw usb_info

all: $(TARGETS)

get_device_descriptors: get_device_descriptors.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

read_gamepad_raw: read_gamepad_raw.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

usb_info: usb_info.c
	$(CC) $(CFLAGS) -o $@ $< -lusb-1.0

clean:
	rm -f $(TARGETS) *.o