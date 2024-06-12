#ifndef LV_TERMINAL_H
#define LV_TERMINAL_H

#include <lvgl.h>

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
uint16_t lv_terminal_get_cols(lv_obj_t * obj);
uint16_t lv_terminal_get_rows(lv_obj_t * obj);
void lv_terminal_set_opacity(lv_obj_t * obj, lv_opa_t opa);

// Events
uint32_t lv_terminal_reset_event_id();
uint32_t lv_terminal_resize_event_id();

#endif
