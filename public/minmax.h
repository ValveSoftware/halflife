#ifdef __cplusplus

# undef min
# undef max

# ifndef MINMAX_H
# define MINMAX_H
# ifdef _WIN32
#   pragma once
# endif

extern "C++"
{
	// Josh: I would define my own, but older C++ versions we are targeting
	// had very very strange copy constructor rules which make it so, eg.
	// (double&, double) doesn't fit (const T& a, const T& b).
	// Just use the std min/max which solves it.
	#include <algorithm>

	#if defined(_MSC_VER) && _MSC_VER < 1300
	template <class T>
	inline const T& min(const T& x, const T& y)
	{
		return (x < y) ? x : y;
	}
	template <class T>
	inline const T& max(const T& x, const T& y)
	{
		return (x > y) ? x : y;
	}
	#else
	using std::min;
	using std::max;
	#endif

#if 0
	template< class T >
	inline const T& clamp( const T& val, const T& minVal, const T& maxVal )
	{
		if( val < minVal )
			return minVal;
		else if( val > maxVal )
			return maxVal;
		else
			return val;
	}
#else
# ifndef clamp
#  define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))
# endif
#endif

}
# endif

#else
# ifndef min
#  define min(a,b)  (((a) < (b)) ? (a) : (b))
# endif

# ifndef max
#  define max(a,b)  (((a) > (b)) ? (a) : (b))
# endif
# ifndef clamp
#  define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))
# endif
#endif
