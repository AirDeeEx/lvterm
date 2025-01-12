#include "lv_terminal_private.h"
#include <lvgl/lvgl.h>
#include <stdio.h>

#define MY_CLASS (&lv_terminal_class)

static void lv_terminal_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_terminal_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_terminal_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void draw_new(lv_event_t * e);
static void vt_init(_lv_terminal_t * term, int rows, int cols);
static void handle_term_resize(_lv_terminal_t * term, uint32_t rows, uint32_t cols);

const lv_obj_class_t lv_terminal_class = {
    .constructor_cb = lv_terminal_constructor,
    .destructor_cb = lv_terminal_destructor,
    .event_cb = lv_terminal_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(_lv_terminal_t),
    .base_class = &lv_obj_class,
    .name = "terminal",
};

static uint32_t TERM_RESIZE_EVENT = 0;
static uint32_t TERM_RESET_EVENT = 0;

lv_obj_t * lv_terminal_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_terminal_set_font(lv_obj_t * obj, lv_font_t * font)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    lv_obj_invalidate(obj);

    term->font = font;
}

// void lv_terminal_set_font(lv_obj_t * obj, lv_font_t * font)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     _lv_terminal_t * term = (_lv_terminal_t *)obj;

//     lv_obj_invalidate(obj);

//     term->font = font;
// }

void lv_terminal_set_matrix_size(lv_obj_t * obj, uint16_t rows, uint16_t columns)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    lv_obj_invalidate(obj);

    if (!term->vt)
    {
        vt_init(term,rows,columns);
    }

    handle_term_resize(term, rows, columns);
    
}

uint16_t lv_terminal_get_matrix_aval_rows(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;
    lv_area_t content;

    lv_obj_update_layout(obj);
    lv_obj_get_content_coords(&term->obj,&content);

    //LV_LOG_WARN("UPD: self_h=%d, obj_h=%d, cont_h=%d",lv_obj_get_self_height(obj),lv_obj_get_height(obj), lv_area_get_height(&content));

    LV_LOG_WARN("rows: %d/%d=%d",
                lv_area_get_height(&content),
                term->cell_height,
                lv_area_get_height(&content) / term->cell_height);

    return lv_area_get_height(&content) / term->cell_height;
}

uint16_t lv_terminal_get_matrix_aval_cols(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;
    lv_area_t content;

    lv_obj_update_layout(obj);
    lv_obj_get_content_coords(obj,&content);

    LV_LOG_WARN("cols: %d/%d=%d",
                lv_area_get_width(&content),
                term->cell_width,
                lv_area_get_width(&content) / term->cell_width);

    return lv_area_get_width(&content) / term->cell_width;
}

uint16_t lv_terminal_get_cols(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    return term->columns;
}

uint16_t lv_terminal_get_rows(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    return term->rows;
}

void lv_terminal_set_cell_size(lv_obj_t * obj, uint16_t width, uint16_t height)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    lv_obj_invalidate(obj);

    term->cell_width = width;
    term->cell_height = height;
}

void lv_terminal_get_font_max_bitmap(lv_obj_t * obj, uint16_t * max_width, uint16_t * max_height)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    #define WIDEST_LETTER 'W'

    *max_width = lv_font_get_glyph_width(term->font, WIDEST_LETTER, 0);
    *max_height = lv_font_get_line_height(term->font);

    LV_LOG_TRACE("max_w=%d", *max_width);
}

union utf8_letter
{
    char bytes[4];
    uint32_t letter;
};

void lv_terminal_putc(lv_obj_t * obj, uint32_t letter)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    

    union utf8_letter buf;
    buf.letter = letter;
    
    

    vterm_input_write(term->vt, &buf.bytes[0], sizeof(buf));

    lv_obj_invalidate(obj);
}

void lv_terminal_puts(lv_obj_t * obj, const char * str)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;
    int str_len = lv_strlen(str);

    vterm_input_write(term->vt, str, str_len);

    lv_obj_invalidate(obj);
}

uint32_t lv_terminal_resize_event_id()
{
    LV_ASSERT_FORMAT_MSG(TERM_RESIZE_EVENT != 0, "Attempt to get unregistred event id (%d)", TERM_RESIZE_EVENT);
    return TERM_RESIZE_EVENT;
}

uint32_t lv_terminal_reset_event_id()
{
    LV_ASSERT_FORMAT_MSG(TERM_RESET_EVENT != 0, "Attempt to get unregistred event id (%d)", TERM_RESET_EVENT);
    return TERM_RESET_EVENT;
}

void lv_terminal_clear(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    vterm_screen_reset(term->vts, 0);

    lv_obj_invalidate(obj);

    lv_obj_send_event(&term->obj, lv_terminal_reset_event_id(), NULL);
}

