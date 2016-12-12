#ifndef __WINDOWS_H
#define __WINDOWS_H

/*
 * Define WINDOWS_H_INCLUDE_WINDOWS in order to include window operations and 
 * related staff.
 */

/* Vista target */
#define NTDDI_VERSION NTDDI_VISTA
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

/* XP target */
// #define WINVER 0x501
// #define _WIN32_WINNT 0x501

#define WIN32_LEAN_AND_MEAN

/* CC_*, LC_*, PC_*, CP_*, TC_*, RC_ */
#define NOGDICAPMASKS

/* VK_* */
#define NOVIRTUALKEYCODES

/* WM_*, EM_*, LB_*, CB_* */
#ifndef WINDOWS_H_INCLUDE_WINDOWS
# define NOWINMESSAGES
#endif

/* WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_* */
#ifndef WINDOWS_H_INCLUDE_WINSTYLES
# define NOWINSTYLES
#endif

/* SM_* */
#define NOSYSMETRICS

/* MF_* */
#ifndef WINDOWS_H_INCLUDE_MENUS
# define NOMENUS
#endif

/* IDI_* */
#define NOICONS

/* MK_* */
#define NOKEYSTATES

/* SC_* */
#ifndef WINDOWS_H_INCLUDE_SYSCOMMANDS
# define NOSYSCOMMANDS
#endif

/* Binary and Tertiary raster ops */
#define NORASTEROPS

/* SW_* */
#define NOSHOWWINDOW

/* OEM Resource values */
#define OEMRESOURCE

/* Atom Manager routines */
#define NOATOM

/* Clipboard routines */
#define NOCLIPBOARD

/* Screen colors */
#define NOCOLOR

/* Control and Dialog routines */
#define NOCTLMGR

/* DrawText() and DT_* */
#define NODRAWTEXT

/* All GDI defines and routines */
#define NOGDI

/* All KERNEL defines and routines */
// #define NOKERNEL

/* All USER defines and routines */
// #define NOUSER

/* All NLS defines and routines */
#define NONLS

/* MB_* and MessageBox() */
#define NOMB

/* GMEM_*, LMEM_*, GHND, LHND, associated routines */
#define NOMEMMGR

/* typedef METAFILEPICT */
#define NOMETAFILE

/* Macros min(a,b) and max(a,b) */
#define NOMINMAX

/* typedef MSG and associated routines */
#define NOMSG

/* OpenFile(), OemToAnsi, AnsiToOem, and OF_* */
#define NOOPENFILE

/* SB_* and scrolling routines */
#define NOSCROLL

/* All Service Controller routines, SERVICE_ equates, etc. */
// #define NOSERVICE

/* Sound driver routines */
#define NOSOUND

/* typedef TEXTMETRIC and associated routines */
#define NOTEXTMETRIC

/* SetWindowsHook and WH_* */
#define NOWH

/* GWL_*, GCL_*, associated routines */
#ifndef WINDOWS_H_INCLUDE_WINDOWS
# define NOWINOFFSETS
#endif

/* COMM driver routines */
#define NOCOMM

/* Kanji support stuff. */
#define NOKANJI

/* Help engine interface. */
#define NOHELP

/* Profiler interface. */
#define NOPROFILER

/* DeferWindowPos routines */
#define NODEFERWINDOWPOS

/* Modem Configuration Extensions */
#define NOMCX


#include <windows.h>


#undef NOGDICAPMASKS
#undef NOVIRTUALKEYCODES
#undef NOWINMESSAGES
#undef NOWINSTYLES
#undef NOSYSMETRICS
#undef NOMENUS
#undef NOICONS
#undef NOKEYSTATES
#undef NOSYSCOMMANDS
#undef NORASTEROPS
#undef NOSHOWWINDOW
#undef OEMRESOURCE
#undef NOATOM
#undef NOCLIPBOARD
#undef NOCOLOR
#undef NOCTLMGR
#undef NODRAWTEXT
#undef NOGDI
#undef NOKERNEL
#undef NOUSER
#undef NONLS
#undef NOMB
#undef NOMEMMGR
#undef NOMETAFILE
#undef NOMINMAX
#undef NOMSG
#undef NOOPENFILE
#undef NOSCROLL
#undef NOSERVICE
#undef NOSOUND
#undef NOTEXTMETRIC
#undef NOWH
#undef NOWINOFFSETS
#undef NOCOMM
#undef NOKANJI
#undef NOHELP
#undef NOPROFILER
#undef NODEFERWINDOWPOS
#undef NOMCX

#endif /* __WINDOWS_H */
