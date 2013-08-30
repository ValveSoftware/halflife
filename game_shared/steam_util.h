// steam_util.h
// Steam utility classes
// Author: Michael S. Booth (mike@turtlerockstudios.com), April 2003

#ifndef _STEAM_UTIL_H_
#define _STEAM_UTIL_H_

//--------------------------------------------------------------------------------------------------------------
/**
 * Used to load a file via Steam
 */
class SteamFile
{
public:
	SteamFile( const char *filename );
	~SteamFile();

	bool IsValid( void ) const				{ return (m_fileData) ? true : false; }	///< returns true if this file object is attached to a file
	bool Read( void *data, int length );		///< read 'length' bytes from the file

private:
	byte *m_fileData;												///< the file read into memory
	int m_fileDataLength;										///< the length of the file
	
	byte *m_cursor;													///< where we are in the file
	int m_bytesLeft;												///< the number of bytes left in the file
};

inline SteamFile::SteamFile( const char *filename )
{
	m_fileData = (byte *)LOAD_FILE_FOR_ME( const_cast<char *>( filename ), &m_fileDataLength );
	m_cursor = m_fileData;
	m_bytesLeft = m_fileDataLength;
}

inline SteamFile::~SteamFile()
{
	if (m_fileData)
		FREE_FILE( m_fileData );
}

inline bool SteamFile::Read( void *data, int length )
{
	if (length > m_bytesLeft || m_cursor == NULL || m_bytesLeft <= 0)
		return false;

	byte *readCursor = static_cast<byte *>( data );

	for( int i=0; i<length; ++i )
	{
		*readCursor++ = *m_cursor++;
		--m_bytesLeft;
	}

	return true;
}

#endif // _STEAM_UTIL_H_
