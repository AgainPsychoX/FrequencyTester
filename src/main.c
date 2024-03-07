#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
	// Setup LED output pin, driven by HIGH
	DDRD = 1 << PB6;

	lcd_init();
	lcd_print_p(PSTR("hello"));
	lcd_print("?!");
	_delay_ms(3000);

	while (1) {
		_delay_ms(1000);
		PORTD |= 1 << PB6;
		lcd_go_home();
		lcd_print_p(PSTR("HIGH!   "));

		_delay_ms(1000);
		PORTD &= ~(1 << PB6);
		lcd_go_home();
		lcd_print_p(PSTR("     low"));
	}

	return 0;
}
