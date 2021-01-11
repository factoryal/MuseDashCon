// --------------------------------------
//    ArcadeBoardTypeA Version Selector
// --------------------------------------
// https://github.com/factoryal/Arcade-Board

// !!! IMPORTANT !!!
// Make sure these following three version defines must be defined before including this header file.
// >> BOARD_MAJOR_VERSION, BOARD_MINOR_VERSION, BOARD_PATCH_VERSION
// Check your board version written on PCB and write proper version value!
//
// example >>>
// If board version written on PCB is 'v1.0.2',
// #define BOARD_MAJOR_VERSION 1
// #define BOARD_MINOR_VERSION 0
// #define BOARD_PATCH_VERSION 2
// #include "ArcadeBoardTypeA.h"

#pragma once

#if defined(BOARD_MAJOR_VERSION) && defined(BOARD_MINOR_VERSION) && defined(BOARD_PATCH_VERSION)
    #define _BOARD_VERSION BOARD_MAJOR_VERSION * 10000 + BOARD_MINOR_VERSION * 100 + BOARD_PATCH_VERSION
    #if (_BOARD_VERSION > 10000 && _BOARD_VERSION < 10299)
        #include "ArcadeBoardTypeA_1_0_0.h"
    #else
        #error "Invalid BOARD_VERSION. Please check proper version value or update library to latest version."
    #endif
#else
    #error "Cannot find board version information."
#endif