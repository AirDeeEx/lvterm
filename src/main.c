
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
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/src/libs/freetype/lv_freetype.h"
#include <stdio.h>
#include "./main/src/widgets/terminal/lv_terminal.h"
#include "main/src/term_io.h"
#include "main/src/config.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/    

static void terminal_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * term = (lv_obj_t *)lv_event_get_param(e);
    term_pty_t * pty = (term_pty_t *)lv_event_get_user_data(e);

    if(code == lv_terminal_resize_event_id())
    {
        uint16_t cols = lv_terminal_get_cols(term);
        uint16_t rows = lv_terminal_get_rows(term);

        LV_LOG_USER("Resize event: rows=%d, cols=%d", rows, cols);
        term_set_size(pty, cols, rows);
    }
    else if(code == lv_terminal_reset_event_id())
    {
        LV_LOG_USER("Reset event!");
        term_notify_resize(pty);
    }
    else if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
        lv_terminal_clear(term);
    }
}

typedef struct {
    term_pty_t * pty;
    lv_obj_t * term;
} term_reader_data_t;

void my_timer(lv_timer_t * timer)
{
    /*Use the user_data*/
    term_reader_data_t * data = timer->user_data;
    //LV_LOG_USER("my_timer called with pty: %s", data->pty->slave_path);

    uint32_t debug_char[2];
    debug_char[1] = '\0';

    int nread = term_readptym(data->pty);

    if (nread != -1)
    {
        LV_LOG_USER("read: %d",nread);
        lv_terminal_parse(data->term, data->pty->ptybuf, nread);
    }else{
        //LV_LOG_USER("empty read");
    }
}

lv_obj_t * init_term(lv_obj_t * parent, lv_font_t * font, term_pty_t * pty)
{
    int res = -1;

    lv_obj_t * term = lv_terminal_create(parent);
    lv_obj_add_event_cb(term, terminal_event_handler, LV_EVENT_ALL, pty);

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

int main(int argc, char **argv)
{
    /*Initialize LVGL*/
    lv_init();

    term_config_t conf;
    config_args_parse(&conf, argc, argv);
    printf("term cmd: %s\n", conf.command_argv[LV_MAX(conf.command_argc-1, 0)]);
    

    //hal_init(204, 180);
    hal_init(198, 171);

    printf("Lvgl ver %d.%d\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR);

    int res;


    lv_font_t * font = lv_freetype_font_create("/tmp/Hack-Regular.ttf",
                                            LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                            11,
                                            LV_FREETYPE_FONT_STYLE_NORMAL);

    if(!font) {
        LV_LOG_WARN("Freetype font create failed. Use default");
        //return -1;
    }

    term_pty_t term_pty;
    res = term_openpty(&term_pty);
    LV_LOG_WARN("Pty open: %s, res=%d", term_pty.slave_path, res);


    lv_obj_t * term = init_term(lv_screen_active(), font, &term_pty);

    term_reader_data_t reader_data;
    reader_data.pty = &term_pty;
    reader_data.term = term;

    lv_timer_t * timer = lv_timer_create(my_timer, 100,  &reader_data);

    if (!conf.pty_only)
    {
        

        char* default_cmd[] = { "/usr/bin/htop", NULL };
        //char* term_envp[] = { NULL };

        LV_LOG_USER("Try Fork");

        char** term_cmd = NULL;
        if (conf.command_argc > 0)
        {
            term_cmd = conf.command_argv;
        }
        else
        {
            LV_LOG_USER("Use default cmd!");
            term_cmd = default_cmd;
        }
        
        term_process(&term_pty, term_cmd, environ);
    }else{
        char ansi_escape[50];
        char* text = "Pty ready!";
        int slen;
        // lv_memset(ansi_escape, 0, sizeof(char) * 50);

        uint16_t col = lv_terminal_get_cols(term)/2;
        uint16_t row = lv_terminal_get_rows(term)/2;

        LV_LOG_USER("col,row: %d, %d", col, row);

        slen = lv_strlen(text);

        sprintf(ansi_escape, "\e[2J\e[%d;%dH%s", row, col - slen, text);
        

        lv_terminal_puts(term, ansi_escape);
        // LV_LOG_USER("%s", ansi_escape);
        // lv_terminal_parse(term, ansi_escape, 49);
        // lv_terminal_puts(term, "test");
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

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if USE_SIMULATOR
static lv_display_t * sdl_init(int32_t w, int32_t h)
{
lv_group_set_default(lv_group_create());

lv_display_t * disp = lv_sdl_window_create(w, h);

lv_indev_t * mouse = lv_sdl_mouse_create();
lv_indev_set_group(mouse, lv_group_get_default());
lv_indev_set_display(mouse, disp);
lv_display_set_default(disp);

LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
lv_obj_t * cursor_obj;
cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
lv_image_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
lv_indev_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/

lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
lv_indev_set_display(mousewheel, disp);
lv_indev_set_group(mousewheel, lv_group_get_default());

lv_indev_t * kb = lv_sdl_keyboard_create();
lv_indev_set_display(kb, disp);
lv_indev_set_group(kb, lv_group_get_default());
//lv_indev_add_event_cb(kb,kb_callback,LV_EVENT_ALL, NULL);
return disp;
}
#else
static lv_display_t * fbdev_init(int32_t w, int32_t h)
{
    int res = 0;
    uint32_t offset_x, offset_y, x_max, y_max;

    lv_display_t * disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    // offset_x = 17;
    // offset_y = 130;

    // x_max = w;
    // y_max = h;

    x_max = lv_disp_get_hor_res(disp);
    y_max = lv_disp_get_ver_res(disp);
    offset_x = 0;
    offset_y = 0;

    char* fb_geometry_env = getenv("FB_GEOMETRY");
    LV_LOG_WARN("FB_GEOMETRY: %s", fb_geometry_env);
    if (fb_geometry_env)
    {
        res = sscanf(fb_geometry_env, "%dx%d+%d+%d", &x_max, &y_max, &offset_x, & offset_y);
        if (res < 4)
        {
            LV_LOG_ERROR("sscanf err: %d", res);
        }
        
    }
    else
    {
        LV_LOG_WARN("DEF: %dx%d+%d+%d", x_max, y_max, offset_x, offset_y);
    }

    lv_display_set_offset(disp, offset_x, offset_y);
    //lv_display_set_physical_resolution(disp,205,108);
    //lv_display_set_resolution(disp,208,178);
    lv_display_set_resolution(disp, x_max, y_max);
    

    lv_display_set_default(disp);

    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_180);

    return disp;
}
#endif
/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t * hal_init(int32_t w, int32_t h)
{
#ifdef USE_SIMULATOR
return sdl_init(w,h);
#else
return fbdev_init(w,h);
#endif


}
