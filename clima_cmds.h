#ifndef CLIMA_CMDS_H_
#define CLIMA_CMDS_H_

//#define NULL 0

struct  scli_command_s;

typedef enum clima_bool_e {
	CLIMA_FALSE = 0,
	CLIMA_TRUE = 1
} clima_bool_t;

typedef int (*fn_handler_t) (char *pbCmd, struct scli_command_s **psCmdTab);

typedef struct scli_command_s
{
	char *cmd;
    char *hint;
	struct scli_command_s *next_cmd;
	fn_handler_t fn_handler;
	int cmd_idx;
} scli_command_t;

extern scli_command_t asMainCmd[];

#endif