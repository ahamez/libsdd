#ifndef _SDD_UTIL_PACKED_HH_
#define _SDD_UTIL_PACKED_HH_

#if defined LIBSDD_PACKED
#  define LIBSDD_ATTRIBUTE_PACKED __attribute__ ((__packed__))
#else
#  define LIBSDD_ATTRIBUTE_PACKED
#endif

#endif // _SDD_UTIL_PACKED_HH_
