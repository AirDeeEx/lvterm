
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <lvgl.h>
#include <stdio.h>
#include "src/widgets/terminal/lv_terminal.h"
#include "src/term_io.h"
#include "src/config.h"
#include "font.h"
#include <term_hal.h>
#include <log.h>
#include <fcntl.h>
 #include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    term_pty_t * pty;
    lv_obj_t * term;
} term_event_data_t;

typedef struct {
    term_pty_t * pty;
    lv_obj_t * term;
} term_pty_event_data_t;

typedef struct {
    int source_fd;
    char *buf;
    lv_obj_t *term;
    lv_timer_t *timer;
} term_updater_data_t;

static void terminal_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    term_event_data_t * term_event_data = (term_event_data_t *)lv_event_get_user_data(e);

    lv_obj_t * term = term_event_data->term;
    term_pty_t * pty = term_event_data->pty;

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_INFO("Clicked");
        lv_terminal_clear(term);
    }
}

static void terminal_pty_event_handler(lv_event_t * e) //TODO: sptil into separate
{
    lv_event_code_t code = lv_event_get_code(e);
    term_event_data_t * term_event_data = (term_event_data_t *)lv_event_get_user_data(e);

    lv_obj_t * term = term_event_data->term;
    term_pty_t * pty = term_event_data->pty;

    if(code == lv_terminal_resize_event_id())
    {
        if (pty)
        {
            uint16_t cols = lv_terminal_get_cols(term);
            uint16_t rows = lv_terminal_get_rows(term);

            LV_LOG_INFO("Resize event: rows=%d, cols=%d", rows, cols);
            term_set_size(pty, cols, rows);
        } else {
            LV_LOG_INFO("Resize event: ignored (not pty)");
        }
        
    }
    else if(code == lv_terminal_reset_event_id())
    {
        if (pty)
        {
            LV_LOG_INFO("Reset event!");
            term_notify_resize(pty);
        } else {
             LV_LOG_INFO("Reset event: ignored (not pty)");
        }
    }
}

void term_update_timer(lv_timer_t * timer)
{
    /*Use the user_data*/
    term_updater_data_t * data = timer->user_data;
    //LV_LOG_USER("my_timer called with pty: %s", data->pty->slave_path);

    //memset(data->buf,'\0', BUFFSIZE); //TODO: dynamic buffer size
    int nread = read(data->source_fd, data->buf, BUFFSIZE);

    if (nread != -1)
    {
        LV_LOG_INFO("term_update: read: %d", nread);
        lv_terminal_parse(data->term, data->buf, nread);
    }else{
        //LV_LOG_USER("empty read");
    }
}



