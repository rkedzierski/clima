#include "clima.h"
#include "clima_config.h"
#include "clima_cmds.h"

#include <string.h>
#include <stdio.h>

#define CLIMA_NULL 0

#define IS_CLIMA_END_CHAR(ch) ((ch=='\0') || (ch==' '))

//typedef enum clima_bool_e {
 //   CLIMA_FALSE = 0,
 //   CLIMA_TRUE = 1
//} clima_bool_t;

typedef enum parse_result_e {
    SCLI_PARSE_ERROR,
    SCLI_PARSE_NO_RESULTS,
    SCLI_PARSE_SINGLE_RESULT,
    SCLI_PARSE_MULTI_RESULTS,
    SCLI_PARSE_EMPTY_END
} parse_result_t;

typedef struct clima_ctx_s {
    clima_cli_print_clbk cli_print_clbk;
	clima_log_print_clbk log_print_clbk;
    clima_command_p menu_ptr;
    char command_buf[MAX_COMMAND_SIZE];
    char exit_flag;
} clima_ctx_t;
typedef clima_ctx_t* clima_ctx_p;

typedef struct search_result_s {
    int results;
    clima_command_t* result_list[16];
} search_result_t;

clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_cli_print_clbk cli_print_clbk);
clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_print_clbk log_print_clbk);
clima_retv_t clima_set_cmds_root_impl(clima_p self, clima_command_p cmds_root);
clima_retv_t clima_put_char_impl(clima_p self, char ch);
clima_retv_t clima_is_end_impl(clima_p self);

