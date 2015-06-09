#ifndef VMINMAX_H
#define VMINMAX_H

#if defined ( _MSC_VER )
#pragma once
#endif

#define V_max(a, b)  (((a) > (b)) ? (a) : (b))
#define V_min(a, b)  (((a) < (b)) ? (a) : (b))
#define V_fabs(x)	 ((x) > 0 ? (x) : 0 - (x))

#endif // VMINMAX_H