#include "config.h"
#include <cargs.h>

static struct cag_option options[] = {
    // {
    //     .identifier = 's',
    //     .access_letters = "s",
    //     .access_name = NULL,
    //     .value_name = NULL,
    //     .description = "Simple flag"
    // },
    // {
    //     .identifier = 'm',
    //     .access_letters = "mMoO",
    //     .access_name = NULL,
    //     .value_name = NULL,
    //     .description = "Multiple access letters"
    // },
    {
        .identifier = 'p',
        .access_letters = NULL,
        .access_name = "pty",
        .value_name = NULL,
        .description = "Open pty only"
    },
    {
        .identifier = 'o',
        .access_letters = "o",
        .access_name = "opacity",
        .value_name = "VALUE",
        .description = "Cell opacity"
    },
    {
        .identifier = 'q',
        .access_letters = "q",
        .access_name = "quiet",
        .value_name = "VALUE",
        .description = "Run without output"
    },
    {
        .identifier = 'f',
        .access_letters = "f",
        .access_name = NULL,
        .value_name = "VALUE",
        .description = "Read from file"
    },
    {
        .identifier = 'h',
        .access_letters = "h",
        .access_name = "help",
        .description = "Shows the command help"
    },
};

void config_args_parse(term_config_t * conf, int argc, char **argv)
{
    int param_index;
    int additional_param_index = 0;
    int additional_param_count = 0;
    const char *value = NULL;


    conf->command_argv = NULL;
    conf->file_path = NULL;
    conf->pty_only = false;
    conf->quiet = false;

    cag_option_context context;
    cag_option_init(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context)) {
        switch (cag_option_get_identifier(&context)) {
        case 'p':
            conf->pty_only = true;
            break;
        case 'q':
            conf->quiet = true;
            break;
        case 'f':
            conf->file_path = cag_option_get_value(&context);
            break;
        case 'o':
            value = cag_option_get_value(&context);
            conf->opa = atoi(value);
            break;
        case 'h':
            printf("Usage: cargsdemo [OPTION]...\n");
            printf("Demonstrates the cargs library.\n\n");
            cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
            printf("\nNote that all formatting is done by cargs.\n");
            // return EXIT_SUCCESS;
        case '?':
            cag_option_print_error(&context, stdout);
            break;
        }
    }

    param_index = cag_option_get_index(&context);
    additional_param_count = argc - param_index;

    LV_LOG_TRACE("additional parameter count: %d\n", additional_param_count);

    // +1 for NULL at end
    conf->command_argv = lv_malloc_zeroed(sizeof(char*) * (additional_param_count + 1));
    // conf->command_argv[additional_param_count] = NULL

    for (; param_index < argc; ++param_index)
    {
        LV_LOG_TRACE("additional parameter: %s\n", argv[param_index]);
        conf->command_argv[additional_param_index] = argv[param_index];
        additional_param_index++;
    }

    conf->command_argc = additional_param_index;
}