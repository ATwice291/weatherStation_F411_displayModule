#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_main_info_auto_wiggle(lv_obj_t *obj);
extern void action_chart_invalidate(lv_event_t * e);

void update_forecast_chart(lv_obj_t* chart, lv_chart_series_t* series);

const lv_image_dsc_t* getImageByIndex(uint8_t index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/