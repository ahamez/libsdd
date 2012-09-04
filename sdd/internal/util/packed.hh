#ifndef _SDD_INTERNAL_UTIL_PACKED_HH_
#define _SDD_INTERNAL_UTIL_PACKED_HH_

#if defined LIBSDD_PACKED
#  define _LIBSDD_ATTRIBUTE_PACKED __attribute__ ((__packed__))
#else
#  define _LIBSDD_ATTRIBUTE_PACKED
#endif

#endif // _SDD_INTERNAL_UTIL_PACKED_HH_
