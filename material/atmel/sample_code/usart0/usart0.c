#include <avr/io.h>
#include "usart0.h"

#define HI(x) (x>>8)
#define LO(x) (x&0xff)

void
usart0_init( uint16_t baud )
{
    DDRD |= _BV(1) ; // enable tx output
    /* UBRR0H = HI(baud); UBRR0L = LO(baud) */
    UBRR0 = baud;
    /* Asynchronous mode, no parity, 1-stop bit, 8-bit data */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00 );

    /* no 2x mode, no multi-processor mode */
    UCSR0A = 0x00;

    /* interrupts disabled, rx and tx enabled, 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}

void
usart0_put( uint8_t b )
{
    // wait for data register to be ready
	while ( (UCSR0A & _BV(UDRE0)) == 0 )
		;
	// load b for transmission
	UDR0 = b;
}
    
uint8_t
usart0_get( void )
{
	// poll for data available
	while ( (UCSR0A & _BV(RXC0)) == 0 )
		;
	return UDR0;
}
  
int16_t
usart0_get_delay( uint16_t max_delay )
{
	// poll for data available, with timeout
	while ( (UCSR0A & _BV(RXC0)) == 0  && max_delay != 0) {
		max_delay--;
	}
	if ( (UCSR0A & _BV(RXC0)) == 0  ) {
		return -1;
	}
	return UDR0;
}

int
main( void )
{
    CLKPR = _BV(CLKPCE); // enable clock scalar update
    CLKPR = 0x00;		// set to 8Mhz
    PORTC = 0xff;		// all off
    DDRC  = 0xff; 		// show byte in leds
    usart0_init( 51 );	// 9600 baud at 8MHz

    while ( 1 ) {
        uint8_t b = usart0_get();
		PORTC = ~b;
        if ( b >= 'A' && b <= 'Z' ) {
            b ^= 0x20; // convert to lower case
        }
		else if ( b >='a' && b <= 'z') {
			b &= 0xDF;	// convert to upper case
        }
		usart0_put( b );

		if ( b == '\r') {	// add a newline after return
			 b = '\n';
			 usart0_put( b );
		}
    }
    return 0;
}
