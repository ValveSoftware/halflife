//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "archtypes.h"     // DAL
#include "interface.h"

/**
*	@defgroup FileSystemV009 GoldSource Filesystem interface
*
*	@{
*/

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
/**
*	Filesystem file handle.
*/
typedef void * FileHandle_t;

/**
*	Filesystem find handle.
*/
typedef int FileFindHandle_t;

/**
*	Filesystem wait for resources handle.
*	Obsolete since the SteamPipe update.
*/
typedef int WaitForResourcesHandle_t;


//-----------------------------------------------------------------------------
// Enums used by the interface
//-----------------------------------------------------------------------------
#ifndef FILESYSTEM_INTERNAL_H

/**
*	Seek types.
*/
typedef enum
{
	/**
	*	Seek relative to the start of the file.
	*/
	FILESYSTEM_SEEK_HEAD = 0,

	/**
	*	Seek relative to the current position in the file.
	*/
	FILESYSTEM_SEEK_CURRENT,

	/**
	*	Seek relative to the end of the file.
	*/
	FILESYSTEM_SEEK_TAIL,
} FileSystemSeek_t;

enum
{
	/**
	*	Invalid filesystem find handle.
	*/
	FILESYSTEM_INVALID_FIND_HANDLE = -1
};

/**
*	Filesystem warning levels.
*/
typedef enum
{
	/**
	*	Don't print anything.
	*/
	FILESYSTEM_WARNING_QUIET = 0,

	/**
	*	On shutdown, report names of files left unclosed.
	*/
	FILESYSTEM_WARNING_REPORTUNCLOSED,

	/**
	*	Report number of times a file was opened, closed.
	*/
	FILESYSTEM_WARNING_REPORTUSAGE,

	/**
	*	Report all open/close events to console ( !slow! )
	*/
	FILESYSTEM_WARNING_REPORTALLACCESSES
} FileWarningLevel_t;

/**
*	Invalid file handle.
*/
#define FILESYSTEM_INVALID_HANDLE	( FileHandle_t )0
#endif

// turn off any windows defines
#undef GetCurrentDirectory

/**
*	Purpose: Main file system interface
*/
class IFileSystem : public IBaseInterface
{
public:
	// Mount and unmount the filesystem
	/**
	*	Mounts the Steam filesystem. The engine is responsible for calling this.
	*	Obsolete since the SteamPipe update.
	*/
	virtual void			Mount(void) = 0;

	/**
	*	Unmounts the Steam filesystem. The engine is responsible for calling this.
	*	Obsolete since the SteamPipe update.
	*/
	virtual void			Unmount(void) = 0;

	/**
	*	Remove all search paths (including write path)
	*/
	virtual void			RemoveAllSearchPaths( void ) = 0;

	/**
	*	Add paths in priority order (mod dir, game dir, ....)
	*	If one or more .pak files are in the specified directory, then they are
	*	 added after the file system path
	*	If the path is the relative path to a .bsp file, then any previous .bsp file 
	*	 override is cleared and the current .bsp is searched for an embedded PAK file
	*	 and this file becomes the highest priority search path ( i.e., it's looked at first
	*	  even before the mod's file system path ).
	*	@param pPath Relative path to the directory to search in. Starts in the root game directory (e.g. Half-Life/)
	*	@param pathID Which path ID this should be assigned to. Can be null, in which case the path will only be used if a null path ID is given to methods taking a path ID.
	*/
	virtual void			AddSearchPath( const char *pPath, const char *pathID ) = 0;

	/**
	*	Removes a search path. Causes the game to crash due to an illegal access exception.
	*	See https://github.com/ValveSoftware/halflife/issues/1715 for more information.
	*	@param pPath Path to remove.
	*	@return true if the path was removed, false otherwise.
	*/
	virtual bool			RemoveSearchPath( const char *pPath ) = 0;

