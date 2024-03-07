#include "lcd.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

// PORTB is being used for all the pins related to the LCD:
// 4-bit mode is used: PB3..0 = D7..4
#define RS_PIN	PB4
#define EN_PIN	PB5
// RW pin is grounded (always writing)

/// Pulses LCD enable pin for at least 1 microsecond
void lcd_pulse_enable()
{
	PORTB |= 1 << EN_PIN;
	_delay_us(1);
	PORTB &= ~(1 << EN_PIN);
			_delay_us(100);
}

/// Sends command in 4-bit mode to the LCD
void lcd_command_4(uint8_t x)
{
	PORTB = x >> 4;
	lcd_pulse_enable();
	PORTB = x & 0b1111;
	lcd_pulse_enable();
	_delay_us(40);
}

/// Sends data in 4-bit mode to the LCD
void lcd_data_4(uint8_t x)
{
	PORTB = (x >> 4) | (1 << RS_PIN);
	lcd_pulse_enable();
	PORTB = (x & 0b1111) | (1 << RS_PIN);
	lcd_pulse_enable();
	_delay_us(40);
}

/// Prints C-style string to the LCD
void lcd_print(const char* cstr)
{
	while (*cstr) {
		lcd_data_4(*cstr++);
	}
}

/// Prints text of given length to the LCD
void lcd_print_n(const char* cstr, size_t length)
{
	while (length--) {
		lcd_data_4(*cstr++);
	}
}

/// Prints C-style string from program memory to the LCD
void lcd_print_p(PGM_P cstr)
{
	while (1) {
		uint8_t x = pgm_read_byte(cstr++);
		if (!x) break;
		lcd_data_4(x);
	}
}

/// Prints text of given length from program memory to the LCD
void lcd_print_np(PGM_P cstr, size_t length)
{
	while (length--) {
		lcd_data_4(pgm_read_byte(cstr++));
	}
}

/// Sets address/cursor position to given offset.
void lcd_set_cursor(uint8_t offset)
{
	lcd_command_4(0b10000000 | offset);
}

/// Resets address/cursor position & shift to original position (address 0)
void lcd_go_home()
{
	lcd_command_4(0b00000001);
	_delay_us(2000);
}

/// Initializes the LCD
void lcd_init()
{
	// Set pins as outputs, except PB7..6, as they are used for XTAL
	DDRB = 0b00111111;
	PORTB = 0b00000000;

	// Wait some minimal delay until the LCD driver self-initializes and the voltages stabilize
	_delay_ms(50);

	// Initial sequence (semi 8-bit mode)
	PORTB = 0b000011;
	lcd_pulse_enable();
	// _delay_us(4100);
			_delay_us(4500);
	PORTB = 0b000011;
	lcd_pulse_enable();
	// _delay_us(100);
			_delay_us(450);
	PORTB = 0b000011;
	lcd_pulse_enable();
	// _delay_us(40);
			_delay_us(150);

	// Set 4-bit mode (semi 8-bit mode command)
	PORTB = 0b000010;
	lcd_pulse_enable();
	_delay_us(40);

	// Function: 4 bit mode, single line, 5x8 font
	lcd_command_4(0b00100000);

	// // Display off
	// lcd_command_4(0b00001000);

	// Display on, no blink, no cursor
	lcd_command_4(0b00001100);

	// Display clear
	lcd_command_4(0b00000001);
	_delay_us(1600);

	// Entry mode: increment address, no shift
	lcd_command_4(0b00000110);

	// // Display on, no blink, no cursor
	// lcd_command_4(0b00001100);

	// Reset position to 0
	lcd_command_4(0b10000000);
}
