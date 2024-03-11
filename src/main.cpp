#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h"

#define HUMAN_READABLE_FREQUENCY

enum class Mode : uint8_t
{
	FrequencyBase,
	FrequencyExtra,
	BatteryVoltage,
};
constexpr auto modesCount = 3;
volatile Mode _mode;

union Flags
{
	uint8_t raw;
	struct 
	{
		bool reading : 1;	// Set if when inside display routing, to prevent issues when updating total count.
		bool overflow : 1;	// Set if overflow occurred when counting
	};
};
volatile Flags flags;

// next partial pulse count bytes from timer overflows
volatile uint8_t countExtra1;
volatile uint8_t countExtra2;
volatile uint8_t countExtra3;

volatile uint32_t fullCount; // last full pulse count value
volatile uint16_t batteryAnalogValue; // last battery analog value

ISR(TIMER0_OVF_vect)
{
	if (!++countExtra1) {
		if (!++countExtra2) {
			if (!++countExtra3) {
				flags.overflow = true;
			}
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	if (!flags.reading) {
		// Copy current counter value and extra bytes as total count
		*(reinterpret_cast<volatile uint8_t*>(&fullCount) + 0) = TCNT0;
		*(reinterpret_cast<volatile uint8_t*>(&fullCount) + 1) = countExtra1;
		*(reinterpret_cast<volatile uint8_t*>(&fullCount) + 2) = countExtra2;
		*(reinterpret_cast<volatile uint8_t*>(&fullCount) + 3) = countExtra3;
	}

	// Clear raw counter values for next cycle
	TCNT0 = 0;
	countExtra1 = 0;
	countExtra2 = 0;
	countExtra3 = 0;
}

inline void enableExternalPrescaler()
{
	PORTD &= ~(1 << PD5);
}
inline void disableExternalPrescaler()
{
	PORTD |= (1 << PD5);
}

inline void enableLED()
{
	PORTD |= (1 << PD6);
}
inline void disableLED()
{
	PORTD &= ~(1 << PD6);
}

void setupFrequencyMeasurement()
{
	// TC0 will be used to count positive edges
	// TC1 will be used as timeout counter (250ms)
#if (F_CPU > 16776960)
	OCR1A = F_CPU / 1024;
	OCR1B = 0xFFFF;
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS12); // prescaler = 256
#elif (F_CPU > 2097120)
	OCR1A = F_CPU / 256;
	OCR1B = 0xFFFF;
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // prescaler = 64
#else
	OCR1A = F_CPU / 32;
	OCR1B = 0xFFFF;
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11); // prescaler = 8
#endif
	TCCR0 = (1 << CS02) | (1 << CS01) | (1 << CS00); // count rising edges on T0
	TIMSK =	(1 << OCIE1A) | (1 << TOIE0); // TC1 A compare interrupt & TC0 overflow interrupt
}

inline Mode getMode()
{
	return _mode;
}

void setMode(Mode next)
{
	lcd_go_home();
	_mode = next;
	switch (next) {
		case Mode::FrequencyExtra:
			lcd_print_p(PSTR("Freq /16"));
			_delay_ms(1000);
			enableExternalPrescaler();
			enableLED();
			setupFrequencyMeasurement();
			break;
		case Mode::FrequencyBase:
			lcd_print_p(PSTR("Freq  /1"));
			_delay_ms(1000);
			disableExternalPrescaler();
			disableLED();
			setupFrequencyMeasurement();
			break;
		case Mode::BatteryVoltage:
			disableLED();
			TIMSK = 0; // disable all timer/counter interrupts
			ADMUX = (0 << REFS1) | (1 << REFS0); // AVcc ref., selected ADC0
			ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // enable the ADC with prescaler = 128
			break;
	}
}

inline void setNextMode()
{
	uint8_t n = (static_cast<uint8_t>(_mode) + 1) % modesCount;
	setMode(static_cast<Mode>(n));
}

inline bool isButtonPressed()
{
	return !(PIND & (1 << PD7));
}

char lcd_buffer[20];

int main(void)
{
	// Setup ports
	DDRD = (1 << PD5)	// External prescaler (divide 16) switch, enabled by LOW
		 | (1 << PD6);	// LED output pin, driven by HIGH
	PORTD = (1 << PD7);	// Button input, with pull-up

	// Setup LCD
	lcd_init();

	// Set initial mode
	flags.raw = 0;
	setMode(Mode::FrequencyBase);
	sei();

	while (1) {
		if (isButtonPressed()) {
			while (isButtonPressed()) {
				// Waiting for release
			}

			setNextMode();
		}

		// Move LCD cursor to start as we usually aim to overwrite whole width
		lcd_go_home();

		Mode mode = getMode();
		if (mode == Mode::FrequencyBase || mode == Mode::FrequencyExtra) {
			if (flags.overflow) {
				flags.overflow = false;
				lcd_print_p(PSTR("overflow"));
			}
			else {
				// Read the count and calculate frequency
				flags.reading = 1;
				uint32_t frequency = fullCount;
				flags.reading = 0;
				frequency *= mode == Mode::FrequencyExtra ? 64 : 4;

#ifdef HUMAN_READABLE_FREQUENCY
				// Prepare to display as MHz, kHz or Hz depending on which is easier to read
				if (frequency > 999'000) {
					uint8_t i = frequency / 1'000'000;
					uint16_t f = frequency / 100 % 10000;
					sprintf_P(lcd_buffer, PSTR("%u.%04uMHz"), i, f);
				}
				else if (frequency > 99'000) {
					uint8_t i = frequency / 1'000;
					uint16_t f = frequency % 1'000;
					sprintf_P(lcd_buffer, PSTR("%u.%03ukHz"), i, f);
				}
				else /* easily readable as accurate */ {
					sprintf_P(lcd_buffer, PSTR("%6luHz"), frequency);
				}
#else
				// Prepare to display, always as much accurate as possible (Hz)
				sprintf_P(lcd_buffer, PSTR("%6luHz"), frequency);
#endif

				lcd_print(lcd_buffer);
			}
		}
		else if (mode == Mode::BatteryVoltage) {
			// Start new conversion
			ADCSRA |= (1 << ADSC);
			while (ADCSRA & (1 << ADSC)) {
				// Waiting for conversion to finish
			}
			uint16_t raw = ADC;

			// Calculate the voltage (1 digit of precision)
			//	Voltage divider used: From 12V to 5V using 33kOhm and 24kOhm,
			//	accurate ratio would be: 11.8 / 102.4, so 59 / 512 is used.
			// TODO: check hardware and recalibrate it again, something is off
			uint8_t v_x10 = raw * 59 / 512;

			// Display on the LCD
			sprintf_P(lcd_buffer, PSTR("BAT %u.%uV"), v_x10 / 10, v_x10 % 10);
			lcd_print(lcd_buffer);
		}
	}

	return 0;
}

// TODO: custom itoa instead sprintf
// TODO: consider rewriting to assembly
