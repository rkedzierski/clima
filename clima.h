#ifndef __CLIMA_H__
#define __CLIMA_H__

#include "clima_cmds.h"

typedef enum clima_retv_e {
    CLIMA_RETV_OK,
    CLIMA_RETV_ERR,
} clima_retv_t;

struct clima_ctx_s;
typedef struct clima_ctx_s clima_ctx_t;
typedef struct clima_ctx_s* clima_ctx_p;

struct clima_s;
typedef struct clima_s clima_t;
typedef struct clima_s* clima_p;


typedef clima_retv_t (*clima_log_print_clbk)(const char* string);

typedef clima_retv_t (*clima_set_cli_print_clbk)(clima_p self, clima_cli_print_clbk cli_print_clbk);

typedef clima_retv_t (*clima_set_log_print_clbk)(clima_p self, clima_log_print_clbk log_print_clbk);

typedef clima_retv_t (*clima_set_cmds_root)(clima_p self, clima_command_p cmds_root);

typedef clima_retv_t (*clima_is_end)(clima_p self);

typedef clima_retv_t (*clima_check_cmd)(clima_p self, char* cmd);

typedef clima_retv_t (*clima_exec_cmd)(clima_p self, const char* cmd);

clima_retv_t init_clima(clima_p self);

struct clima_s {
    clima_ctx_p ctx;
    clima_set_cli_print_clbk set_cli_print_clbk;
	clima_set_log_print_clbk set_log_print_clbk;
    clima_set_cmds_root set_cmds_root;
    clima_check_cmd check_command;
    clima_exec_cmd exec_command;
    clima_is_end is_end;
};

#endif /* __CLIMA_H__ */
