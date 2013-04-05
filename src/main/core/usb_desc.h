#ifndef _usb_desc_h_
#define _usb_desc_h_

#include <stdint.h>
#include <stddef.h>

#define EP0_SIZE 16
#define VENDOR_ID      0x16C0
#define PRODUCT_ID     0x0483

#define CDC_ACM_ENDPOINT        2
#define CDC_RX_ENDPOINT         3
#define CDC_TX_ENDPOINT         4
#define CDC_ACM_SIZE            16
#define CDC_RX_SIZE             64
#define CDC_TX_SIZE             64


// number of non-zero endpoints (0 to 15)
#define NUM_ENDPOINTS 15
extern const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS];

typedef struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint16_t	length;
} usb_descriptor_list_t;

extern const usb_descriptor_list_t usb_descriptor_list[];


#endif
