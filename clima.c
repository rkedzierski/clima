#include "clima.h"
#include "clima_config.h"

#define CLIMA_NULL 0

typedef enum clima_bool_e {
    CLIMA_FALSE = 0,
    CLIMA_TRUE = 1
} clima_bool_t;

struct clima_ctx_s {
    clima_cli_print_clbk cli_print_clbk;
	clima_log_print_clbk log_print_clbk;
    char command_buf[MAX_COMMAND_SIZE];
};

clima_retv_t clima_set_cli_print_clbk_impl(clima_p self, clima_print_clbk cli_print_clbk);
clima_retv_t clima_set_log_print_clbk_impl(clima_p self, clima_log_clbk log_print_clbk);

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