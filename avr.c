/**
 * avr.c
 * Copyright (C) 2001-2020, Tony Givargis
 */

#include "avr.h"
#include <stdio.h>
void
avr_wait(unsigned short msec){
	TCCR0 = 1;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ) * 0.00005);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}