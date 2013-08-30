#ifndef _METER_H_
#define _METER_H_

/*
	meter.h

	Dorky status bar stuff
*/

void MeterStart( int max );
void MeterAdvance( int amt );
void MeterEnd( void );

extern int showmeter;

#endif