	/**
	*	Deletes a file.
	*	@param pRelativePath The relative path to the file. This path can be relative to any search path that is part of the given path ID.
	*	@param pathID If not null, only search paths defined for the given path ID will be used to search for the file.
	*/
	virtual void			RemoveFile( const char *pRelativePath, const char *pathID ) = 0;

	/**
	*	Creates a directory hierarchy.
	*	This isn't implementable on STEAM as is. (The Steam filesystem is no longer used since the SteamPipe update)
	*	@param path Path containing one or more directories to create.
	*			Uses the standard path syntax, e.g. models/player/freeman creates the directory models, creates the directory player in that directory, and creates the directory freeman in that directory.
	*	@param pathID If not null, uses the first search path defined for the given path ID as the basis for the full path.
	*			If null, or if no search paths are defined for the given path ID, uses the first write path that was added.
	*/
	virtual void			CreateDirHierarchy( const char *path, const char *pathID ) = 0;

	// File I/O and info
	/**
	*	Checks if the given file exists. Checks all search paths.
	*	@param pFileName File to check.
	*	@return Whether the given file exists or not.
	*/
	virtual bool			FileExists( const char *pFileName ) = 0;

	/**
	*	Checks if the given file is a directory.
	*	@param pFileName File to check.
	*	@return Whether the given file is a directory or not.
	*/
	virtual bool			IsDirectory( const char *pFileName ) = 0;

	/**
	*	Opens a file.
	*	Note: can open directories as though they were files, but will fail to read anything.
	*	@param pFileName Name of the file to open.
	*	@param pOptions File open options. Matches the set of options provided by fopen. See http://www.cplusplus.com/reference/cstdio/fopen/#parameters
	*	@param pathID Which search paths should be used to find the file. If NULL, all paths will be searched for the file.
	*	@return Handle to the file, or FILESYSTEM_INVALID_HANDLE if the file could not be opened.
	*	@see FILESYSTEM_INVALID_HANDLE
	*/
	virtual FileHandle_t	Open( const char *pFileName, const char *pOptions, const char *pathID = 0L ) = 0;

	/**
	*	Closes a file handle that was previously returned by a call to Open. The handle is no longer valid after this call.
	*	@param file Handle to the file to close.
	*/
	virtual void			Close( FileHandle_t file ) = 0;

