// config.h -*-c-*-


#ifndef CONFIG_h
#define CONFIG_h


#define BufferSize 500


// Used by net/enc28j60.c
#define ENC28J60_CONTROL_PORT PORTB
#define ENC28J60_CONTROL_DDR  DDRB
#define ENC28J60_CONTROL_CS   PORTB2
#define ENC28J60_CONTROL_SO   PORTB4
#define ENC28J60_CONTROL_SI   PORTB3
#define ENC28J60_CONTROL_SCK  PORTB5


static uint8_t mymac[6]    = { 0x00, 0x09, 0xbf, 0x01, 0x02, 0x02 };
static uint8_t myip[4]     = { 192, 168, 8, 4 };

static uint8_t gwmac[6];
static uint8_t gwip[4];

static uint8_t serverip[4] = { 192, 168, 8, 3 };
static uint8_t netmask[4]  = { 255, 255, 255, 0 };


#endif
