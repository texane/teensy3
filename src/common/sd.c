#include <stdint.h>
#include "mk20dx128.h"
#include "spi.c"


/* sdcard */


/* debugging */

#define CONFIG_SD_DEBUG 1
#if CONFIG_SD_DEBUG
#define XSTR(__s) STR(__s)
#define STR(__s) #__s
#define PRINT_FAIL() serial_write_string(XSTR(__LINE__) ": fail\r\n")
#define PRINT_PASS() serial_write_string(XSTR(__LINE__) ": pass\r\n")
#define PRINT_BUF(__p, __n)				\
do {							\
  serial_write_string(XSTR(__LINE__) ": buf\r\n");	\
  serial_write_hex(__p, __n);				\
  serial_write_string("\r\n");				\
} while (0)
#else
#define PRINT_FAIL()
#define PRINT_PASS()
#define PRINT_BUF(__p, __n)
#endif /* CONFIG_SD_DEBUG */


/* some notes from specs */
/* host is the master, can communicate in ptop or broadcast. */
/* only supports sd card (not sdxc, sdhc ...) */
/* the data line must be high by the pullup */

#define SD_RESP_SIZE 5
static uint8_t sd_resp_buf[SD_RESP_SIZE];

#define SD_BLOCK_SIZE 512
static uint8_t sd_block_buf[SD_BLOCK_SIZE];

#define SD_INFO_V2 (1 << 0)
#define SD_INFO_SDHC (1 << 1)
#define SD_INFO_WP (1 << 2)
static uint8_t sd_info = 0;

static inline void sd_ss_high(void)
{
  /* PTD0 */
  GPIOD_PDOR |= 1 << 0;
}

static inline void sd_ss_low(void)
{
  /* PTD0 */
  GPIOD_PDOR &= ~(1 << 0);
}

static inline void sd_wait_not_busy(void)
{
  /* TODO: timeout */
  while (spi_read_uint8() != 0xff) ;
}

static void sd_write_cmd
(uint8_t op, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t crc)
{
  /* select the slave */
  sd_ss_high();

  sd_ss_low();

  /* wont work otherwise */
  sd_wait_not_busy();

  /* write the 6 bytes command */

  /* start bit, command index */
  spi_write_uint8((1 << 6) | op);

  /* argument */
  spi_write_uint8(a);
  spi_write_uint8(b);
  spi_write_uint8(c);
  spi_write_uint8(d);

  /* crc, stop bit */
  spi_write_uint8((crc << 1) | 1);
}

static inline void sd_write_cmd_32(uint8_t op, uint32_t x, uint8_t crc)
{
  const uint8_t a = (x >> 24) & 0xff;
  const uint8_t b = (x >> 16) & 0xff;
  const uint8_t c = (x >>  8) & 0xff;
  const uint8_t d = (x >>  0) & 0xff;

  sd_write_cmd(op, a, b, c, d, crc);
}

static int sd_read_r1(void)
{
  uint16_t i;

  /* read reply into sd_resp_buf, r1 format */
  for (i = 0; i < 10000; ++i)
  {
    sd_resp_buf[0] = spi_read_uint8();
    /* 0xff means no data transfered (test msb 0) */
    if ((sd_resp_buf[0] & 0x80) == 0) return 0;
  }

  return -1;
}

static int sd_read_r1b(void)
{
  if (sd_read_r1()) return -1;
  /* busy signal, wait for non 0 value (card ready) */
  while (spi_read_uint8() == 0) ;
  return 0;
}

static int sd_read_r2(void)
{
  if (sd_read_r1()) return -1;
  if (sd_resp_buf[0] & (1 << 2)) return 0;
  sd_resp_buf[1] = spi_read_uint8();
  return 0;
}

static int sd_read_r3(void)
{
  if (sd_read_r1()) return -1;
  /* illegal command, dont read remaining bytes */
  if (sd_resp_buf[0] & (1 << 2)) return 0;
  spi_read(sd_resp_buf + 1, 4);
  return 0;
}

static inline int sd_read_r7(void)
{
  /* same length as r3 */
  return sd_read_r3();
}

__attribute__((unused)) static int sd_read_csd(void)
{
  /* in spi mode, the card responds with a response
     token followed by a data block of 16 bytes and
     a 2 bytes crc.
   */

  /* TODO: redundant with sd_read_block */

  /* read application card specific data */
  sd_write_cmd(0x09, 0x00, 0x00, 0x00, 0x00, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) return -1;

  /* read response token */
  while (spi_read_uint8() != 0xfe) ;

  /* read the data block */
  spi_read_512(sd_block_buf);

  /* skip 2 bytes crc */
  spi_read_uint8();
  spi_read_uint8();

  return 0;
}

