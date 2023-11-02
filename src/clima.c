/**
 * @file clima.c
 * @author Rafał Kędzierski (rafal.kedzierski@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "clima.h"
#include "clima_cmds.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef   __WEAK
  #define __WEAK    __attribute__((weak))
#endif

/**
 * @brief TODO: remove
 * 
 */
#define CLIMA_NULL 0

/**
 * @brief Macro to check is end of token.
 * Detect space or end of string.
 */
#define IS_CLIMA_END_CHAR(ch) ((ch=='\0') || (ch==' '))

/**
 * @brief Define of command parsing results.
 * Private type.
 * Status returned by clima_parse_cmd function.
 * Status of parsing is processing in clima_check_cmd_impl and clima_exec_cmd_impl functions.
 * @see clima_parse_cmd, clima_check_cmd_impl, clima_exec_cmd_impl
 */
typedef enum parse_result_e {
    SCLI_PARSE_ERROR,                   /**< Parsing function error. */
    SCLI_PARSE_NO_RESULTS,              /**< Command not founded in command tree. No results. */
    SCLI_PARSE_SINGLE_RESULT_ARGS,      /**< Command founded. Single result with argument after command. */
    SCLI_PARSE_SINGLE_RESULT,           /**< Command founded. Single result. */
    SCLI_PARSE_MULTI_RESULTS,           /**< Command founded with many results. */
    SCLI_PARSE_EMPTY_END                /**< Command founded with many results. Last token is empty (space at the end). */
} parse_result_t;

/**
 * @brief Private internal context of library
 */
typedef struct clima_ctx_s {
    clima_cli_print_clbk cli_print_clbk;    /**< Pointer on termina printing function. */
	clima_log_print_clbk log_print_clbk;    /**< Pointer on logs printing function. */
    clima_command_p menu_ptr;               /**< Pointer on command tree. */
    char command_buf[MAX_COMMAND_SIZE];     /**< Command buffer. */
    char exit_flag;                         /**< Exit flag. True if exit command was invoked. */
} clima_ctx_t;

/**
 * @brief Structure with parsing result.
 */
typedef struct search_result_s {
    int results;                        /**< Number of parsing results. */
    clima_command_t* result_list[MAX_SEARCH_RESULTS];   /**< List of pointer to found tokens in command tree. */
    int first_idx;                      /**< First token index on the results list. */
    char *args_hint;                    /**< Pointer to hit about arguments. */
    bool more_results;                  /**< Flas is set if results is more than list capacity. */
} search_result_t;

/**
 * @{ \name API implementation functions.
*/
clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_cli_print_clbk cli_print_clbk);
clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_print_clbk log_print_clbk);
clima_retv_t clima_set_cmds_root_impl(clima_p self, clima_command_p cmds_root);
clima_retv_t clima_is_end_impl(clima_p self);
clima_retv_t clima_check_cmd_impl(clima_p self, char* cmd);
clima_retv_t clima_exec_cmd_impl(clima_p self, const char* cmd);
/**
 * @}
 */


