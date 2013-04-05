#ifndef _usb_dev_h_
#define _usb_dev_h_

#include "usb_mem.h"
#include "usb_desc.h"

#ifdef __cplusplus
extern "C" {
#endif

void usb_init(void);
void usb_isr(void);
usb_packet_t *usb_rx(uint32_t endpoint);
uint32_t usb_rx_byte_count(uint32_t endpoint);
uint32_t usb_tx_byte_count(uint32_t endpoint);
uint32_t usb_tx_packet_count(uint32_t endpoint);
void usb_tx(uint32_t endpoint, usb_packet_t *packet);
void usb_tx_isr(uint32_t endpoint, usb_packet_t *packet);

extern volatile uint8_t usb_configuration;

#if defined(CDC_RX_ENDPOINT) && defined(CDC_TX_ENDPOINT)
extern uint8_t usb_cdc_line_coding[7];
extern uint8_t usb_cdc_line_rtsdtr;
extern volatile uint8_t usb_cdc_transmit_flush_timer;
#endif

#ifdef __cplusplus
}
#endif



#endif