__attribute__((unused)) static void sd_print_csd(void)
{
  struct csd_v1
  {
    /* byte 0 */
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    /* byte 1 */
    uint8_t taac;
    /* byte 2 */
    uint8_t nsac;
    /* byte 3 */
    uint8_t tran_speed;
    /* byte 4 */
    uint8_t ccc_high;
    /* byte 5 */
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    /* byte 6 */
    unsigned c_size_high : 2;
    unsigned reserved2 : 2;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    /* byte 7 */
    uint8_t c_size_mid;
    /* byte 8 */
    unsigned vdd_r_curr_max : 3;
    unsigned vdd_r_curr_min : 3;
    unsigned c_size_low :2;
    /* byte 9 */
    unsigned c_size_mult_high : 2;
    unsigned vdd_w_cur_max : 3;
    unsigned vdd_w_curr_min : 3;
    /* byte 10 */
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned c_size_mult_low : 1;
    /* byte 11 */
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    /* byte 12 */
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved3 : 2;
    unsigned wp_grp_enable : 1;
    /* byte 13 */
    unsigned reserved4 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    /* byte 14 */
    unsigned reserved5: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    /* byte 15 */
    unsigned always1 : 1;
    unsigned crc : 7;
  } __attribute__((packed)) *csd = (struct csd_v1*)sd_block_buf;

  uint8_t x;

  serial_write_string("csd: ");
  serial_write_hex(sd_block_buf, 16);
  serial_write_string("\r\n");

  serial_write_string("vers: ");
  x = csd->csd_ver;
  serial_write_hex(&x, 1);
  serial_write_string("\r\n");

  x = csd->read_bl_len;
  serial_write_string("bl_len: ");
  serial_write_hex(&x, 1);
  serial_write_string("\r\n");

  serial_write_string("csize: ");
  x = csd->c_size_high;
  serial_write_hex(&x, 1);
  x = csd->c_size_mid;
  serial_write_hex(&x, 1);
  x = csd->c_size_low;
  serial_write_hex(&x, 1);
  serial_write_string("\r\n");
}

__attribute__((unused))
static int sd_write_csd(void)
{
  sd_write_cmd(0x1c, 0x00, 0x00, 0x00, 0x00, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) return -1;
  return 0;
}

static inline uint32_t sd_bid_to_baddr(uint32_t bid)
{
  return ((sd_info & SD_INFO_SDHC) == 0) ? bid *= 512 : bid;
}

__attribute__((unused))
static int sd_read_block(uint32_t bid)
{
  /* block at addr is read in sd_block_buf */

  const uint32_t baddr = sd_bid_to_baddr(bid);

  /* cmd17, read single block */
  sd_write_cmd_32(0x11, baddr, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }

  /* response token */
  while (spi_read_uint8() != 0xfe) ;

  /* data block */
  spi_read_512(sd_block_buf);

  /* skip 2 bytes crc */
  spi_read_uint8();
  spi_read_uint8();

  return 0;
}

__attribute__((unused))
static int sd_erase(uint32_t bid, uint32_t n)
{
  /* bid the index of the first block to erase */
  /* n the block count */

  uint32_t baddr;

  /* cmd32, erase_wr_blk_start_addr */
  baddr = sd_bid_to_baddr(bid);
  sd_write_cmd_32(0x20, baddr, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }

  /* cmd33, erase_wr_blk_start_addr */
  baddr = sd_bid_to_baddr(bid + n - 1);
  sd_write_cmd_32(0x21, baddr, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }

  /* cmd38, erase */
  sd_write_cmd(0x26, 0x00, 0x00, 0x00, 0x00, 0xff);
  if (sd_read_r1b() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }

  /* TODO: check status */

  return 0;
}

__attribute__((unused))
static int sd_write_block(uint32_t bid)
{
  /* sd_block_buf is written into block at addr */

  const uint32_t baddr = sd_bid_to_baddr(bid);

  /* cmd24, write_block */
  sd_write_cmd_32(0x18, baddr, 0xff);
  if (sd_read_r1() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }

  /* start block token */
  spi_write_uint8(0xfe);

  /* data block */
  spi_write_512(sd_block_buf);

  /* dummy crc */
  spi_write_uint8(0xff);
  spi_write_uint8(0xff);

  /* data response */
  if ((spi_read_uint8() & 0x1f) != 0x05) { PRINT_FAIL(); return -1; }

  /* busy signal */
  while (spi_read_uint8() == 0x00) ;

  /* cmd13, send_status */
  sd_write_cmd(0x0d, 0x00, 0x00, 0x00, 0x00, 0xff);
  if (sd_read_r2() || sd_resp_buf[0] || sd_resp_buf[1]) 
  {
    PRINT_FAIL();
    return -1;
  }

  return 0;
}

