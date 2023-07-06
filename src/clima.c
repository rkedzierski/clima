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

#ifndef   __WEAK
  #define __WEAK                                 __attribute__((weak))
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
    clima_command_t* result_list[16];   /**< List of pointer to found tokens in command tree. */
    int first_idx;                      /**< First token index on the results list. */
    char *args_hint;                    /**< Pointer to hit about arguments. */
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

    while(menu_ptr->cmd) {
        idx++;
        if(CLIMA_TRUE == clima_is_start_with(menu_ptr->cmd, token)) {
            result->result_list[result->results]=menu_ptr;           
            result->results++;
            
            if(!result->first_idx) {
                result->first_idx = idx;
            }
        }
        menu_ptr++;
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
	ctx->cli_print_clbk(CLIMA_NEW_LINE);

	return CLIMA_RETV_OK;
}

/**
 * @brief Function adds source string to destinaton string.
 * 
 * @param dest Destination string.
 * @param src Source string.
 */
void clima_addstr(char* dest, char* src)
{
    int dc=0, sc=0;
    while(dest[dc]!='\0') {dc++;}
    while(src[sc]!='\0') {
        dest[dc++] = src[sc++];
    }
    dest[dc]='\0';
}

/**
 * @brief Function completes the last command token.
 * 
 * @param cmd Command string to complelation.
 * @param full_token Token string.
 */
void clima_completion(char* cmd, char* full_token)
{
    int cc=0, tc=0;
    while(cmd[cc]!='\0') {
        cc++;
    }
    while(cmd[cc]!=' ' && cc!=0) {
        cc--;
    }
    if(cmd[cc]==' ') {
        cc++;
    }
    while(full_token[tc]!='\0') {
        cmd[cc++] = full_token[tc++];
    }
    cmd[cc] = '\0';
}

/**
 * @brief Function checks if the string ends with space. 
 * 
 * @param str String to check.
 * @return int TODO change on bool
 */
int clima_is_ending_space(const char* str)
{
    int c=0;
    while(str[c]!='\0') {
        c++;
    }

    if(c>0) {
        if(str[c-1]==' ') {
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
            //printf("SCLI_PARSE_SINGLE_RESULT_ARGS\n");
            return SCLI_PARSE_SINGLE_RESULT_ARGS;
        }

        clima_find_cmds(token, menu_ptr, search_result);
        //printf("\nFind %d results.\n", search_result.results);

        if(search_result->results == 0 || (search_result->results>1 && next_token)) {
            //printf("SCLI_PARSE_NO_RESULTS\n");
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
        //printf("SCLI_PARSE_EMPTY_END"CLIMA_NEW_LINE);
        return SCLI_PARSE_EMPTY_END;
    }

    if(search_result->results == 1) {
        //printf("SCLI_PARSE_SINGLE_RESULT"CLIMA_NEW_LINE);
        return SCLI_PARSE_SINGLE_RESULT;
    }
    //printf("SCLI_PARSE_MULTI_RESULTS"CLIMA_NEW_LINE);
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
    parse_result_t parse_ret = clima_parse_cmd(ctx, cmd, &search_result, CLIMA_NULL);

    //printf("clima_check_cmd parse_ret = %d", parse_ret);
    switch(parse_ret) {
        case SCLI_PARSE_ERROR:
            return CLIMA_RETV_ERR;

        case SCLI_PARSE_MULTI_RESULTS:
            clima_print_hints(ctx, search_result);
            //ctx->cli_print_clbk(cmd);
            break;

        case SCLI_PARSE_EMPTY_END:
            if(search_result.results>1) {
                clima_print_hints(ctx, search_result);
                //ctx->cli_print_clbk(cmd);
                break;  
            }
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
            //ctx->cli_print_clbk(cmd);
            
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
