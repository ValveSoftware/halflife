#ifndef __WXDEBUG__
#define __WXDEBUG__

// This library provides fairly straight forward debugging functionality, this
// is split into two main sections. The first is assertion handling, there are
// three types of assertions provided here. The most commonly used one is the
// ASSERT(condition) macro which will pop up a message box including the file
// and line number if the condition evaluates to FALSE. Then there is the
// EXECUTE_ASSERT macro which is the same as ASSERT except the condition will
// still be executed in NON debug builds. The final type of assertion is the
// KASSERT macro which is more suitable for pure (perhaps kernel) filters as
// the condition is printed onto the debugger rather than in a message box.
//
// The other part of the debug module facilties is general purpose logging.
// This is accessed by calling DbgLog(). The function takes a type and level
// field which define the type of informational string you are presenting and
// it's relative importance. The type field can be a combination (one or more)
// of LOG_TIMING, LOG_TRACE, LOG_MEMORY, LOG_LOCKING and LOG_ERROR. The level
// is a DWORD value where zero defines highest important. Use of zero as the
// debug logging level is to be encouraged ONLY for major errors or events as
// they will ALWAYS be displayed on the debugger. Other debug output has it's
// level matched against the current debug output level stored in the registry
// for this module and if less than the current setting it will be displayed.
//
// Each module or executable has it's own debug output level for each of the
// five types. These are read in when the DbgInitialise function is called
// for DLLs linking to STRMBASE.LIB this is done automatically when the DLL
// is loaded, executables must call it explicitely with the module instance
// handle given to them through the WINMAIN entry point. An executable must
// also call DbgTerminate when they have finished to clean up the resources
// the debug library uses, once again this is done automatically for DLLs

// These are the five different categories of logging information

#ifdef _DEBUG


enum
{
	LOG_TRACE		= 0x00000001,	// General tracing
    LOG_ENTRY		= 0x00000002,	// Function entry logging
    LOG_EXIT		= 0x00000004,	// Function exit logging
    LOG_MEMORY		= 0x00000008,	// Memory alloc/free debugging
    LOG_ERROR		= 0x00000010,	// Error notification
	LOG_UNUSED0		= 0x00000020,	// reserved
	LOG_UNUSED1		= 0x00000040,	// reserved
	LOG_UNUSED2		= 0x00000080,	// reserved
	LOG_CHUM		= 0x00000100,	// Chumtoad debugging
	LOG_LEECH		= 0x00000200,	// Leech debugging
	LOG_ICHTHYOSAUR = 0x00000400,   // Ichthyosaur debugging
};


// These are public but should be called only by the DLLMain function
void WINAPI DbgInitialise(HINSTANCE hInst);
void WINAPI DbgTerminate();
// These are public but should be called by macro only
void WINAPI DbgKernelAssert(const TCHAR *pCondition,const TCHAR *pFileName,INT iLine);
void WINAPI DbgLogInfo(DWORD Type,DWORD Level,const TCHAR *pFormat,...);
void WINAPI DbgOutString(LPCTSTR psz);


// These are the macros that should be used in code.

#define DBGASSERT(_x_) \
    if (!(_x_)) \
        DbgKernelAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)

#define DBGBREAK(_x_)                   \
    DbgKernelAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)

#define DBGASSERTEXECUTE(_x_) DBGASSERT(_x_)

#define DBGLOG(_x_) DbgLogInfo _x_

#define DBGOUT(_x_) DbgOutString(_x_)

#define ValidateReadPtr(p,cb) \
    {if(IsBadReadPtr((PVOID)p,cb) == TRUE) \
        DBGBREAK("Invalid read pointer");}

#define ValidateWritePtr(p,cb) \
    {if(IsBadWritePtr((PVOID)p,cb) == TRUE) \
        DBGBREAK("Invalid write pointer");}

#define ValidateReadWritePtr(p,cb) \
    {ValidateReadPtr(p,cb) ValidateWritePtr(p,cb)}

#define ValidateStringPtr(p) \
    {if(IsBadStringPtr((LPCTSTR)p,INFINITE) == TRUE) \
        DBGBREAK("Invalid string pointer");}

#define ValidateStringPtrA(p) \
    {if(IsBadStringPtrA((LPCSTR)p,INFINITE) == TRUE) \
        DBGBREAK("Invalid ANSII string pointer");}

#define ValidateStringPtrW(p) \
    {if(IsBadStringPtrW((LPCWSTR)p,INFINITE) == TRUE) \
        DBGBREAK("Invalid UNICODE string pointer");}

#else // !_DEBUG

// Retail builds make public debug functions inert  - WARNING the source
// files do not define or build any of the entry points in debug builds
// (public entry points compile to nothing) so if you go trying to call
// any of the private entry points in your source they won't compile

#define DBGASSERT(_x_)
#define DBGBREAK(_x_)
#define DBGASSERTEXECUTE(_x_) _x_
#define DBGLOG(_x_)
#define DBGOUT(_x_)
#define ValidateReadPtr(p,cb)
#define ValidateWritePtr(p,cb)
#define ValidateReadWritePtr(p,cb)
#define ValidateStringPtr(p)
#define ValidateStringPtrA(p)
#define ValidateStringPtrW(p)

#endif  // !_DEBUG


#ifndef REMIND
    //  REMIND macro - generates warning as reminder to complete coding
    //  (eg) usage:
    //
    //  #pragma message (REMIND("Add automation support"))


    #define REMINDQUOTE(x) #x
    #define REMINDQQUOTE(y) REMINDQUOTE(y)
    #define REMIND(str) __FILE__ "(" REMINDQQUOTE(__LINE__) ") :  " str
#endif

#endif // __WXDEBUG__


