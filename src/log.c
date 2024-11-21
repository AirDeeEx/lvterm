#include <log.h>
#include <stdio.h>
#include <stdarg.h>

static int log_level = LV_LOG_LEVEL_INFO;


void term_lv_log_callback(lv_log_level_t level, const char * buf)
{
    if(level < log_level) return;

    printf("%s", buf);
}

void term_set_log_level(lv_log_level_t level)
{
    if (level >= _LV_LOG_LEVEL_NUM)
    {
        log_level = LV_LOG_LEVEL_INFO;
    } else {
        log_level = level;
    }
    
}