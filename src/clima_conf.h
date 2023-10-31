/**
 * @file clima_conf.h
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <stddef.h>

/**
 * @brief Size of command buffer [configurable]
 */
#ifndef MAX_COMMAND_SIZE
#define MAX_COMMAND_SIZE 256
#endif

/**
 * @brief Maximum number of tokens in command [configurable]
 */
#ifndef MAX_COMMAND_TOKENS
#define MAX_COMMAND_TOKENS 16
#endif

/**
 * @brief Maximum number of search results [configurable]
 * 
 */
#ifndef MAX_SEARCH_RESULTS
#define MAX_SEARCH_RESULTS 32
#endif

/**
 * @brief New line character [configurable]
 */
#ifndef CLIMA_NEW_LINE
#define CLIMA_NEW_LINE	"\n\r"
#endif

/**
 * @brief Tabulator character [configurable]
 */
#ifndef CLIMA_TAB
#define CLIMA_TAB "\t"
#endif

/**
 * @brief Default declaration of eddy size type [configurable]
 */
#ifndef clima_size_t
typedef size_t clima_size_t;
#endif

/**
 * @brief Default ERROR message [configurable]
 */
#ifndef CLIMA_ERROR_MESSAGE
#define CLIMA_ERROR_MESSAGE "ERROR."
#endif

/**
 * @brief Default [ENTER] hint [configurable]
 */
#ifndef CLIMA_ENTER_MESSAGE
#define CLIMA_ENTER_MESSAGE "[ENTER]"
#endif

#ifndef CLIMA_MORE_MESSAGE
#define CLIMA_MORE_MESSAGE "-- more --"
#endif
