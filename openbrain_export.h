#ifndef OPENBRAIN_EXPORT_H
#define OPENBRAIN_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef OPENBRAIN_EXPORT
# if defined(MAKE_PLASMA_APPLET_OPENBRAIN_LIB)
   /* We are building this library */ 
#  define OPENBRAIN_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define OPENBRAIN_EXPORT KDE_IMPORT
# endif
#endif

# ifndef OPENBRAIN_EXPORT_DEPRECATED
#  define OPENBRAIN_EXPORT_DEPRECATED KDE_DEPRECATED OPENBRAIN_EXPORT
# endif

#endif
