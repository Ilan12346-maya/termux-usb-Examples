#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <unistd.h> // For close
#include <string.h> // For memset

// Function to get a string descriptor
static void print_string_descriptor(libusb_device_handle *handle, uint8_t index) {
    if (index == 0) {
        printf(" (No string descriptor)");
        return;
    }
    char s_desc[256];
    int r = libusb_get_string_descriptor_ascii(handle, index, (unsigned char*)s_desc, sizeof(s_desc));
    if (r > 0) {
        printf(" (%s)", s_desc);
    } else {
        printf(" (Error getting string descriptor %d: %s)", index, libusb_error_name(r));
    }
}

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

#define VENDOR_ID 0x045e // ZhiXu Controller Vendor ID
#define PRODUCT_ID 0x028e // ZhiXu Controller Product ID

int main(int argc, char **argv) {
    libusb_context *context = NULL;
    libusb_device_handle *handle = NULL;
    int fd = -1;
    int r = 0;

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

    libusb_device *device = libusb_get_device(handle);
    if (!device) {
        fprintf(stderr, "libusb_get_device failed.\n");
        libusb_close(handle);
        libusb_exit(context);
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_get_device successful.\n");

    struct libusb_device_descriptor dev_desc;
    r = libusb_get_device_descriptor(device, &dev_desc);
    if (r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor failed: %s\n", libusb_error_name(r));
        libusb_close(handle);
        libusb_exit(context);
        return 1;
    }
    fprintf(stderr, "DEBUG: libusb_get_device_descriptor successful.\n");

    printf("== Device Descriptor ==\n");
    printf("  bLength: %d\n", dev_desc.bLength);
    printf("  bDescriptorType: %d (Device)\n", dev_desc.bDescriptorType);
    printf("  bcdUSB: %04x\n", dev_desc.bcdUSB);
    printf("  bDeviceClass: %02x\n", dev_desc.bDeviceClass);
    printf("  bDeviceSubClass: %02x\n", dev_desc.bDeviceSubClass);
    printf("  bDeviceProtocol: %02x\n", dev_desc.bDeviceProtocol);
    printf("  bMaxPacketSize0: %d\n", dev_desc.bMaxPacketSize0);
    printf("  idVendor: %04x\n", dev_desc.idVendor);
    printf("  idProduct: %04x\n", dev_desc.idProduct);
    printf("  bcdDevice: %04x\n", dev_desc.bcdDevice);
    printf("  iManufacturer: %d", dev_desc.iManufacturer);
    print_string_descriptor(handle, dev_desc.iManufacturer);
    printf("\n");
    printf("  iProduct: %d", dev_desc.iProduct);
    print_string_descriptor(handle, dev_desc.iProduct);
    printf("\n");
    printf("  iSerialNumber: %d", dev_desc.iSerialNumber);
    print_string_descriptor(handle, dev_desc.iSerialNumber);
    printf("\n");
    printf("  bNumConfigurations: %d\n", dev_desc.bNumConfigurations);

    for (int config_idx = 0; config_idx < dev_desc.bNumConfigurations; config_idx++) {
        struct libusb_config_descriptor *config_desc;
        r = libusb_get_config_descriptor(device, config_idx, &config_desc);
        if (r < 0) {
            fprintf(stderr, "Error getting config descriptor %d: %s\n", config_idx, libusb_error_name(r));
            continue;
        }

        printf("\n== Configuration Descriptor %d ==\n", config_idx);
        printf("  bLength: %d\n", config_desc->bLength);
        printf("  bDescriptorType: %d (Configuration)\n", config_desc->bDescriptorType);
        printf("  wTotalLength: %d\n", config_desc->wTotalLength);
        printf("  bNumInterfaces: %d\n", config_desc->bNumInterfaces);
        printf("  bConfigurationValue: %d\n", config_desc->bConfigurationValue);
        printf("  iConfiguration: %d", config_desc->iConfiguration);
        print_string_descriptor(handle, config_desc->iConfiguration);
        printf("\n");
        printf("  bmAttributes: %02x\n", config_desc->bmAttributes);
        // printf("  bMaxPower: %d mA\n", config_desc->bMaxPower * 2); // Commented out due to compilation error

        for (int if_idx = 0; if_idx < config_desc->bNumInterfaces; if_idx++) {
            const struct libusb_interface *interface = &config_desc->interface[if_idx];

            for (int alt_idx = 0; alt_idx < interface->num_altsetting; alt_idx++) {
                const struct libusb_interface_descriptor *if_desc = &interface->altsetting[alt_idx];

                printf("\n  == Interface Descriptor %d, AltSetting %d ==\n", if_idx, alt_idx);
                printf("    bLength: %d\n", if_desc->bLength);
                printf("    bDescriptorType: %d (Interface)\n", if_desc->bDescriptorType);
                printf("    bInterfaceNumber: %d\n", if_desc->bInterfaceNumber);
                printf("    bAlternateSetting: %d\n", if_desc->bAlternateSetting);
                printf("    bNumEndpoints: %d\n", if_desc->bNumEndpoints);
                printf("    bInterfaceClass: %02x", if_desc->bInterfaceClass);
                if (if_desc->bInterfaceClass == LIBUSB_CLASS_HID) printf(" (HID)");
                else if (if_desc->bInterfaceClass == 0xFF) printf(" (Vendor Specific)");
                printf("\n");
                printf("    bInterfaceSubClass: %02x\n", if_desc->bInterfaceSubClass);
                printf("    bInterfaceProtocol: %02x\n", if_desc->bInterfaceProtocol);
                printf("    iInterface: %d", if_desc->iInterface);
                print_string_descriptor(handle, if_desc->iInterface);
                printf("\n");

                // Check for HID descriptor
                if (if_desc->bInterfaceClass == LIBUSB_CLASS_HID || if_desc->bInterfaceClass == 0xFF) {
                    printf("    Attempting to read HID Report Descriptor...\n");
                    unsigned char hid_report_desc[512]; // Max 512 bytes for HID report descriptor
                    memset(hid_report_desc, 0, sizeof(hid_report_desc));
                    // The HID descriptor is usually embedded within the configuration descriptor data.
                    // libusb_get_descriptor with LIBUSB_DT_REPORT type targets the HID report descriptor.
                    // The wIndex for HID report descriptor is the interface number.
                    r = libusb_get_descriptor(handle, LIBUSB_DT_REPORT, if_desc->bInterfaceNumber, hid_report_desc, sizeof(hid_report_desc));

                    if (r > 0) {
                        printf("    HID Report Descriptor (%d bytes):\n", r);
                        printf("      ");
                        for (int i = 0; i < r; ++i) {
                            printf("%02x ", hid_report_desc[i]);
                            if ((i + 1) % 16 == 0) printf("\n      "); // 16 bytes per line
                        }
                        printf("\n");
                    } else if (r == LIBUSB_ERROR_NOT_FOUND) {
                        printf("    HID Report Descriptor not found for this interface.\n");
                    }
                    else {
                        printf("    Error reading HID Report Descriptor for interface %d: %s\n", if_desc->bInterfaceNumber, libusb_error_name(r));
                    }
                }


                for (int ep_idx = 0; ep_idx < if_desc->bNumEndpoints; ep_idx++) {
                    const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[ep_idx];

                    printf("\n    == Endpoint Descriptor %d ==\n", ep_idx);
                    printf("      bLength: %d\n", ep_desc->bLength);
                    printf("      bDescriptorType: %d (Endpoint)\n", ep_desc->bDescriptorType);
                    printf("      bEndpointAddress: %02x (%s, EP %d)\n",
                           ep_desc->bEndpointAddress,
                           (ep_desc->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN ? "IN" : "OUT",
                           ep_desc->bEndpointAddress & 0x0F);
                    printf("      bmAttributes: %02x (Transfer Type: %d - %s)\n",
                           ep_desc->bmAttributes,
                           ep_desc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK,
                           libusb_transfer_type_to_string(ep_desc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK));
                    printf("      wMaxPacketSize: %d\n", ep_desc->wMaxPacketSize);
                    printf("      bInterval: %d\n", ep_desc->bInterval);
                }
            }
        }
        libusb_free_config_descriptor(config_desc);
    }

    libusb_close(handle);
    libusb_exit(context);
    return 0;
}
