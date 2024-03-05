#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	// Setup LED output pin, driven by HIGH
	DDRD = 1 << PB6;

	while (1) {
		_delay_ms(500);

		// Flip the LED
		PORTD ^= 1 << PB6;
	}

	return 0;
}
