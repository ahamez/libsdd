#pragma once

#if defined LIBSDD_PACKED
#  define LIBSDD_ATTRIBUTE_PACKED __attribute__ ((__packed__))
#else
#  define LIBSDD_ATTRIBUTE_PACKED
#endif