void lv_terminal_parse(lv_obj_t * obj, const char * bytes, size_t len)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    vterm_input_write(term->vt, bytes, len);

    lv_obj_invalidate(obj);
}



void lv_terminal_set_opacity(lv_obj_t * obj, lv_opa_t opa)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    lv_obj_invalidate(obj);

    term->cell_opacity = opa;
}

static void lv_terminal_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
    LV_LOG_TRACE("term constructor\n");
    LV_LOG_TRACE("cell sizeof: %ld\n",sizeof(lv_term_cell_t));

    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    term->rows = 0;
    term->columns = 0;
    term->font = (lv_font_t *)lv_font_default();
    //term->cell_width = 8;
    term->cell_width = 8;
    term->cell_height = 16;
    term->cell_grid_align = LV_ALIGN_CENTER;
    term->cell_opacity = 255;
    term->default_bg = lv_color_black();
    term->default_fg = lv_color_white();
    term->is_initialized = false;
    term->vt = NULL;
    term->vts = NULL;

    if (TERM_RESIZE_EVENT == 0)
    {
        TERM_RESIZE_EVENT = lv_event_register_id();
        LV_LOG_WARN("register TERM_RESIZE_EVENT: id=%d", TERM_RESIZE_EVENT);
    }
    
    if (TERM_RESET_EVENT == 0)
    {
        TERM_RESET_EVENT = lv_event_register_id();
        LV_LOG_WARN("register TERM_RESET_EVENT: id=%d", TERM_RESET_EVENT);
    }

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_terminal_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;

    if(term->vt)
        vterm_free(term->vt);

    term->vt = NULL;
    term->font = NULL;
}

static void lv_terminal_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    const lv_result_t res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RESULT_OK) return;

    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if((code == LV_EVENT_STYLE_CHANGED) || (code == LV_EVENT_SIZE_CHANGED)) {

    }
    else if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        /* Italic or other non-typical letters can be drawn of out of the object.
         * It happens if box_w + ofs_x > adw_w in the glyph.
         * To avoid this add some extra draw area.
         * font_h / 4 is an empirical value. */
        // const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        // const int32_t font_h = lv_font_get_line_height(font);
        // lv_event_set_ext_draw_size(e, font_h / 4);
    }
    else if(code == LV_EVENT_GET_SELF_SIZE) {
        _lv_terminal_t * term = (_lv_terminal_t *)obj;
        lv_point_t * self_size = lv_event_get_param(e);
        self_size->x = (term->columns * term->cell_width);// - 1;
        self_size->y = (term->rows * term->cell_height);// - 1;

        // self_size->x -= 1;
        // self_size->y -= 1;
        //LV_LOG_WARN("self_size: x1=%d, x2=%d",self_size->y,self_size->y);
    }
    else if(code == LV_EVENT_SCROLL) {
        LV_LOG_WARN("LV_EVENT_SCROLL");
        //draw_new(e);
    }
    else if(code == LV_EVENT_DRAW_MAIN) {
        draw_new(e);
    }
}

static void draw_cell_bg(lv_layer_t * layer, lv_area_t * area, lv_color_t color, lv_opa_t opacity)
{
  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_color = color;
  rect_dsc.bg_opa = opacity;
  //lv_memcpy(&rect_dsc.bg_color,color,sizeof(lv_color_t));

  lv_draw_rect(layer,&rect_dsc,area);
}

static void draw_cell_char(lv_layer_t * layer,lv_area_t * cell_area, uint32_t letter, lv_font_t * font, lv_color_t color)
{
    if (letter == 0x0)
    {
        //printf("null char return!\n");
        return;
    }

    LV_PROFILER_BEGIN_TAG("lv_terminal__draw_cell_char");

    //printf("draw char: %x\n", letter);
    
    
    lv_point_t char_point;
    char_point.x = cell_area->x1;
    char_point.y = cell_area->y1;


    //uint32_t letter = 0x2588;

    lv_draw_label_dsc_t char_dsc;
    lv_draw_label_dsc_init(&char_dsc);
    char_dsc.font = font;
    char_dsc.color = color;

    lv_font_glyph_dsc_t glyph_metrics;
    lv_font_get_glyph_dsc(char_dsc.font,&glyph_metrics,letter,letter);

    //printf("glyph: w=%d,h=%d\n",glyph_metrics.box_w,glyph_metrics.box_h);
    //printf("offset: x=%d,y=%d\n",glyph_metrics.ofs_x,glyph_metrics.ofs_y);

    lv_draw_character(layer,&char_dsc,&char_point,letter);

    LV_PROFILER_END_TAG("lv_terminal__draw_cell_char");
}