static int sd_setup(void)
{
  /* sd initialization sequence */

  unsigned int i;
  uint8_t arg;

  spi_setup_master();

  /* send at least 74 warmup pulses. cs must be high. */
  sd_ss_high();
  for (i = 0; i < 10; ++i) spi_write_uint8(0xff);

  /* enter spi mode */
  /* cs low, send cmd0 (reset), check error and idle state */
  for (i = 0; i < 0xff; ++i)
  {
    sd_write_cmd(0x00, 0x00, 0x00, 0x00, 0x00, 0x4a);
    if (sd_read_r1() != -1)
    {
      /* wait for in_idle_state == 1 */
      if (sd_resp_buf[0] & (1 << 0)) break ;
    }
  }
  if (i == 0xff) { PRINT_FAIL(); return -1; }

  PRINT_BUF(sd_resp_buf, 5);

  /* cmd8 (send_if_cond) */
  sd_write_cmd(0x08, 0x00, 0x00, 0x01, 0xaa, 0x43);
  /* card echos back voltage and check pattern */
  if (sd_read_r7()) { PRINT_FAIL(); return -1; }

  PRINT_BUF(sd_resp_buf, 5);

  /* illegal command */
  if (sd_resp_buf[0] & (1 << 2))
  {
    /* sd version 1 */
    /* check for other errors */
    /* if (sd_resp_buf[0] & ~(1 << 2)) { PRINT_FAIL(); return -1; } */
  }
  else
  {
    /* sd version 2 */
    /* check errors, pattern, voltage (2.7v-3.6v) */
    sd_info |= SD_INFO_V2;
    if (sd_resp_buf[0] & 0xfe) { PRINT_FAIL(); return -1; }
    if (sd_resp_buf[4] != 0xaa) { PRINT_FAIL(); return -1; }
    if ((sd_resp_buf[3] & 0xf) != 0x01) { PRINT_FAIL(); return -1; }
  }

#if 0 /* fixme: optionnal, but should work */
  /* cmd58 (read ocr operation condition register) for voltages */
  sd_write_cmd(0x3a, 0x00, 0x00, 0x00, 0x00, 0xff);
  if (sd_read_r3()) { PRINT_FAIL(); return -1; }
  /* accept 3.3v - 3.6v */
  PRINT_BUF(sd_resp_buf, 5);
  if ((sd_resp_buf[3] >> 4) == 0) { PRINT_FAIL(); return -1; }
#endif

  PRINT_PASS();

  /* acmd41, wait for in_idle_state == 0 */

  /* enable sdhc is v2 */
  arg = (sd_info & SD_INFO_V2) ? (1 << 6) : 0;

  while (1)
  {
    /* acmd commands are preceded by cmd55 */
    sd_write_cmd(0x37, 0x00, 0x00, 0x00, 0x00, 0xff);
    if (sd_read_r1()) { PRINT_FAIL(); return -1; }

    sd_write_cmd(0x29, arg, 0x00, 0x00, 0x00, 0xff);
    if (sd_read_r1()) { PRINT_FAIL(); return -1; }

    /* wait for in_idle_state == 0 */
    if ((sd_resp_buf[0] & (1 << 0)) == 0) break ;
  }

  PRINT_PASS();

  /* get sdhc status */
  if (sd_info & SD_INFO_V2)
  {
    /* cmd58 (get ccs) */
    sd_write_cmd(0x3a, 0x00, 0x00, 0x00, 0x00, 0xff);
    if (sd_read_r3()) { PRINT_FAIL(); return -1; }
    if ((sd_resp_buf[1] & 0xc0) == 0xc0) sd_info |= SD_INFO_SDHC;
  }

  /* initialization sequence done, data transfer mode. */

  /* set block length to 512 if not sdhc */
  if ((sd_info & SD_INFO_SDHC) == 0)
  {
    sd_write_cmd(0x10, 0x00, 0x00, 0x02, 0x00, 0xff);
    if (sd_read_r1() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }
  }

#if 0 /* unused */
  /* TODO: cache card infos */
  if (sd_read_csd()) { PRINT_FAIL(); return -1; }
  sd_print_csd();

  /* TODO: disable wp, if supported and is_ronly == 0 */
  if ((sd_info & SD_INFO_WP) && (is_ronly == 0))
  {
    /* cmd29, clear_write_prot */
    sd_write_cmd(0x1d, 0x00, 0x00, 0x00, 0x00, 0xff);
    if (sd_read_r1b() || sd_resp_buf[0]) { PRINT_FAIL(); return -1; }
  }
#endif /* unused */

#if 0
  sd_set_freq();
#endif

  return 0;
}
