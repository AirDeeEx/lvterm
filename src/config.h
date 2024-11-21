#ifndef LV_TERM_CONFIG_H
#define LV_TERM_CONFIG_H

#include <stdlib.h>
#include <stdint.h>
#include <lvgl.h>


typedef struct {
    lv_opa_t opa;
    char** command_argv;
    int command_argc;
    bool pty_only;
    const char* file_path;
    bool quiet;
} term_config_t;

void config_args_parse(term_config_t * conf, int argc, char **argv);

#endif