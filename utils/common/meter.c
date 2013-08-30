/*
	meter.c

	Implements a dorky progess meter
*/

int showmeter = 0;
static int meter_cur, meter_max;

void MeterStart( int max )
{
	meter_cur = 0;
	meter_max = max;
}


void MeterAdvance( int amt )
{
	float pct;

	meter_cur += amt;

	if( showmeter )
	{
		pct = ( (float) meter_cur / (float) meter_max ) * 100.0;
		printf( "\r%d/%d (%0.2f%%)                   ", meter_cur, meter_max, pct );
	}
}


void MeterEnd( void )
{
	printf( "\n" );
}
