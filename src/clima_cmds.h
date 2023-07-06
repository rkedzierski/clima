/**
 * @file clima_cmds.h
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __CLIMA_CMDS_H__
#define __CLIMA_CMDS_H__

#include "clima_conf.h"

/**
 * @brief 
 */
#define CLIMA_RETURN_ERROR_IF_EXTRA_ARGS \
	if(cmd_struct->args) { \
		cprint("ERROR.\n\r"); \
		return CLIMA_CMD_OK; \
	}

/**
 * @brief Pointer on terminal printing function.
 */
typedef void (*clima_cli_print_clbk)(const char* string);

/**
 * @brief Forward declarations.
 */
struct  clima_command_s;

/**
 * @brief Structure of complete command.
 */
typedef struct cmd_struct_s {
    int token_idx[16];		/**< Table of all of command tokens indexes. */
    int tokens_count;		/**< Number of tokens in command. */
	const char *args;		/**< Argument after command. */
} cmd_struct_t;

/**
 * @brief Definition of pointer on cmd_struct_s struct type.
 * @see cmd_struct_s
 */
typedef cmd_struct_t* cmd_struct_p;

/**
 * @brief Internal bool definition.
 * TODO: remove
 */
typedef enum clima_bool_e {
	CLIMA_FALSE = 0,
	CLIMA_TRUE = 1
} clima_bool_t;

/**
 * @brief Executed function return type. 
*/
typedef enum clima_cmd_fn_retv_e {
	CLIMA_CMD_ERR,			/**< Returned if error. */
	CLIMA_CMD_OK,			/**< Returned if everyfing is OK. */
	CLIMA_CMD_EXIT			/**< Returned if exit command is invoked. */
} clima_cmd_fn_retv_t;

/**
 * @brief Pointer on command callback function.
 * 
 * @param pbCmd 
 * @param psCmdTab 
 * @param cmd_struct Pointer on command structure @see cmd_struct_s
 * @param cprint TODO: remove!
 * @return int Error code: CLIMA_CMD_OK if succes, CLIMA_CMD_ERR if error or CLIMA_CMD_EXIT if exit is invoked. TODO: change on clima_cmd_fn_retv_t
 */
typedef int (*fn_handler_t) (const char *pbCmd, struct clima_command_s **psCmdTab, cmd_struct_t* cmd_struct, clima_cli_print_clbk cprint);

/**
 * @brief Structure with definition of entrace in command tree.
 * 
 */
typedef struct clima_command_s {
	char *cmd; 							/**< Command token name string. */
    char *hint;							/**< String with hint. */
	char *args_hint;					/**< String with command argument hint. */
	struct clima_command_s *next_cmd;	/**< Pointer on next token table. */
	fn_handler_t fn_handler;			/**< Pointer on command function. */
} clima_command_t;

/**
 * @brief Definition of pointer on clima_command_p struct type.
 * @see clima_command_p
 */
typedef clima_command_t* clima_command_p;

#endif /* __CLIMA_CMDS_H__ */
