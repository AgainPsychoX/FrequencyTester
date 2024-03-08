#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

inline int is_button_pressed()
{
	return PIND & (1 << PD7);
}

int main(void)
{
	// Setup LED output pin, driven by HIGH
	DDRD = 1 << PB6;

	// Pull-up for button input
	PORTD |= 1 << PD7;

	lcd_init();
	lcd_print_p(PSTR("hello"));

	while (1) {
		if (is_button_pressed()) {
			PORTD &= ~(1 << PB6);
		}
		else {
			PORTD |= 1 << PB6;
		}
	}

	return 0;
}
