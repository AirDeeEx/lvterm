#include "font.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

void font_check_error(lv_font_t** font)
{
    if(!*font)
    {
       LV_LOG_WARN("Font error, fallback to built-in font.");
       LV_LOG_WARN("f point: %p", *font);
       *font = &lv_font_unscii_8;
       LV_LOG_WARN("f end point: %p", *font); 
    }
}

lv_font_t * font_init(char * path, int prefered_size)
{
	lv_font_t * font = NULL;
	char actualpath [PATH_MAX+1];
	char lvgl_path [PATH_MAX+3];
	char *actualpath_p;
	
	LV_LOG_WARN("Path %s", path);
    actualpath_p = realpath(path, actualpath);
    LV_LOG_WARN("Actual path %s", actualpath_p);
    sprintf(lvgl_path, "A:%s", actualpath);
    LV_LOG_WARN("LVGL path %s", lvgl_path);
    

#if LV_USE_FREETYPE
    LV_LOG_WARN("Use freetype font.");
    font = lv_freetype_font_create(actualpath_p,
                                            LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                            prefered_size,
                                            LV_FREETYPE_FONT_STYLE_NORMAL);
    font_check_error(&font);
#elif LV_USE_TINY_TTF
    LV_LOG_WARN("Use tiny_ttf font.");
    font = lv_tiny_ttf_create_file(lvgl_path, prefered_size);
    LV_LOG_WARN("before f point: %p", font); 
    font_check_error(&font);
    LV_LOG_WARN("after f point: %p", font); 
#else


    LV_LOG_WARN("Use built-in font.");
    font = &lv_font_unscii_8;
#endif
    if(!font)
    {
        LV_LOG_WARN("Null font");
    }
    return font;
}
