#ifndef CLIMA_CMDS_H_
#define CLIMA_CMDS_H_

typedef void (*clima_cli_print_clbk)(const char* string);

struct  clima_command_s;

typedef struct cmd_struct_s {
    int token_idx[16];
    int tokens_count;
	const char *args;
} cmd_struct_t;
typedef cmd_struct_t* cmd_struct_p;

typedef enum clima_bool_e {
	CLIMA_FALSE = 0,
	CLIMA_TRUE = 1
} clima_bool_t;

typedef enum clima_cmd_fn_retv_e {
	CLIMA_CMD_ERR,
	CLIMA_CMD_OK,
	CLIMA_CMD_EXIT
} clima_cmd_fn_retv_t;

typedef int (*fn_handler_t) (const char *pbCmd, struct clima_command_s **psCmdTab, cmd_struct_t* cmd_struct, clima_cli_print_clbk cprint);

typedef struct clima_command_s
{
	char *cmd;
    char *hint;
	char *args_hint;
	struct clima_command_s *next_cmd;
	fn_handler_t fn_handler;
} clima_command_t, *clima_command_p;

#endif
