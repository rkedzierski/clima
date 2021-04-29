#ifndef CLIMA_CMDS_H_
#define CLIMA_CMDS_H_

//#define NULL 0

struct  clima_command_s;

typedef enum clima_bool_e {
	CLIMA_FALSE = 0,
	CLIMA_TRUE = 1
} clima_bool_t;

typedef enum clima_cmd_fn_retv_e {
	CLIMA_CMD_ERR,
	CLIMA_CMD_OK,
	CLIMA_CMD_EXIT
} clima_cmd_fn_retv_t;

typedef int (*fn_handler_t) (char *pbCmd, struct clima_command_s **psCmdTab);

typedef struct clima_command_s
{
	char *cmd;
    char *hint;
	struct clima_command_s *next_cmd;
	fn_handler_t fn_handler;
	int cmd_idx;
} clima_command_t, *clima_command_p;

//extern scli_command_t asMainCmd[];

#endif