	/**
	*	@param file Handle to the file.
	*	@param pos Position to seek to.
	*	@param seekType Position to use as a reference for pos.
	*	@see FileSystemSeek_t
	*/
	virtual void			Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType ) = 0;

	/**
	*	Gets the current read/write offset in the file, measured in bytes.
	*	@param file Handle to the file.
	*	@return Offset.
	*/
	virtual unsigned int	Tell( FileHandle_t file ) = 0;

	/**
	*	Gets the size of the file, in bytes.
	*	@param file Handle to the file.
	*	@return Size of the file.
	*/
	virtual unsigned int	Size( FileHandle_t file ) = 0;

	/**
	*	Gets the size of the file, in bytes.
	*	@param pFileName Name of the file.
	*	@return Size of the file. Returns -1 if the file couldn't be found.
	*/
	virtual unsigned int	Size( const char *pFileName ) = 0;

	/**
	*	Gets the 32 bit UNIX timestamp at which the given file was last modified.
	*	@param pFileName File to check.
	*	@return Timestamp, or 0 if the file could not be queried for information.
	*/
	virtual long			GetFileTime( const char *pFileName ) = 0;

	/**
	*	Converts the given file time to a string. The resulting string produces output as if the CRT function ctime were used.
	*	@param pString Destination buffer.
	*	@param maxCharsIncludingTerminator Maximum number of characters that can be written to pStrip, including the null terminator.
	*	@param fileTime File time to convert.
	*/
	virtual void			FileTimeToString( char* pStrip, int maxCharsIncludingTerminator, long fileTime ) = 0;

	/**
	*	Checks whether the file's I/O status is good for input or output operations to occur on it.
	*	@param file Handle to the file.
	*	@return File I/O status.
	*/
	virtual bool			IsOk( FileHandle_t file ) = 0;

	/**
	*	Flushes pending changes to disk.
	*	@param file Handle to the file.
	*/
	virtual void			Flush( FileHandle_t file ) = 0;

	/**
	*	Checks whether the end of the file has been reached by an input operation.
	*	@param file Handle to the file.
	*	@param Whether the end of the file has been reached or not.
	*/
	virtual bool			EndOfFile( FileHandle_t file ) = 0;

	/**
	*	Reads a number of bytes from the file.
	*	@param pOutput Destination buffer.
	*	@param size Size of the destination buffer, in bytes.
	*	@param file Handle to the file.
	*	@return Number of bytes that were read.
	*/
	virtual int				Read( void* pOutput, int size, FileHandle_t file ) = 0;

	/**
	*	Writes a number of bytes to the file.
	*	@param pInput Source buffer.
	*	@param size Size of the source buffer, in bytes.
	*	@param file Handle to the file.
	*	@return Number of bytes that were written.
	*/
	virtual int				Write( void const* pInput, int size, FileHandle_t file ) = 0;

	/**
	*	Reads a line from the file.
	*	@param pOutput Destination buffer.
	*	@param maxChars Maximum number of characters to read, including the null terminator.
	*	@param file Handle to the file.
	*	@return If the read operation succeeded, pointer to the destination buffer. Otherwise, a pointer to a read-only empty string is returned.
	*/
	virtual char			*ReadLine( char *pOutput, int maxChars, FileHandle_t file ) = 0;

	/**
	*	Print formatted data to the file.
	*	@param file Handle to the file.
	*	@param pFormat Format string.
	*	@param ... Arguments.
	*	@return If the operation succeeded, returns the number of characters that were written. Otherwise, returns a negative number.
	*/
	virtual int				FPrintf( FileHandle_t file, char *pFormat, ... ) = 0;

	// direct filesystem buffer access

	/**
	*	Returns a handle to a buffer containing the file data.
	*	This is the optimal way to access the complete data for a file, 
	*	since the file preloader has probably already got it in memory.
	*
	*	Note: always fails. Sets outBufferSize to 0 and returns null in all cases.
	*
	*	@param file Handle to the file.
	*	@param outBufferSize Pointer to a variable that will contain the size of the buffer.
	*	@param failIfNotInCache If true, do not load the file into memory if it wasn't already cached.
	*	@return Pointer to the buffer, or null if the file buffer couldn't be loaded into memory, or if it wasn't loaded and failIfNotInCache was true.
	*/
	virtual void			*GetReadBuffer( FileHandle_t file, int *outBufferSize, bool failIfNotInCache ) = 0;

	/**
	*	Releases a read buffer previously returned by a call to GetReadBuffer.
	*	The buffer is no longer valid after this call.
	*	@param file Handle to the file.
	*	@param readBuffer Buffer to release.
	*/
	virtual void            ReleaseReadBuffer( FileHandle_t file, void *readBuffer ) = 0;

	// FindFirst/FindNext
	/**
	*	Finds the first file using a given wildcard and set of search paths.
	*	Note: Check if a file was found using the return value, not the pHandle parameter.
	*	@param pWildCard WildCard to use when searching.
	*	@param pHandle If a file was found, this is set to the find handle. If no file was found, this parameter is not modified.
	*	@param pathID If not null, the search paths assigned to this path ID are searched. Otherwise, all paths are searched.
	*	@return If a file was found, returns the name of the file. Otherwise, returns null.
	*/
	virtual const char		*FindFirst( const char *pWildCard, FileFindHandle_t *pHandle, const char *pathID = 0L ) = 0;

	/**
	*	Finds the next file using the wildcard set by a previous call to FindFirst.
	*	@param handle Handle set by a previous call to FindFirst.
	*	@return If a file was found, returns the name of the file. Otherwise, returns null.
	*	@see FindFirst
	*/
	virtual const char		*FindNext( FileFindHandle_t handle ) = 0;

	/**
	*	Checks whether the last file returned by a call to FindFirst or FindNext is a directory.
	*	@param handle Handle set by a previous call to FindFirst.
	*	@return Whether the file is a directory or not.
	*/
	virtual bool			FindIsDirectory( FileFindHandle_t handle ) = 0;

	/**
	*	Closes a find operation. The handle will be invalid after this call.
	*	@param handle Handle set by a previous call to FindFirst.
	*/
	virtual void			FindClose( FileFindHandle_t handle ) = 0;

	/**
	*	Creates a local copy for a file in a GCF file.
	*	Obsolete since the SteamPipe update, does nothing.
	*	@param pFileName Name of the file.
	*/
	virtual void			GetLocalCopy( const char *pFileName ) = 0;

	/**
	*	Gets the local path of a file. This converts a relative path to an absolute path. All search paths are checked.
	*	@param pFileName Name of the file.
	*	@param pLocalPath Destination buffer.
	*	@param localPathBufferSize Size of the buffer, including null terminator.
	*	@return Pointer to pLocalPath, or null if the file could not be found.
	*/
	virtual const char		*GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize ) = 0;

	/**
	*	Parses a string into tokens.
	*	Note: This function does not take a buffer size parameter. Be careful when using this.
	*	Note: This is sort of a secondary feature; but it's really useful to have it here.
	*
	*	Uses the characters "{}()':" to split tokens.
	*
	*	@param pFileBytes String to parse.
	*	@param pToken Destination buffer.
	*	@param pWasQuoted Optional. Whether the token was quoted or not.
	*	@return pointer to the next character in the string to parse.
	*/
	virtual char			*ParseFile( char* pFileBytes, char* pToken, bool* pWasQuoted )	= 0;

	/**
	*	Converts a full (absolute) path to a relative path.
	*	Note: This function does not take a buffer size parameter. Ensure pRelative is large enough to store at least the entire pFullpath string to prevent buffer overflows.
	*	@param pFullpath Path to convert.
	*	@param pRelative Destination buffer.
	*	@return true on success ( based on current list of search paths, otherwise false if 
	*	it can't be resolved )
	*/
	virtual bool			FullPathToRelativePath( const char *pFullpath, char *pRelative ) = 0;

	/**
	*	Gets the current working directory.
	*	@param pDirectory Destination buffer. Must not be null.
	*	@param maxlen Maximum number of characters, including the null terminator.
	*/
	virtual bool			GetCurrentDirectory( char* pDirectory, int maxlen ) = 0;

	/**
	*	Dump to printf/OutputDebugString the list of files that have not been closed.
	*/
	virtual void			PrintOpenedFiles( void ) = 0;

	/**
	*	Sets the warning function to invoke when a problem occurs.
	*	Note: This may be called during shutdown. Do not set a function that is not guaranteed to be in memory for at least as long as the filesystem itself.
	*	@param pfnWarning Function to set.
	*/
	virtual void			SetWarningFunc( void (*pfnWarning)( const char *fmt, ... ) ) = 0;

	/**
	*	Sets the warning level. Higher levels output more information.
	*	@param level Warning level.
	*/
	virtual void			SetWarningLevel( FileWarningLevel_t level ) = 0;

	/**
	*	Does nothing.
	*/
	virtual void			LogLevelLoadStarted( const char *name ) = 0;

	/**
	*	Does nothing.
	*/
	virtual void			LogLevelLoadFinished( const char *name ) = 0;

	/**
	*	Hints that a set of files will be loaded in near future.
	*	HintResourceNeed() is not to be confused with resource precaching.
	*	Obsolete since the SteamPipe update.
	*/
	virtual int				HintResourceNeed( const char *hintlist, int forgetEverything ) = 0;

	/**
	*	Pauses resource preloading.
	*	Obsolete since the SteamPipe update.
	*/
	virtual int				PauseResourcePreloading( void ) = 0;

	/**
	*	Resumes resource preloading.
	*	Obsolete since the SteamPipe update.
	*/
	virtual int				ResumeResourcePreloading( void ) = 0;

	/**
	*	Sets the buffer for the given file.
	*	See the CRT function setvbuf (http://www.cplusplus.com/reference/cstdio/setvbuf/) for more information.
	*	@param stream Handle to the file.
	*	@param buffer Buffer to use.
	*	@param mode Buffering mode.
	*	@param size Size of the buffer, in bytes.
	*	@return If successful, returns 0. Otherwise, returns a non-zero value.
	*/
	virtual int				SetVBuf( FileHandle_t stream, char *buffer, int mode, long size ) = 0;

	/**
	*	Gets the name and version of the interface.
	*	Note: The buffer may not be null terminated if the result has a number of characters equal to or greater than the size of the buffer.
	*	Note: This is not the same as the CreateInterface name.
	*	@param p Destination buffer.
	*	@param maxlen Size of the destination buffer, in characters, including the null terminator.
	*/
	virtual void			GetInterfaceVersion( char *p, int maxlen ) = 0;

	/**
	*	Checks whether the given file is immediately available for file operations.
	*	Obsolete since the SteamPipe update.
	*	@param pFileName Name of the file.
	*	@return true in all cases.
	*/
	virtual bool			IsFileImmediatelyAvailable(const char *pFileName) = 0;

	/**
	*	Starts waiting for resources to be available.
	*	Obsolete since the SteamPipe update.
	*	@return FILESYSTEM_INVALID_HANDLE if there is nothing to wait on.
	*/
	virtual WaitForResourcesHandle_t WaitForResources( const char *resourcelist ) = 0;

	/**
	*	Get progress on waiting for resources; progress is a float [0, 1], complete is true on the waiting being done.
	*	Any calls after complete is true or on an invalid handle will return false, 0.0f, true.
	*	Obsolete since the SteamPipe update.
	*	@return false if no progress is available, true otherwise.
	*/
	virtual bool			GetWaitForResourcesProgress( WaitForResourcesHandle_t handle, float *progress /* out */ , bool *complete /* out */ ) = 0;

	/**
	*	Cancels a progress call.
	*	Obsolete since the SteamPipe update.
	*/
	virtual void			CancelWaitForResources( WaitForResourcesHandle_t handle ) = 0;

	/**
	*	Checks whether the appID has all its caches fully preloaded.
	*	Obsolete since the SteamPipe update.
	*	@return true if the appID has all its caches fully preloaded.
	*/
	virtual bool			IsAppReadyForOfflinePlay( int appID ) = 0;

	/**
	*	Adds pack files to the given path ID for searching.
	*	Interface for custom pack files > 4Gb.
	*	@param fullpath Path to the file. The path should be relative to the current working directory.
	*	@param pathID If not null, the path ID to add the pack file to. Otherwise, only used if null path IDs are used for searching.
	*	@return true if the pack file was successfully added, false otherwise.
	*	@see GetCurrentDirectory
	*/
	virtual bool			AddPackFile( const char *fullpath, const char *pathID ) = 0;
	
	/**
	*	Open a file but force the data to come from the steam cache, NOT from disk.
	*	Similar to Open, but will not fall back to the first write path for non-existent or null path IDs.
	*	Instead, non-existent path IDs result in a failure to open, and null IDs result in all paths being searched.
	*	@see Open
	*/
	virtual FileHandle_t	OpenFromCacheForRead( const char *pFileName, const char *pOptions, const char *pathID = 0L ) = 0;

	/**
	*	Adds a read-only search path.
	*	@see AddSearchPath
	*/
	virtual void			AddSearchPathNoWrite( const char *pPath, const char *pathID ) = 0;
};

// Steam3/Src compat
#define IBaseFileSystem IFileSystem

/**
*	Interface name.
*/
#define FILESYSTEM_INTERFACE_VERSION "VFileSystem009"

/** @} */

#endif // FILESYSTEM_H