static bool get_cell_area(_lv_terminal_t * term, uint16_t row, uint16_t column, lv_area_t * area, lv_point_t * content_offset, lv_point_t * align_offset)
{
    //LV_LOG_WARN("CELL(%d): row=%d, col=%d",cell_index,row,column);

    if (row > term->rows-1)
    {
        LV_LOG_WARN("CELL: out of index (%d > %d)", row, term->rows);
        return false;
    }

    if (column > term->columns-1)
    {
        LV_LOG_WARN("CELL: out of index (%d > %d)", column, term->columns);
        return false;
    }


    // area->x1 = LV_MAX((term->cell_width * row) - 1, 0);
    // area->y1 = LV_MAX((term->cell_height * column) - 1, 0);

    // area->x2 = LV_MAX((term->cell_width * (row + 1)) - 1, 0);
    // area->y2 = LV_MAX((term->cell_height * (column + 1)) - 1, 0);
    // int32_t x_start = lv_obj_get_x(&term->obj);
    // LV_LOG_WARN("CELL: x_start %d)",x_start);

    //int32_t cell_start_w = LV_MAX((term->cell_width * column) - 1, 0); 
    int32_t cell_start_w = term->cell_width * column;
    int32_t cell_start_h = term->cell_height * row; 


    area->x1 = align_offset->x + cell_start_w + content_offset->x;
    area->y1 = align_offset->y + cell_start_h + content_offset->y;

    area->x2 = area->x1 + term->cell_width - 1;
    area->y2 = area->y1 + term->cell_height - 1;

    //LV_LOG_WARN("CELL: w=%d, h=%d",lv_area_get_width(area),lv_area_get_height(area));
    //LV_LOG_WARN("CELL(%d): (%d,%d), (%d,%d)",LV_MAX((term->cell_width * column) - 1, 0),area->x1,area->y1,area->x2,area->y2);
    LV_ASSERT(lv_area_get_width(area) == term->cell_width);
    LV_ASSERT(lv_area_get_height(area) == term->cell_height);

    return true;
}

void draw_term(_lv_terminal_t * term, lv_layer_t * layer)
{
    LV_PROFILER_BEGIN_TAG("lv_terminal__draw_term");

    VTermScreenCell cell;
    VTermPos pos;
    lv_color_t fg, bg;
    lv_area_t content_area;
    lv_point_t content_offset, align_offset;


    lv_obj_get_content_coords(&term->obj,&content_area);
    content_offset.x = content_area.x1;
    content_offset.y = content_area.y1;

    int32_t content_w = lv_area_get_width(&content_area);
    int32_t content_h = lv_area_get_height(&content_area);

    int32_t grid_w = (term->cell_width * term->columns);
    int32_t grid_h = (term->cell_height * term->rows);

    int32_t free_space_w = content_w - grid_w;
    int32_t free_space_h = content_h - grid_h;

    switch (term->cell_grid_align)
    {
    case LV_ALIGN_TOP_MID:
        align_offset.x = LV_MAX(free_space_w/2, 0);
        align_offset.y = 0;
        break;
    case LV_ALIGN_TOP_RIGHT:
        align_offset.x = LV_MAX(free_space_w, 0);
        align_offset.y = 0;
        break;
    case LV_ALIGN_BOTTOM_LEFT:
        align_offset.x = 0;
        align_offset.y = LV_MAX(free_space_h, 0);
        break;
    case LV_ALIGN_BOTTOM_MID:
        align_offset.x = LV_MAX(free_space_w/2, 0);
        align_offset.y = LV_MAX(free_space_h, 0);
        break;
    case LV_ALIGN_BOTTOM_RIGHT:
        align_offset.x = LV_MAX(free_space_w, 0);
        align_offset.y = LV_MAX(free_space_h, 0);
        break;
    case LV_ALIGN_LEFT_MID:
        align_offset.x = 0;
        align_offset.y = LV_MAX(free_space_h/2, 0);
        break;
    case LV_ALIGN_RIGHT_MID:
        align_offset.x = LV_MAX(free_space_w, 0);
        align_offset.y = LV_MAX(free_space_h/2, 0);
        break;
    case LV_ALIGN_CENTER:
        align_offset.x = LV_MAX(free_space_w/2, 0);
        align_offset.y = LV_MAX(free_space_h/2, 0);
        break;
    case LV_ALIGN_DEFAULT:
    case LV_ALIGN_TOP_LEFT:
    default:
        align_offset.x = 0;
        align_offset.y = 0;
        break;
    }


    //LV_LOG_WARN("c_w-g_w=free: %d-%d=%d (align=%d)", content_w,grid_w,free_space_w,align_offset.x);
    //LV_LOG_WARN("content_size: width=%d", content_w);
    //LV_LOG_WARN("align x: left=%d, right=%d", align_offset.x, content_area.x2 - (content_area.x1 + align_offset.x + grid_w));

    bg = lv_color_make(cell.bg.rgb.red, cell.bg.rgb.blue, cell.bg.rgb.green);

    lv_area_t grid_background_area;
    grid_background_area.x1 = content_offset.x + content_offset.x;
    grid_background_area.y1 = content_offset.y + content_offset.y;
    grid_background_area.x2 = content_offset.x + content_offset.x + grid_w;
    grid_background_area.y2 = content_offset.y + content_offset.y + grid_h;


    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_black();
    rect_dsc.bg_opa = term->cell_opacity;
    //lv_memcpy(&rect_dsc.bg_color,color,sizeof(lv_color_t));

    lv_draw_rect(layer,&rect_dsc,&grid_background_area);

    LV_PROFILER_BEGIN_TAG("lv_terminal__cell_draw_loop");

    for (size_t row = 0; row < term->rows; row++)
    {
        for (size_t col = 0; col < term->columns; col++)
        {
            pos.col = col;
            pos.row = row;
            vterm_screen_get_cell(term->vts, pos, &cell);

            fg = lv_color_make(cell.fg.rgb.red, cell.fg.rgb.blue, cell.fg.rgb.green);
            //bg = lv_color_black();

            lv_area_t cell_area;

            if (layer == NULL)
            {
                LV_LOG_WARN("layer is null!");
                return;
            }
            

            get_cell_area(term,row,col,&cell_area, &content_offset, &align_offset);

            //draw_cell_bg(layer, &cell_area, bg, term->cell_opacity);
            draw_cell_char(layer,&cell_area, cell.chars[0], term->font, fg);
        }
        //cl_flag = !cl_flag;
    }
    LV_PROFILER_BEGIN_TAG("lv_terminal__cell_draw_loop");

    LV_PROFILER_END_TAG("lv_terminal__draw_term");
}

