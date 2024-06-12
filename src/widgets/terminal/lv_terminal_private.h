#ifndef LV_TERMINAL_PRIVATE_H
#define LV_TERMINAL_PRIVATE_H

#include <lvgl/lvgl.h>
#include <vterm.h>

typedef struct {
    uint32_t cell_char;
    lv_color_t bg;
    lv_color_t fg;
} lv_term_cell_t;

typedef lv_term_cell_t** lv_term_cell_array_t;

typedef struct {
    lv_obj_t obj;
    uint16_t rows;
    uint16_t columns;

    lv_point_t cursor_pos;

    lv_font_t * font;
    lv_color_t default_fg;
    lv_color_t default_bg;
    lv_opa_t cell_opacity;


    uint16_t cell_width;
    uint16_t cell_height;
    lv_align_t cell_grid_align;

    VTerm *vt;
    VTermScreen *vts;

    bool is_initialized;
//     char * text;
//     union {
//         char * tmp_ptr; /*Pointer to the allocated memory containing the character replaced by dots*/
//         char tmp[LV_LABEL_DOT_NUM + 1]; /*Directly store the characters if <=4 characters*/
//     } dot;
//     uint32_t dot_end;  /*The real text length, used in dot mode*/


//     lv_point_t size_cache; /*Text size cache*/
//     lv_point_t offset; /*Text draw position offset*/
//     lv_label_long_mode_t long_mode : 3; /*Determine what to do with the long texts*/
//     uint8_t static_txt : 1;             /*Flag to indicate the text is static*/
//     uint8_t expand : 1;                 /*Ignore real width (used by the library with LV_LABEL_LONG_SCROLL)*/
//     uint8_t dot_tmp_alloc : 1;          /*1: dot is allocated, 0: dot directly holds up to 4 chars*/
//     uint8_t invalid_size_cache : 1;     /*1: Recalculate size and update cache*/
} _lv_terminal_t;

lv_obj_t * lv_terminal_create(lv_obj_t * parent);
void lv_terminal_set_matrix_size(lv_obj_t * obj, uint16_t rows, uint16_t columns);
uint16_t lv_terminal_get_matrix_aval_rows(lv_obj_t * obj);
uint16_t lv_terminal_get_matrix_aval_cols(lv_obj_t * obj);
void lv_terminal_set_font(lv_obj_t * obj, lv_font_t * font);
void lv_terminal_putc(lv_obj_t * obj, uint32_t letter);
void lv_terminal_puts(lv_obj_t * obj, const char * str);
void lv_terminal_clear(lv_obj_t * obj);
void lv_terminal_parse(lv_obj_t * obj, const char * bytes, size_t len);
void lv_terminal_set_cell_size(lv_obj_t * obj, uint16_t width, uint16_t height);
void lv_terminal_get_font_max_bitmap(lv_obj_t * obj, uint16_t * max_width, uint16_t * max_height);
uint32_t lv_terminal_resize_event_id();
void lv_terminal_set_opacity(lv_obj_t * obj, lv_opa_t opa);


#endif