clima_retv_t init_clima(clima_p self)
{
	if(self == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx = clima_malloc(sizeof(self->ctx));

	self->set_cli_print_clbk = clima_set_cli_print_clbk_impl;
	self->set_log_print_clbk = clima_set_log_print_clbk_impl;
    self->set_cmds_root = clima_set_cmds_root_impl;
    self->put_char = clima_put_char_impl;
    self->is_end = clima_is_end_impl;

	return CLIMA_RETV_OK;
}

clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_cli_print_clbk cli_print_clbk)
{
	if(self == CLIMA_NULL || cli_print_clbk == NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx->cli_print_clbk = cli_print_clbk;

	return CLIMA_RETV_OK;
}

clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_print_clbk log_print_clbk)
{
	if(self == CLIMA_NULL || log_print_clbk == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx->log_print_clbk = log_print_clbk;

	return CLIMA_RETV_OK;
}

clima_retv_t clima_set_cmds_root_impl(clima_p self, clima_command_p cmds_root)
{
	if(self == CLIMA_NULL || cmds_root == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

    self->ctx->menu_ptr = cmds_root;

    return CLIMA_RETV_OK;
}

clima_retv_t clima_is_end_impl(clima_p self)
{
	if(self == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

    return self->ctx->exit_flag;
}

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

clima_retv_t clima_find_cmds(char* token, clima_command_t *menu_ptr, search_result_t* result)
{
    if(menu_ptr == CLIMA_NULL || result == CLIMA_NULL) {
        return CLIMA_RETV_ERR;
    }

    if(token == CLIMA_NULL) {
        token = "";
    }

    result->results=0;
    result->result_list[0]=0;

    while(menu_ptr->cmd) {
        if(CLIMA_TRUE == clima_is_start_with(menu_ptr->cmd, token)) {
            result->result_list[result->results]=menu_ptr;           
            result->results++;
        }
        menu_ptr++;
    } 

    return CLIMA_RETV_OK;
}

char* clima_find_next_token(char* token)
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

clima_retv_t clima_print_hints(clima_ctx_p ctx, const search_result_t search_result)
{
    if(ctx == CLIMA_NULL || ctx->cli_print_clbk == CLIMA_NULL) {
        return CLIMA_RETV_ERR;
    }  

    for(int i=0; i<search_result.results; i++) {
		ctx->cli_print_clbk("\n\t");
        ctx->cli_print_clbk(search_result.result_list[i]->cmd);
        if(search_result.result_list[i]->hint) {
			ctx->cli_print_clbk(" - ");
            ctx->cli_print_clbk(search_result.result_list[i]->hint);
        }
    }
	ctx->cli_print_clbk("\n");

	return CLIMA_RETV_OK;
}

void clima_addstr(char* dest, char* src)
{
    int dc=0, sc=0;
    while(dest[dc]!='\0') {dc++;}
    while(src[sc]!='\0') {
        dest[dc++] = src[sc++];
    }
    dest[dc]='\0';
}

int clima_completion(char* cmd, char* full_token)
{
    int ret=0;

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
        if(cmd[cc] != full_token[tc]) {
            ret = 1;
        }
        cmd[cc++] = full_token[tc++];
    }
    return ret;
}

int clima_is_ending_space(const char* s)
{
    int c=0;
    while(s[c]!='\0') {
        c++;
    }

    if(c>0) {
        if(s[c-1]==' ') {
            return 1;
        }
    }
    return 0;
}

parse_result_t clima_parse_cmd(clima_ctx_p ctx, char* cmd, search_result_t* search_result)
{
    char *token;
	char *next_token;
    clima_command_p menu_ptr = ctx->menu_ptr;

    if(ctx->menu_ptr == CLIMA_NULL) {
        return SCLI_PARSE_NO_RESULTS; 
    }

    if(cmd == NULL || search_result == NULL) {
        return SCLI_PARSE_ERROR;
    }
    
    search_result->results = 0;
    next_token = token = cmd;

    while(token) {
		/* get next token */
        next_token = clima_find_next_token(next_token);

        clima_find_cmds(token, menu_ptr, search_result);
        //printf("\nFind %d results.\n", search_result.results);

        if(search_result->results == 0 || (search_result->results>1 && next_token)) {
            return SCLI_PARSE_NO_RESULTS;
        }

        menu_ptr = search_result->result_list[0];

		if(menu_ptr->next_cmd) {
			/* set pointer to new menu if exists */
			menu_ptr = menu_ptr->next_cmd;
		}

		token = next_token;
    }

    if(clima_is_ending_space(cmd)) {
        clima_find_cmds(token, menu_ptr, search_result);
        return SCLI_PARSE_EMPTY_END;
    }

    if(search_result->results == 1) {
        return SCLI_PARSE_SINGLE_RESULT;
    }
    return SCLI_PARSE_MULTI_RESULTS;
}

int clima_check_cmd(clima_ctx_p ctx, char* cmd)
{
    search_result_t search_result={0};
    parse_result_t parse_ret = clima_parse_cmd(ctx, cmd, &search_result);

    switch(parse_ret) {
        case SCLI_PARSE_ERROR:
            return 1;

        case SCLI_PARSE_MULTI_RESULTS:
            clima_print_hints(ctx, search_result);
            ctx->cli_print_clbk(cmd);    
            break;

        case SCLI_PARSE_EMPTY_END:
            if(search_result.results>1) {
                clima_print_hints(ctx, search_result);
                ctx->cli_print_clbk(cmd);
                break;  
            }
        case SCLI_PARSE_SINGLE_RESULT:
            clima_completion(cmd, search_result.result_list[0]->cmd);

            if(search_result.result_list[0]->next_cmd) {
                clima_addstr(cmd, " ");
            } else {
                ctx->cli_print_clbk("\n\n\t[ENTER]\n");
            }
            ctx->cli_print_clbk("\n");
            ctx->cli_print_clbk(cmd);
            
            break;

        case SCLI_PARSE_NO_RESULTS:
            break;
    }
    return 0;
}

int clima_exec_cmd(clima_ctx_p ctx, char* cmd)
{
    search_result_t search_result;
    parse_result_t parse_ret = clima_parse_cmd(ctx, cmd, &search_result);

    switch(parse_ret) {
        case SCLI_PARSE_SINGLE_RESULT:
            if(!search_result.result_list[0]->fn_handler) {
                return 1;
            }
            if(CLIMA_CMD_EXIT == search_result.result_list[0]->fn_handler(cmd, &search_result.result_list[0])) {
                ctx->exit_flag = 1;
            }
            break;

        default:
            return 1;
    }

    return 0;
}

#define CLIMA_KEY_ESC    27
#define CLIMA_KEY_TAB    9
#define CLIMA_KEY_ENTER  10
#define CLIMA_KEY_BACKSPACE 127

clima_retv_t clima_put_char_impl(clima_p self, char ch)
{
    static char tout[256];
    static char tmp[260];
    char putc[2] = "x\0";

    //printf("process_terminal teminal_in=%d\n",teminal_in);

    switch(ch) {
        case CLIMA_KEY_BACKSPACE:
            if(tout[0]=='\0') break;
            tout[strlen(tout)-1]='\0';
            self->ctx->cli_print_clbk("\b \b");
            //sprintf(terminal_out, "\b \b");
            break;
        case CLIMA_KEY_TAB:
            if(clima_check_cmd(self->ctx, tout)) {
                self->ctx->cli_print_clbk("\nERROR!\n");
            }           
            break;
        case CLIMA_KEY_ESC:
            break;
        case CLIMA_KEY_ENTER:
            //sprintf(terminal_out, "\nCMD: %s\n", tout);
            //printf("test");
            //if(parse_cmd2(tout, terminal_out, SCLI_TRUE)) {
            //    sprintf(terminal_out, "\nERROR!\n");
            //}
            if(clima_exec_cmd(self->ctx, tout)) {
                self->ctx->cli_print_clbk("\nERROR!\n");
            }   
            tout[0]='\0';
            break;
        default:
            sprintf(tmp, "%s%c", tout, ch);
            strncpy(tout, tmp, 256);
            putc[0] = ch;
            self->ctx->cli_print_clbk(putc);
            //sprintf(terminal_out, "%c", teminal_in);
            break;
    }

    
    return CLIMA_RETV_OK;
}
