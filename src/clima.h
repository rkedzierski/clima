/**
 * @file clima.h
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __CLIMA_H__
#define __CLIMA_H__

#include "clima_cmds.h"
#include <stdlib.h>
#include <stddef.h>

/**
 * @brief Size of command buffer.
 */
#ifndef MAX_COMMAND_SIZE
#define MAX_COMMAND_SIZE 256
#endif

/**
 * @brief Maximum number of tokens in command.
 */
#ifndef MAX_COMMAND_TOKENS
#define MAX_COMMAND_TOKENS 16
#endif

/**
 * @brief New line character.
 */
#ifndef CLIMA_NEW_LINE
#define CLIMA_NEW_LINE	"\n"
#endif

#ifndef clima_size_t
/**
 * @brief Default declaration of eddy size type[replaceable]
 * 
 */
typedef size_t clima_size_t;
#endif

/**
 * @brief Library return type
 */
typedef enum clima_retv_e {
    CLIMA_RETV_OK,      /**< Returned if everyfing is OK. */
    CLIMA_RETV_ERR,     /**< Returned if error. */
} clima_retv_t;

/**
 * @{ \name Forward declarations
 */
struct clima_ctx_s;
struct clima_s;
/**
 * @}
 */

/**
 * @brief Definition of clima_ctx_s struct type.
 * @see clima_ctx_s
 */
typedef struct clima_ctx_s clima_ctx_t;

/**
 * @brief Definition of pointer on clima_ctx_s struct type.
 * @see clima_ctx_s
 */
typedef struct clima_ctx_s* clima_ctx_p;

/**
 * @brief Definition of clima_s struct type.
 * @see clima_s
 */
typedef struct clima_s clima_t;

/**
 * @brief Definition of pointer on clima_s struct type.
 * @see clima_s
 */
typedef struct clima_s* clima_p;

/**
 * @brief Pointer on print to terminal callback functrion.
 * @param string Pointer on buffer to print
 */
typedef clima_retv_t (*clima_log_print_clbk)(const char* string);

/**
 * @{ \name Pointers on API functions.
 */
typedef clima_retv_t (*clima_set_cli_print_clbk)(clima_p self, clima_cli_print_clbk cli_print_clbk);
typedef clima_retv_t (*clima_set_log_print_clbk)(clima_p self, clima_log_print_clbk log_print_clbk);
typedef clima_retv_t (*clima_set_cmds_root)(clima_p self, clima_command_p cmds_root);
typedef clima_retv_t (*clima_check_cmd)(clima_p self, char* cmd);
typedef clima_retv_t (*clima_exec_cmd)(clima_p self, const char* cmd);
typedef clima_retv_t (*clima_is_end)(clima_p self);
/**
 * @}
 */

/**
 * @brief Library initialization function
 * 
 * Initialize library context. It is posible to init many contextes.
 * 
 * @param self Pointer on library context.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t init_clima(clima_p self);

/**
 * @brief Clima malloc function implementation. [replaceable]
 * 
 * Fuction have internal default implementation with statdard malloc call.
 * Function implementation can be replaced because default function is defined with WEAK.
 * 
 * @param size Size of memory to allocation.
 * @return void* Pointer to allocated memory.
 */
void* clima_malloc(clima_size_t size);

/**
 * @brief Clima free function implementation. [replaceable]
 * 
 * Fuction have internal default implementation with statdard free call.
 * Function implementation can be replaced because default function is defined with WEAK.
 * 
 * @param ptr Pointer on memory block to free.
 */
void clima_free(void *ptr);

/**
 * @brief Library context with API
 * 
 * Example of use:
 * 
 */
struct clima_s {
    /**
     * @{ \name Library context.
    */
    clima_ctx_p ctx; /**< Internal private context.*/
    /**
     * @}
     */
    
    /**
     * @{ \name Library API 
    */
    clima_set_cli_print_clbk set_cli_print_clbk;    /**< To set terminal printing callback function. @see clima_set_cli_print_clbk_impl */
	clima_set_log_print_clbk set_log_print_clbk;    /**< To set logs printing callback function. @see clima_set_log_print_clbk_impl */
    clima_set_cmds_root set_cmds_root;              /**< To set pointer on command tree. @see clima_set_cmds_root_impl */
    clima_check_cmd check_command;                  /**< Function to passes command to check. Fn prints hints. @see clima_check_cmd_impl */
    clima_exec_cmd exec_command;                    /**< Function to passes command to execute. @see clima_exec_cmd_impl */
    clima_is_end is_end;                            /**< Function returns exit flag. True if exit is invoked. @see clima_is_end_impl */
    /**
     * @}
     */
};

#endif /* __CLIMA_H__ */
