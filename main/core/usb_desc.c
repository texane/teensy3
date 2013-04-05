#include "usb_desc.h"




// 0x00 = not used
// 0x19 = Recieve only
// 0x15 = Transmit only
// 0x1D = Transmit & Recieve
// 
const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS] = 
{
	0x00, 0x15, 0x19, 0x15, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};



// These descriptors must NOT be "const", because the USB DMA
// has trouble accessing flash memory with enough bandwidth
// while the processor is executing from flash.

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)

static uint8_t device_descriptor[] = {
        18,                                     // bLength
        1,                                      // bDescriptorType
        0x00, 0x02,                             // bcdUSB
        2,                                      // bDeviceClass
        0,                                      // bDeviceSubClass
        0,                                      // bDeviceProtocol
        EP0_SIZE,                               // bMaxPacketSize0
        LSB(VENDOR_ID), MSB(VENDOR_ID),         // idVendor
        LSB(PRODUCT_ID), MSB(PRODUCT_ID),       // idProduct
        0x00, 0x01,                             // bcdDevice
        1,                                      // iManufacturer
        2,                                      // iProduct
        3,                                      // iSerialNumber
        1                                       // bNumConfigurations
};

//#define CONFIG1_DESC_SIZE 9+9
#define CONFIG1_DESC_SIZE (9+9+5+5+4+5+7+9+7+7)
static uint8_t config_descriptor[] = {
        // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
        9,                                      // bLength;
        2,                                      // bDescriptorType;
        LSB(CONFIG1_DESC_SIZE),                 // wTotalLength
        MSB(CONFIG1_DESC_SIZE),
        2,                                      // bNumInterfaces
        1,                                      // bConfigurationValue
        0,                                      // iConfiguration
        0xC0,                                   // bmAttributes
        50,                                     // bMaxPower
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        0,                                      // bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        0,                                      // bMasterInterface
        1,                                      // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_ACM_ENDPOINT | 0x80,                // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        64,                                     // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        1,                                      // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_RX_SIZE, 0,                         // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_TX_SIZE, 0,                         // wMaxPacketSize
        0                                       // bInterval
};

//#if sizeof(wchar_t) != 2
//#error "wchar_t is not 2 bytes, please compile with -fshort-wchar"
//#endif

struct usb_string_descriptor_struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        //wchar_t wString[];
        uint16_t wString[];
};

static struct usb_string_descriptor_struct string0 = {
        4,
        3,
        {0x0409}
};

//static uint8_t string0[] = { 4, 3, 9, 4 };

static struct usb_string_descriptor_struct string1 = {
        24,
        3,
        {'T','e','e','n','s','y','d','u','i','n','o'}
};
static struct usb_string_descriptor_struct string2 = {
	22,
        3,
        {'U','S','B',' ','S','e','r','i','a','l'}
};
static struct usb_string_descriptor_struct string3 = {
        12,
        3,
        {'1','2','3','4','5'}
};


const usb_descriptor_list_t usb_descriptor_list[] = {
	//wValue, wIndex, address,          length
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
        {0x0300, 0x0000, (const uint8_t *)&string0, 4},
        {0x0301, 0x0409, (const uint8_t *)&string1, 24},
        {0x0302, 0x0409, (const uint8_t *)&string2, 22},
        {0x0303, 0x0409, (const uint8_t *)&string3, 12},
	{0, 0, NULL, 0}
};



