// simple_checksum.h
// Functions to compute a simple checksum value for a file
// Author: Michael S. Booth, Turtle Rock Studios (www.turtlerockstudios.com), September 2003

#ifndef _SIMPLE_CHECKSUM_H_
#define _SIMPLE_CHECKSUM_H_

/**
 * Compute a simple checksum for the given data.
 * Each byte in the data is multiplied by its position to track re-ordering changes
 */
inline unsigned int ComputeSimpleChecksum( const unsigned char *dataPointer, int dataLength )
{
	unsigned int checksum = 0;

	for( int i=1; i<=dataLength; ++i )
	{
		checksum += (*dataPointer) * i;
		++dataPointer;
	}

	return checksum;
}

#endif // _SIMPLE_CHECKSUM_H_
