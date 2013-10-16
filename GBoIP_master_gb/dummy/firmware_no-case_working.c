// firmware.c -*-c-*-

#include "firmware.h"

// variablen initialisierung

volatile uint8_t i=0;
volatile uint8_t rx = 0;
volatile uint8_t tx = 0;
volatile uint8_t ser_in = 0;
volatile uint8_t ser_in_buffer = 0;
volatile uint8_t ser_out = 0;
volatile uint8_t ser_out_buffer = 0;
volatile uint8_t ser_send_counter = 0;
volatile uint8_t sync = 0;
volatile uint8_t sync_timer = 0;
volatile uint8_t bit_counter = 0;

int main(void) {

// zuweisung udp package
char *message;
message = &ser_in_buffer;

//message = &ser__buffer

// timer initialisierung
TCCR0=(1<<CS00);
TIMSK|=(1<<TOIE0);
TCNT0=0;

// external interrupt konfiguration
GICR = 1<<INT0;
MCUCR = 1<<ISC01;

// enable interrupts
sei();

// Eingaenge / Ausgaenge konfigurieren
DDRC = 0xff;				//PORTC - output / signal LED's
DDRD = 0b00010000;		//PD0 - 3 input, PD 4 output

//Netzwerk

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
			if (tx == 1){
			     	PORTC = ser_in_buffer; 			// signal LED's setzen                        
				send_udp(buffer, message, 1, 37351, serverip, 33750, gwmac);			// udp paket senden
				tx = 0;						//paket als gesendet markieren
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



		// Listen for UDP packets on port 57351 (0xe007) and process
		// received data.
		if (
			buffer[IP_PROTO_P] == IP_PROTO_UDP_V &&
			buffer[UDP_DST_PORT_H_P] == 0xe0 &&
			buffer[UDP_DST_PORT_L_P] == 0x07) {
	
			//PORTC = buffer[UDP_DATA_P];
			ser_out_buffer = buffer[UDP_DATA_P];

		// Die empfangenen Daten beginnen dann hier: buffer[UDP_DATA_P]

			}		



	}				 // Main loop end.


	return (0);
}

void arpresolverResultCallback(uint8_t *ip, uint8_t refnum, uint8_t *mac) {
	if (refnum == TransNumGwmac)
		memcpy(gwmac, mac, 6);
}

//external interrupt INT0 (falling edge clk signal detection)

ISR(INT0_vect)
{								//ISR INT0 start

//send data to local gameboy

if(ser_send_counter == 0){		//byte aus buffer dem zu versendenden byte zuweisen
ser_out = ser_out_buffer;		
}

//if (sync == 1){					// wenn synchron zu master clock, byte zum lokalen gameboy schicken
	if(ser_out& _BV(7)){			
		PORTD|=(1<<PORTD4);		// 1
	} else {
		PORTD&=~(1<<PORTD4);	// 0
	}
	ser_out = ser_out<<1;
	ser_send_counter++;
//}

if (ser_send_counter >= 8){
ser_send_counter = 0;
}

//receive data from local gameboy


if(bit_counter == 0){				// check if this is the first bit of a byte
ser_in = 0; 
}
	if(PIND& _BV(PD3)){				// read byte from local gameboy
		ser_in |= _BV(0);
	}

	if (bit_counter <= 6){
		ser_in = ser_in<<1;
	}	
bit_counter++;
	
if (bit_counter >= 8){
	ser_in_buffer = ser_in;
	ser_in = 0;
	bit_counter = 0;
	tx=1;

}








}							//ISR INT0 ende


// timer interrupt (timer == 8 == 128us)

ISR(TIMER0_OVF_vect)
{
sync_timer++;
}
