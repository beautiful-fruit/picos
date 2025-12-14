#include <libc.h>
#include <usb.h>

void print_device_descriptor(const struct usb_device_descriptor *desc);

void print_config_descriptor(const struct usb_config_descriptor *desc);

void print_interface_descriptor(const struct usb_interface_descriptor *desc);

void print_hid_descriptor(const struct usb_hid_descriptor *desc);

void print_endpoint_descriptor(const struct usb_endpoint_descriptor *desc);
