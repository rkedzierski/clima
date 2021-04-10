#include "clima.h"
#include "clima_config.h"
#include "clima_cmds.h"

#define CLIMA_NULL 0

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
    char command_buf[MAX_COMMAND_SIZE];
} clima_ctx_t;
typedef clima_ctx_t* clima_ctx_p;

typedef struct search_result_s {
    int results;
    scli_command_t* result_list[16];
} search_result_t;

clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_cli_print_clbk cli_print_clbk);
clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_print_clbk log_print_clbk);

clima_retv_t init_clima(clima_p self)
{
	if(self == CLIMA_NULL) {
		return CLIMA_RETV_ERR;
	}

	self->ctx = clima_malloc(sizeof(self->ctx));

	self->set_cli_print_clbk = clima_set_cli_print_clbk_impl;
	self->set_log_print_clbk = clima_set_log_print_clbk_impl;

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

clima_bool_t clima_is_start_with(const char* command, const char* token)
{
    int ch_count=0;
    
    while(token[ch_count]!=0) {
        if(command[ch_count] != token[ch_count]) {
            return CLIMA_FALSE;
        }
        ch_count++;
    }

    return CLIMA_TRUE;
}

clima_retv_t clima_find_cmds(char* token, scli_command_t *menu_ptr, search_result_t* result)
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
        if(CLIMA_TRUE == scli_is_start_with(menu_ptr->cmd, token)) {
            result->result_list[result->results]=menu_ptr;           
            result->results++;
        }
        menu_ptr++;
    } 

    return CLIMA_RETV_OK;
}

clima_retv_t clima_print_hints(clima_ctx_p ctx, const search_result_t search_result)
{
    int rc=0, cc=0; 
    if(ctx == CLIMA_NULL || ctx->cli_print_clbk == CLIMA_NULL) {
        return CLIMA_RETV_ERR;
    }  

    for(int i=0; i<search_result.results; i++) {
		ctx->cli_print_clbk("\n\t");
        cc=0;
        while(search_result.result_list[i]->cmd[cc] != '\0') {
            ctx->cli_print_clbk(search_result.result_list[i]->cmd[cc++]);
        }
        cc=0;
        if(search_result.result_list[i]->hint) {
			ctx->cli_print_clbk(" - ");
            while(search_result.result_list[i]->hint[cc] != '\0') {
                ctx->cli_print_clbk(search_result.result_list[i]->hint[cc++]);
            }
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

parse_result_t clima_parse_cmd(char* cmd, search_result_t* search_result)
{
    scli_command_t *menu_ptr = asMainCmd;
    char *token;
	char *next_token;
	char *saveptr;
    //search_result_t search_result;
    char tmp_buff[MAX_COMMAND_SIZE];

    if(cmd == NULL || search_result == NULL) {
        return SCLI_PARSE_ERROR;
    }
    
    search_result->results = 0;

    strncpy(tmp_buff, cmd, MAX_COMMAND_SIZE);
    token = strtok_r(tmp_buff, "\t ", &saveptr);

    if(token == NULL) {
        find_cmd("", menu_ptr, search_result);
        return SCLI_PARSE_EMPTY_END;
    }

    while(token) {
		/* get next token */
		next_token = strtok_r(NULL, "\t ", &saveptr);

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
    parse_result_t parse_ret = clima_parse_cmd(cmd, &search_result);

    switch(parse_ret) {
        case SCLI_PARSE_ERROR:
            return 1;

        case SCLI_PARSE_EMPTY_END:
        case SCLI_PARSE_MULTI_RESULTS:
            clima_print_hints(ctx, search_result);
            //clima_scli_addstr(result, "\n");
            ctx->cli_print_clbk(cmd);    
            break;

        case SCLI_PARSE_SINGLE_RESULT:
            clima_scli_completion(cmd, search_result.result_list[0]->cmd);

            if(search_result.result_list[0]->next_cmd) {
                clima_scli_addstr(cmd, " ");
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

int clima_exec_cmd(char* cmd, char* result)
{
    search_result_t search_result;
    parse_result_t parse_ret = parse_cmd(cmd, &search_result);

    switch(parse_ret) {
        case SCLI_PARSE_SINGLE_RESULT:
            if(!search_result.result_list[0]->fn_handler) {
                return 1;
            }
            search_result.result_list[0]->fn_handler(cmd, &search_result.result_list[0]);
            break;

        default:
            return 1;
    }

    return 0;
}