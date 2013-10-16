// firmware.c -*-c-*-

#include "firmware.h"

// variablen initialisierung

volatile uint8_t clk=0;
volatile uint8_t i=0;
volatile uint8_t rx = 1;
volatile uint8_t tx = 0;
volatile uint8_t bit_counter = 0;

volatile uint8_t ser_in = 0;
volatile uint8_t ser_in_buffer = 0;
volatile uint8_t ser_out = 0;
volatile uint8_t ser_out_buffer = 0;
volatile uint8_t ser_send_counter = 0;

int main(void) {

// zuweisung udp package
char *message;
message = &ser_in_buffer;


// timer initialisierung
TCCR0=(1<<CS00);
TIMSK|=(1<<TOIE0);
TCNT0=0;

// external interrupt konfiguration
//GICR = 1<<INT0;
//MCUCR = 1<<ISC01;

// enable interrupts
sei();

// configuring I/O pins
DDRC = 0xff;			// PORTC - status LED's
DDRD = 0b00010100;		// PD2 - outputs the clock signal, PD 3 - reads data from local gameboy, PD4 - writes data to local gameboy

PORTD|=(1<<PORTD2);		// clock signal is high when no data is being transmitted
PORTD&=~(1<<PORTD4);		// set data line from GBoIP to gameboy low 

// setup ethernet interface

	static uint8_t buffer[BufferSize + 1];
	int8_t   i;
	uint16_t received;

	// Initialise network interface.
	enc28j60Init(mymac);
	_delay_ms(100);
	// Magjack leds configuration, see enc28j60 datasheet, page 11
	// LEDB=yellow LEDA=green
	// 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	// enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	enc28j60PhyWrite(PHLCON, 0x476);
	_delay_ms(100);

	init_mac(mymac);
	client_ifconfig(myip, netmask);

	// Resolve MAC address from server IP.
	if (route_via_gw(serverip)) // Must be routed via gateway.
		get_mac_with_arp(gwip, TransNumGwmac, &arpresolverResultCallback);
	else                        // Server is on local network.
		get_mac_with_arp(serverip, TransNumGwmac, &arpresolverResultCallback);

	while (get_mac_with_arp_wait()) {
		received = enc28j60PacketReceive(BufferSize, buffer);
		// Call packetloop to process ARP reply.
		packetloop_arp_icmp_tcp(buffer, received);
	}


	while (1) { 		// Main loop start.

		received = enc28j60PacketReceive(BufferSize, buffer);

		// Do something while no packet in queue.
		if (received == 0) {
			if (tx == 1){									// check if a package is ready to be transmitted
			     	PORTC = ser_in_buffer; 							// set status LED's to the value received from the local gameboy                         
				send_udp(buffer, message, 1, 37351, serverip, 57351, gwmac);		// transfer UDP package to the master
				tx = 0;									// package has been transferred. ready for new data
			}
			continue;
		}

		// Answer to ARP requests.
		if (eth_type_is_arp_and_my_ip(buffer, received)) {
			make_arp_answer_from_request(buffer, received);
			continue;
		}

		// Check if IP packets (ICMP or UDP) are for us.
		if (eth_type_is_ip_and_my_ip(buffer, received) == 0)
			continue;

		// Answer ping with pong.
		if (
			buffer[IP_PROTO_P]  == IP_PROTO_ICMP_V &&
			buffer[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V) {

			make_echo_reply_from_request(buffer, received);
			continue;
		}


		/* receive UDP packets */

		// Listen for UDP packets on port 33750 (0xe007) and process received data.
		if (
			buffer[IP_PROTO_P] == IP_PROTO_UDP_V &&
			buffer[UDP_DST_PORT_H_P] == 0x83 &&
			buffer[UDP_DST_PORT_L_P] == 0xD6) {

			if (rx == 1){					// check if the program is ready to receive a new byte
				ser_out_buffer = buffer[UDP_DATA_P];	// write new byte to ser_out_buffer
				rx = 0;					// byte received. ready to be processed.
			}
		}

	}				 // Main loop end.


	return (0);
}

void arpresolverResultCallback(uint8_t *ip, uint8_t refnum, uint8_t *mac) {
	if (refnum == TransNumGwmac)
		memcpy(gwmac, mac, 6);
}



ISR(TIMER0_OVF_vect)
{

if (rx == 0){				// if UDP package received

if (bit_counter == 0){			// check if this is the first bit of a byte
	ser_out = ser_out_buffer;
}

if (clk <= 2){				// clock signal generator - low half
	PORTD&=~(1<<PORTD2);		// set clock signal to 0
	if(ser_out& _BV(7)){		// send serial data to local gameboy	
		PORTD|=(1<<PORTD4);	// 1
	} else {
		PORTD&=~(1<<PORTD4);	// 0
	}

	if(PIND& _BV(PD3)){		//read serial data from local gamebot
		ser_in |= _BV(0);
	}
}

if (clk >= 3){				// clock signal generator - high half
	PORTD|=(1<<PORTD2);		// set clock signal to 1
}

if (clk >= 4){				// bit complete
clk=0;					// set clock signal timer to 0 

if (bit_counter <= 6){			// only shift the contents of the variable if it's not the last bit
	ser_out = ser_out<<1;		// shift content of variables
	ser_in = ser_in<<1;
}

bit_counter++;				// count number of bits that have been processed

if(bit_counter >= 8){			// check if a byte has been completely processed
	bit_counter = 0;		// byte complete. set bitcounter to 0
        ser_in_buffer = ser_in;         // writing ser_in to ser_in_buffer for transfer to master gameboy        
	ser_in = 0;	
	tx = 1;				// byte is ready to be transmitted
	rx = 1;				// UDP package has been transmitted. ready to receive new package
}
}

clk++;					// increment clock timer

}

}					// interrupt routine end