static void draw_new(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    _lv_terminal_t * term = (_lv_terminal_t *)obj;
    lv_layer_t * layer = lv_event_get_layer(e);

    lv_term_cell_t * cell;
    lv_area_t cell_area;

    bool cl_flag = true;

    if (!term->vt)
    {
        LV_LOG_WARN("term: vt null!!\n");
        return;
    }

    draw_term(term, layer);

    //vterm_screen_flush_damage(term->vts);
}

static void handle_term_resize(_lv_terminal_t * term, uint32_t rows, uint32_t cols)
{
    term->rows = rows;
    term->columns = cols;

    lv_obj_send_event(&term->obj, lv_terminal_resize_event_id(), &term->obj);
}

// VT utils

int vt_damage(VTermRect rect, void *user)
{
    return 0;
}

int vt_moverect(VTermRect dest, VTermRect src, void *user)
{
    return 0;
}

int vt_movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user)
{
    return 0;
}

int vt_resize(int rows, int cols, void *user)
{
    _lv_terminal_t * term = (_lv_terminal_t *)user;
    term->rows = rows;
    term->columns = cols;
    
    return 0;
}

static int tmp_y_cnt = 0;

static VTermScreenCallbacks cb_screen = {
  //.damage      = &vt_damage,
  //.moverect    = &vt_moverect,
  //.movecursor  = &vt_movecursor,
  .resize      = &vt_resize,
  //.sb_pushline = &screen_sb_pushline,
};

static long vt_alloc_total = 0;

static void * vt_lvgl_malloc(size_t size, void *allocdata)
{
    vt_alloc_total += size;
    //LV_LOG_WARN("new=%ld, total=%ld kb", size, vt_alloc_total / 1024);
  void *ptr = lv_malloc_zeroed(size);
  return ptr;
}

static void vt_lvgl_free(void *ptr, void *allocdata)
{
    //LV_LOG_WARN("free at %p", ptr);
  lv_free(ptr);
}

static VTermAllocatorFunctions vt_lvgl_alloc_func = {
  .malloc = &vt_lvgl_malloc,
  .free   = &vt_lvgl_free,
};

static void vt_init(_lv_terminal_t * term, int rows, int cols)
{
    if (term->vt)
    {
        return;
    }
    
    //term->vt = vterm_new(rows, cols);
    term->vt = vterm_new_with_allocator(rows, cols, &vt_lvgl_alloc_func, NULL);
    vterm_set_utf8(term->vt, true);

    term->vts = vterm_obtain_screen(term->vt);
    vterm_screen_set_callbacks(term->vts, &cb_screen, term);

    vterm_screen_reset(term->vts, 1);

}