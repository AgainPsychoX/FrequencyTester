#pragma once

#include <avr/pgmspace.h>

/// Prints C-style string to the LCD
void lcd_print(const char* cstr);

/// Prints text of given length to the LCD
void lcd_print_n(const char* cstr, size_t length);

/// Prints C-style string from program memory to the LCD
void lcd_print_p(PGM_P cstr);

/// Prints text of given length from program memory to the LCD
void lcd_print_np(PGM_P cstr, size_t length);

/// Sets address/cursor position to given offset.
void lcd_set_cursor(uint8_t offset);

/// Resets address/cursor position & shift to original position (address 0)
void lcd_go_home();

/// Initializes the LCD
void lcd_init();
