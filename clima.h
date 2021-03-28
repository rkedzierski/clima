#ifndef CLIMA_H_
#define CLIMA_H_

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

typedef clima_retv_t (*clima_cli_print_clbk)(const char* string);

typedef clima_retv_t (*clima_log_print_clbk)(const char* string);


typedef clima_retv_t (*clima_set_cli_print_clbk)(clima_p self, clima_cli_print_clbk cli_print_clbk);

typedef clima_retv_t (*clima_set_log_print_clbk)(clima_p self, clima_log_print_clbk log_print_clbk);

typedef clima_retv_t (*clima_put_char)(clima_p self, char ch);

clima_retv_t init_clima(clima_p self);

struct clima_s {
    clima_ctx_p ctx;
    clima_set_cli_print_clbk set_cli_print_clbk;
	clima_set_log_print_clbk set_log_print_clbk;
    clima_put_char put_char;
};

#endif //CLIMA_H_