lv_obj_t * init_term(lv_obj_t * parent, lv_font_t * font, term_pty_t * pty)
{
    int res = -1;

    lv_obj_t * term = lv_terminal_create(parent);

    term_event_data_t event_data;
    event_data.pty = pty; //May be NULL, it's fine
    event_data.term = term;

    lv_obj_add_event_cb(term, terminal_event_handler, LV_EVENT_ALL, &event_data);

    int32_t term_w = lv_pct(100);
    int32_t term_h = lv_pct(100);
    lv_obj_set_size(term,term_w,term_h);
    //lv_obj_set_align(term, LV_ALIGN_BOTTOM_RIGHT);

    lv_obj_set_style_border_side(term,LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(term,1,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(term,lv_color_make(0,255,0),LV_PART_MAIN | LV_STATE_DEFAULT);

    //lv_obj_set_pos(term,0,0);

    // lv_obj_set_size(term, lv_pct(100),lv_pct(100));


    uint16_t cell_w, cell_h;
    if (font)
    {
        lv_terminal_set_font(term, font);
    }
    
    lv_terminal_get_font_max_bitmap(term, &cell_w, &cell_h);
    lv_terminal_set_cell_size(term, cell_w, cell_h);

    uint16_t cols = lv_terminal_get_matrix_aval_cols(term);// + 29;
    uint16_t rows = lv_terminal_get_matrix_aval_rows(term);// + 11;

    // rows += 11;
    // cols += 69;

    // rows = 1;
    // cols = 3;

    lv_terminal_set_matrix_size(term,rows,cols);
    LV_LOG_WARN("termial aval: rows=%d, cols=%d",rows,cols);
    LV_LOG_WARN("termial cell: width=%d, height=%d",cell_w,cell_h);

    return term;
}

extern char** environ;
static term_config_t g_conf;

//Declare as global for event handlers (stack-use-after-scope fix) //TODO: migrate to linux specific file
static term_pty_t g_term_pty;
static term_pty_event_data_t g_pty_event_data;
int source_file_fd;

void process_from_pty(term_updater_data_t *updater_data)
{
    int res;
    uint16_t cols, rows;

    //Open linux pty and set as reader source
    res = term_openpty(&g_term_pty);
    updater_data->source_fd = g_term_pty.master_fd;

    //Setup handlers for pty resize
    g_pty_event_data.pty = &g_term_pty;
    g_pty_event_data.term = updater_data->term;
    lv_obj_add_event_cb(updater_data->term, terminal_pty_event_handler, LV_EVENT_ALL, &g_pty_event_data);

    //Apply terminal size to pty first time (terminal_pty_event_handler do it next time)
    cols = lv_terminal_get_cols(updater_data->term);
    rows = lv_terminal_get_rows(updater_data->term);
    term_set_size(&g_term_pty, cols, rows);

    LV_LOG_WARN("Pty open: %s, res=%d", g_term_pty.slave_path, res);

    if (!g_conf.pty_only)
    {
        char* default_cmd[] = { "/usr/bin/htop", NULL };
        char** term_cmd = NULL;

        //char* term_envp[] = { NULL };

        LV_LOG_INFO("Try Fork");

        if (g_conf.command_argc > 0)
        {
            term_cmd = g_conf.command_argv;
        }
        else
        {
            LV_LOG_INFO("Use default cmd!");
            term_cmd = default_cmd;
        }
        
        term_process(&g_term_pty, term_cmd, environ); //maybe use-after-scope
    } else {
        char ansi_escape[50];
        char* text = "Pty ready!";
        int slen;

        printf("%s\n", g_term_pty.slave_path); //TODO better log system

        // lv_memset(ansi_escape, 0, sizeof(char) * 50);

        uint16_t half_col = cols/2;
        uint16_t half_row = rows/2;

        LV_LOG_INFO("col,row: %d, %d", half_col, half_row);

        slen = lv_strlen(text);

        sprintf(ansi_escape, "\e[2J\e[%d;%dH%s", half_row, half_col - slen, text);

        lv_terminal_puts(updater_data->term, ansi_escape);
    }
    
    updater_data->timer = lv_timer_create(term_update_timer, 100,  updater_data);
}

void process_from_file(term_updater_data_t *updater_data)
{
    struct stat file_status;

    source_file_fd = open(g_conf.file_path, O_RDONLY | O_NONBLOCK);
    if (source_file_fd==-1)
    {
        LV_LOG_ERROR("open() error");
        exit(EXIT_FAILURE);
    }

    if(fstat(source_file_fd, &file_status)==-1)
    {
        LV_LOG_ERROR("fstat: error");
        close(source_file_fd);
        exit(EXIT_FAILURE);
    }

    if(S_ISDIR(file_status.st_mode))
    {
        LV_LOG_ERROR("%s in a directory!\n", g_conf.file_path);
        close(source_file_fd);
        exit(EXIT_FAILURE);
    }

    updater_data->source_fd = source_file_fd;

    updater_data->timer = lv_timer_create(term_update_timer, 100,  updater_data);
}

#include <time.h>

#if LV_USE_PROFILER

static FILE *fptr;

static uint32_t my_get_tick_us_cb(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void my_profiler_flush(const char * buf)
{
    size_t buf_len = strlen(buf);
    fwrite(buf, sizeof(char), buf_len, fptr);
}

void my_profiler_init(void)
{
    system("/lvgl_trace.txt");
    fptr = fopen("/lvgl_trace.txt", "w");

    lv_profiler_builtin_config_t config;
    lv_profiler_builtin_config_init(&config);
    config.tick_per_sec = 1000000; /* One second is equal to 1000000 microseconds */
    config.tick_get_cb = my_get_tick_us_cb;
    config.flush_cb = my_profiler_flush;
    lv_profiler_builtin_init(&config);
}
#endif


int main(int argc, char **argv)
{
    lv_font_t * font;
    lv_obj_t * term;

    term_updater_data_t updater_data;
    char term_buf[BUFFSIZE];

    lv_init();
#if LV_USE_PROFILER
    //my_profiler_init();
#endif

    config_args_parse(&g_conf, argc, argv);
    LV_LOG_INFO("term cmd: %s\n", conf.command_argv[LV_MAX(conf.command_argc-1, 0)]);

    if (g_conf.quiet)
    {
        term_set_log_level(LV_LOG_LEVEL_USER);
    }
    

    lv_log_register_print_cb(term_lv_log_callback);
    

    //hal_init(204, 180);
    hal_init(198, 171);

    LV_LOG_INFO("Lvgl ver %d.%d\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR);

    
    //lv_font_t * font = &lv_font_unscii_8;
    font = font_init("./Hack-Regular.ttf",32);
    
    if(!font) {
        LV_LOG_WARN("Freetype font create failed. Use default");
        //return -1;
    }


    term = init_term(lv_screen_active(), font, NULL);

    updater_data.term = term;
    updater_data.buf = term_buf;

    if (g_conf.file_path)
    {
        process_from_file(&updater_data);
    } else {
        process_from_pty(&updater_data);
    }

    while(1) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
        //lv_disp_remove
    }

    return 0;
}