clima_retv_t init_clima(clima_p self)
{
	if(self == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx = clima_malloc(sizeof(*self->ctx));
    self->ctx->exit_flag = 0;

	self->set_cli_print_clbk = clima_set_cli_print_clbk_impl;
	self->set_log_print_clbk = clima_set_log_print_clbk_impl;
    self->set_cmds_root = clima_set_cmds_root_impl;
    self->check_command = clima_check_cmd_impl;
    self->exec_command = clima_exec_cmd_impl;
    self->is_end = clima_is_end_impl;

	return CLIMA_RETV_OK;
}

/**
 * @brief Implementation of api set_cli_print_clbk function.
 * 
 * @param self Pointer on library context.
 * @param cli_print_clbk Pointer on terminal printing function.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_cli_print_clbk cli_print_clbk)
{
	if(self == CLIMA_NULL || cli_print_clbk == NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx->cli_print_clbk = cli_print_clbk;

	return CLIMA_RETV_OK;
}

/**
 * @brief Implementation of api set_log_print_clbk function.
 * 
 * @param self Pointer on library context.
 * @param log_print_clbk Pointer on logs printing function.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_print_clbk log_print_clbk)
{
	if(self == CLIMA_NULL || log_print_clbk == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx->log_print_clbk = log_print_clbk;

	return CLIMA_RETV_OK;
}

/**
 * @brief Implementation of api set_cmds_root function.
 * 
 * @param self Pointer on library context.
 * @param cmds_root 
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_set_cmds_root_impl(clima_p self, clima_command_p cmds_root)
{
	if(self == CLIMA_NULL || cmds_root == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

    self->ctx->menu_ptr = cmds_root;

    return CLIMA_RETV_OK;
}

/**
 * @brief Implementation of api is_end function.
 * 
 * @param self Pointer on library context.
 * @return clima_retv_t 
 */
clima_retv_t clima_is_end_impl(clima_p self)
{
	if(self == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

    return self->ctx->exit_flag;
}

/**
 * @brief Check token is a part of command.
 * 
 * @param command Pointer on command buffer.
 * @param token Pointer on token name string.
 * @return clima_bool_t Returns true if token contain begin of command.
 */
clima_bool_t clima_is_start_with(const char* command, const char* token)
{
    int ch_count=0;
    
    while(!IS_CLIMA_END_CHAR(token[ch_count]) && (command[ch_count]!=0)) {
        if(command[ch_count] != token[ch_count]) {
            return CLIMA_FALSE;
        }
        ch_count++;
    }

    return CLIMA_TRUE;
}

/**
 * @brief Function finds all mached comman tokens in tree branch.
 * 
 * @param token Token to check.
 * @param menu_ptr Branch of commands tree.
 * @param result Result of serch.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_find_cmds(const char* token, clima_command_t *menu_ptr, search_result_t* result)
{
    int idx=0;
    if(menu_ptr == CLIMA_NULL || result == CLIMA_NULL) {
        return CLIMA_RETV_ERR;
    }

    if(token == CLIMA_NULL) {
        token = "";
    }

    result->results = 0;
    result->result_list[0] = 0;
    result->first_idx = 0;
    result->more_results = false;

    while(menu_ptr->cmd) {
        if(CLIMA_TRUE == clima_is_start_with(menu_ptr->cmd, token)) {
            result->result_list[result->results]=menu_ptr;           
            result->results++;
            
            if(!result->first_idx) {
                result->first_idx = idx;
            }

            if(result->results == MAX_SEARCH_RESULTS) {
                result->more_results = true;
                break;
            }
        }
        menu_ptr++;
        idx++;
    } 

    return CLIMA_RETV_OK;
}

/**
 * @brief Function founds next token in command.
 * 
 * @param token Command to search
 * @return const char* Pointer on begin of next token or null if it is last token.
 */
const char* clima_find_next_token(const char* token)
{
    int ch_count=0;
    if(token == CLIMA_NULL) {
        return CLIMA_NULL;
    }

    while(!IS_CLIMA_END_CHAR(token[ch_count])) {
        ch_count++;
    }

    if(token[ch_count]=='\0') {
        return CLIMA_NULL;
    }

    return token + ch_count + 1;
}

/**
 * @brief Print hints based on search result
 * 
 * @param ctx Pointer on library private context.
 * @param search_result Structure with search result. @see search_result_s
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_print_hints(clima_ctx_p ctx, const search_result_t search_result)
{
    if(ctx == CLIMA_NULL || ctx->cli_print_clbk == CLIMA_NULL) {
        return CLIMA_RETV_ERR;
    }  

    for(int i=0; i<search_result.results; i++) {
		ctx->cli_print_clbk(CLIMA_NEW_LINE CLIMA_TAB);
        ctx->cli_print_clbk(search_result.result_list[i]->cmd);
        if(search_result.result_list[i]->hint) {
			ctx->cli_print_clbk(" - ");
            ctx->cli_print_clbk(search_result.result_list[i]->hint);
        }
    }

    if(search_result.more_results == true) {
        ctx->cli_print_clbk(CLIMA_NEW_LINE CLIMA_TAB);
        ctx->cli_print_clbk(CLIMA_MORE_MESSAGE);
    }

	ctx->cli_print_clbk(CLIMA_NEW_LINE);

	return CLIMA_RETV_OK;
}

/**
 * @brief Function adds source string to destinaton string.
 * 
 * @param dest_str Destination string.
 * @param source_str Source string.
 */
void clima_addstr(char* dest_str, char* source_str)
{
    int dest_ch_count = 0;
    int source_ch_count = 0;

    while(dest_str[dest_ch_count]!='\0') {
        dest_ch_count++;
    }

    while(source_str[source_ch_count]!='\0') {
        dest_str[dest_ch_count++] = source_str[source_ch_count++];
    }

    dest_str[dest_ch_count]='\0';
}

/**
 * @brief Function completes the last command token.
 * 
 * @param cmd Command string to complelation.
 * @param full_token Token string.
 */
void clima_completion(char* cmd, char* full_token)
{
    int cmd_ch_count = 0;
    int token_ch_count = 0;

    while(cmd[cmd_ch_count]!='\0') {
        cmd_ch_count++;
    }

    while(cmd[cmd_ch_count]!=' ' && cmd_ch_count!=0) {
        cmd_ch_count--;
    }

    if(cmd[cmd_ch_count]==' ') {
        cmd_ch_count++;
    }

    while(full_token[token_ch_count]!='\0') {
        cmd[cmd_ch_count++] = full_token[token_ch_count++];
    }

    cmd[cmd_ch_count] = '\0';
}

/**
 * @brief Function founds the common part of results.
 * 
 * @param search_result Structure with search result. @see search_result_s
 * @param common_part Common part string pointer.
 */
void clima_common_part(const search_result_t search_result, char* common_part)
{
    int ch_count = 0;
    int result_count=0;
    char ch;

    do {
        if(search_result.result_list[result_count]->cmd[ch_count] == 0) {
            break;
        }

        if(result_count == 0) {
            ch = search_result.result_list[result_count]->cmd[ch_count];
            result_count++;
            continue;
        }

        if(ch != search_result.result_list[result_count]->cmd[ch_count]) {
            break;
        }

        result_count++;

        if(result_count == search_result.results) {
            common_part[ch_count] = search_result.result_list[0]->cmd[ch_count];
            ch_count++;
            result_count=0;
        }
    } while(true);

    common_part[ch_count] = '\0';
}

/**
 * @brief Function checks if the string ends with space. 
 * 
 * @param str String to check.
 * @return int TODO change on bool
 */
int clima_is_ending_space(const char* str)
{
    int ch_count=0;
    while(str[ch_count]!='\0') {
        ch_count++;
    }

    if(ch_count>0) {
        if(str[ch_count-1]==' ') {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Function parses command.
 * 
 * @param ctx Pointer on library private context.
 * @param cmd Command string to parsing.
 * @param search_result Result of parsing. @see search_result_s
 * @param cmd_struct Structure of command. @see cmd_struct_s
 * @return parse_result_t Status of parsing. @see parse_result_s
 */
parse_result_t clima_parse_cmd(clima_ctx_p ctx, const char* cmd, search_result_t* search_result, cmd_struct_t* cmd_struct)
{
    const char *token;
	const char *next_token;
    clima_command_p menu_ptr = ctx->menu_ptr;

    if(ctx->menu_ptr == CLIMA_NULL) {
        return SCLI_PARSE_NO_RESULTS; 
    }

    if(cmd == NULL || search_result == NULL) {
        return SCLI_PARSE_ERROR;
    }
  
    search_result->results = 0;
    search_result->args_hint = 0;
    if(cmd_struct) {
        cmd_struct->tokens_count = 0;
        cmd_struct->args = 0;
    }
    next_token = token = cmd;

    while(token) {
		/* get next token */
        next_token = clima_find_next_token(next_token);

        if(menu_ptr == 0 && token) {
            if(cmd_struct) {
                cmd_struct->args = token;
            }
            return SCLI_PARSE_SINGLE_RESULT_ARGS;
        }

        clima_find_cmds(token, menu_ptr, search_result);

        if(search_result->results == 0 || (search_result->results>1 && next_token)) {
            return SCLI_PARSE_NO_RESULTS;
        }

        menu_ptr = search_result->result_list[0];
        if(cmd_struct) {
            cmd_struct->token_idx[cmd_struct->tokens_count] = search_result->first_idx;
            cmd_struct->tokens_count++;
        }

		if(menu_ptr->next_cmd) {
			/* set pointer to new menu if exists */
			menu_ptr = menu_ptr->next_cmd;
		} else {
            search_result->args_hint = menu_ptr->args_hint;
            menu_ptr = 0;
        }

		token = next_token;
    }

    if(clima_is_ending_space(cmd) && menu_ptr) {
        return SCLI_PARSE_EMPTY_END;
    }

    if(search_result->results == 1) {
        return SCLI_PARSE_SINGLE_RESULT;
    }

    return SCLI_PARSE_MULTI_RESULTS;
}

/**
 * @brief Implementation of api check_command function.
 * 
 * @param self Pointer on library context.
 * @param cmd Pointer on command string.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_check_cmd_impl(clima_p self, char* cmd)
{
	clima_ctx_p ctx = self->ctx;
    search_result_t search_result={0};
    char common_part[MAX_COMMAND_SIZE];
    parse_result_t parse_ret = clima_parse_cmd(ctx, cmd, &search_result, CLIMA_NULL);

    switch(parse_ret) {
        case SCLI_PARSE_ERROR:
            return CLIMA_RETV_ERR;

        case SCLI_PARSE_MULTI_RESULTS:
            clima_print_hints(ctx, search_result);
            clima_common_part(search_result, common_part);
            clima_completion(cmd, common_part);
            break;

        case SCLI_PARSE_EMPTY_END:
            if(search_result.results>1) {
                clima_print_hints(ctx, search_result);
                clima_common_part(search_result, common_part);
                clima_completion(cmd, common_part);
                break;  
            }
            /* else continue in SCLI_PARSE_SINGLE_RESULT */

        case SCLI_PARSE_SINGLE_RESULT:
            clima_completion(cmd, search_result.result_list[0]->cmd);
            if(search_result.result_list[0]->args_hint) {
                clima_addstr(cmd, " ");
            }

        case SCLI_PARSE_SINGLE_RESULT_ARGS:
            if(search_result.result_list[0]->next_cmd) {
                clima_addstr(cmd, " ");
            } else {
                if(search_result.args_hint) {
                    ctx->cli_print_clbk(CLIMA_NEW_LINE CLIMA_NEW_LINE CLIMA_TAB);
                    ctx->cli_print_clbk(search_result.args_hint);
                    ctx->cli_print_clbk(CLIMA_ENTER_MESSAGE CLIMA_NEW_LINE);
                } else {
                    ctx->cli_print_clbk(CLIMA_NEW_LINE CLIMA_NEW_LINE CLIMA_TAB CLIMA_ENTER_MESSAGE CLIMA_NEW_LINE);
                }
            }
            ctx->cli_print_clbk(CLIMA_NEW_LINE); 
            break;

        case SCLI_PARSE_NO_RESULTS:
            ctx->cli_print_clbk(CLIMA_NEW_LINE);
            break;
    }
    return CLIMA_RETV_OK;
}

/**
 * @brief Implementation of api exec_command function.
 * 
 * @param self Pointer on library context.
 * @param cmd Pointer on command string.
 * @return clima_retv_t Error code: CLIMA_RETV_OK if succes or CLIMA_RETV_ERR if error.
 */
clima_retv_t clima_exec_cmd_impl(clima_p self, const char* cmd)
{
	clima_ctx_p ctx = self->ctx;
    search_result_t search_result;
    cmd_struct_t cmd_struct;
    parse_result_t parse_ret = clima_parse_cmd(ctx, cmd, &search_result, &cmd_struct);

    switch(parse_ret) {
        case SCLI_PARSE_SINGLE_RESULT_ARGS:
        case SCLI_PARSE_SINGLE_RESULT:
            if(!search_result.result_list[0]->fn_handler) {
                return CLIMA_RETV_ERR;
            }
            if(CLIMA_CMD_EXIT == search_result.result_list[0]->fn_handler(cmd, &search_result.result_list[0], &cmd_struct, ctx->cli_print_clbk)) {
                ctx->exit_flag = 1;
            }
            break;

        default:
            return CLIMA_RETV_ERR;
    }

    return CLIMA_RETV_OK;
}

/**
 * @brief Default definition of memory allocation function.
 * 
 * @param size in bytes
 * @return void* pointer to buffer
 */
__WEAK void* clima_malloc(clima_size_t size) {
	return malloc(size);
}

/**
 * @brief Default definition of memory allocation function.
 * 
 * @param size in bytes
 * @return void* pointer to buffer
 */
__WEAK void clima_free(void* ptr) {
	return free(ptr);
}
