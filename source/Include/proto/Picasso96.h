/*
 * Platform-aware wrapper for Picasso96.
 *
 * On OS4 the library and interface are named Picasso96API, so we forward to
 * <proto/Picasso96API.h> and expose the same p96* function names via its own
 * inline headers. On every other target this is the canonical proto header
 * pattern (libraries + clib protos + inline or pragmas).
 */

#ifndef PROTO_PICASSO96_H
#define PROTO_PICASSO96_H

#ifdef __amigaos4__
    #include <proto/Picasso96API.h>
#else
    #include <exec/types.h>
    #include <clib/Picasso96_protos.h>

    #ifndef __NOLIBBASE__
    extern struct Library *
    #ifdef __CONSTLIBBASEDECL__
    __CONSTLIBBASEDECL__
    #endif
    P96Base;
    #endif

    #include <libraries/Picasso96.h>

    #ifdef __GNUC__
        #ifdef __PPC__
            #include <ppcinline/Picasso96.h>
        #else
            #include <inline/Picasso96.h>
        #endif
    #else
        #include <pragmas/Picasso96_pragmas.h>
    #endif
#endif

#endif
