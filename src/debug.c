#include <debug.h>

void print_device_descriptor(const struct usb_device_descriptor *desc)
{
    printf("[+] USB Device Descriptor:\n");
    printf("  bLength:            0x%x\n", desc->bLength);
    printf("  bDescriptorType:    0x%x\n", desc->bDescriptorType);
    printf("  bcdUSB:             0x%x\n", desc->bcdUSB);
    printf("  bDeviceClass:       0x%x\n", desc->bDeviceClass);
    printf("  bDeviceSubClass:    0x%x\n", desc->bDeviceSubClass);
    printf("  bDeviceProtocal:    0x%x\n", desc->bDeviceProtocal);
    printf("  bMaxPacketSize0:    0x%x\n", desc->bMaxPacketSize0);
    printf("  idVendor:           0x%x\n", desc->idVendor);
    printf("  idProduct:          0x%x\n", desc->idProduct);
    printf("  bcdDevice:          0x%x\n", desc->bcdDevice);
    printf("  iManufacturer:      0x%x\n", desc->iManufacturer);
    printf("  iProduct:           0x%x\n", desc->iProduct);
    printf("  iSerialNumber:      0x%x\n", desc->iSerialNumber);
    printf("  bNumConfigurations: 0x%x\n", desc->bNumConfigurations);
}

void print_config_descriptor(const struct usb_config_descriptor *desc)
{
    printf("[+] USB Configuration Descriptor:\n");
    printf("  bLength:             0x%x\n", desc->bLength);
    printf("  bDescriptorType:     0x%x\n", desc->bDescriptorType);
    printf("  wTotalLength:        0x%x\n", desc->wTotalLength);
    printf("  bNumInterfaces:      0x%x\n", desc->bNumInterfaces);
    printf("  bConfigurationValue: 0x%x\n", desc->bConfigurationValue);
    printf("  iConfiguration:      0x%x\n", desc->iConfiguration);
    printf("  bmAttributes:        0x%x\n", desc->bmAttributes);
    printf("  bMaxPower:           0x%x\n", desc->bMaxPower);
}

void print_interface_descriptor(const struct usb_interface_descriptor *desc)
{
    printf("[+] USB Interface Descriptor:\n");
    printf("  bLength:            0x%x\n", desc->bLength);
    printf("  bDescriptorType:    0x%x\n", desc->bDescriptorType);
    printf("  bInterfaceNumber:   0x%x\n", desc->bInterfaceNumber);
    printf("  bAlternateSetting:  0x%x\n", desc->bAlternateSetting);
    printf("  bNumEndpoints:      0x%x\n", desc->bNumEndpoints);
    printf("  bInterfaceClass:    0x%x\n", desc->bInterfaceClass);
    printf("  bInterfaceSubClass: 0x%x\n", desc->bInterfaceSubClass);
    printf("  bInterfaceProtocol: 0x%x\n", desc->bInterfaceProtocol);
    printf("  iInterface:         0x%x\n", desc->iInterface);
}

void print_hid_descriptor(const struct usb_hid_descriptor *desc)
{
    printf("USB HID Descriptor:\n");
    printf("  bLength:                0x%x\n", desc->bLength);
    printf("  bDescriptorType:        0x%x\n", desc->bDescriptorType);
    printf("  bcdHID:                 0x%x\n", desc->bcdHID);
    printf("  bCountryCode:           0x%x\n", desc->bCountryCode);
    printf("  bNumDescriptors:        0x%x\n", desc->bNumDescriptors);
    printf("  bReportDescriptorType:  0x%x\n", desc->bReportDescriptorType);
    printf("  wReportDescriptorLength:0x%x\n", desc->wReportDescriptorLength);
}

void print_endpoint_descriptor(const struct usb_endpoint_descriptor *desc)
{
    printf("[+] USB Endpoint Descriptor:\n");
    printf("  bLength:          0x%x\n", desc->bLength);
    printf("  bDescriptorType:  0x%x\n", desc->bDescriptorType);
    printf("  bEndpointAddress: 0x%x\n", desc->bEndpointAddress);
    printf("  bmAttributes:     0x%x\n", desc->bmAttributes);
    printf("  wMaxPacketSize:   0x%x\n", desc->wMaxPacketSize);
    printf("  bInterval:        0x%x\n", desc->bInterval);
}
