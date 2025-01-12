#include <lvgl.h>
#include <stdio.h>


#if USE_SIMULATOR
lv_display_t * sdl_init(int32_t w, int32_t h)
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
lv_display_t * fbdev_init(int32_t w, int32_t h)
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
    if (fb_geometry_env)
    {
        LV_LOG_WARN("FB_GEOMETRY: %s", fb_geometry_env);
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
lv_display_t * hal_init(int32_t w, int32_t h)
{
#ifdef USE_SIMULATOR
    return sdl_init(w,h);
#else
    return fbdev_init(w,h);
#endif


}