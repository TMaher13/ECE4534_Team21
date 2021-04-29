/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*****************************************************************************
// Includes
//*****************************************************************************
// Standard includes

#ifndef DEBUG_IF_H
#define DEBUG_IF_H

/* Define Debug Method */
#define  D_UART_PRINT       0
#define  D_DISPLAY_PRINT    1
#define  F_USER_DEFINED     2

#define D_DEBUG_METHOD    D_UART_PRINT


/* Define Supported Modules */
#define D_GENERAL_IF    (1)
#define D_MQTT_IF       (1)
#define D_OTA_IF        (1)

/* Define Severity Level */
enum
{
    E_TRACE, E_DEBUG, E_INFO, E_WARNING, E_ERROR, E_FATAL, E_DISABLED
};

#define IF_NAME     "GEN"
#define IF_SEVERITY E_INFO


#if (D_DEBUG_METHOD == D_UART_PRINT)
    #include "uart_term.h"
    #define PRINTF(...)  UART_PRINT(__VA_ARGS__);UART_PRINT("\n\r");
#elif (D_DEBUG_METHOD == D_DISPLAY_PRINT)
    #include <ti/display/Display.h>
    extern Display_Handle display;
    #define PRINTF(_module_, _severity_, ...)   Display_printf(display, 0, 0, __VA_ARGS__);
#else
    /* USER DEFIND PRINT METHOD */
    #ifndef PRINTF
        #error "Missing definition of PRINTF method"
    #endif
#endif

#define sl_printf(_severity_, ...) if(_severity_)>= D_SEVERITY)   { PRINTF("[" IF_NAME "] "__VA_ARGS__); }
#define LOG_FATAL( ...)   if(E_FATAL >= IF_SEVERITY)     { PRINTF("[" IF_NAME "::FATAL] "__VA_ARGS__); }
#define LOG_ERROR( ...)   if(E_ERROR >= IF_SEVERITY)     { PRINTF("[" IF_NAME "::ERROR] "__VA_ARGS__); }
#define LOG_WARNING( ...) if(E_WARNING >= IF_SEVERITY)      { PRINTF("[" IF_NAME "::WARN] "__VA_ARGS__); }
#define LOG_INFO( ...)    if(E_INFO >= IF_SEVERITY)      { PRINTF("[" IF_NAME "::INFO] "__VA_ARGS__); }
#define LOG_DEBUG( ...)   if(E_DEBUG >= IF_SEVERITY)     { PRINTF("[" IF_NAME "::DEBUG] "__VA_ARGS__); }
#define LOG_TRACE( ...)   if(E_TRACE >= IF_SEVERITY)     { PRINTF("[" IF_NAME "::TRACE] "__VA_ARGS__); }


#endif // DEBUG_IF_H
