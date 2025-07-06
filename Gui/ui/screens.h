#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *lbl_date_time;
    lv_obj_t *c_temp_and_graph;
    lv_obj_t *lbl_wind_speed_2;
    lv_obj_t *lbl_wind_speed_1;
    lv_obj_t *image_wind_direction;
    lv_obj_t *lbl_wind_speed;
    lv_obj_t *lbl_humidity;
    lv_obj_t *lbl_pressure;
    lv_obj_t *img_weather;
    lv_obj_t *lbl_temperature_black;
    lv_obj_t *lbl_temperature_white;
    lv_obj_t *lbl_weather_description;
    lv_obj_t *graph_forecast;
    lv_obj_t *lbl_hour1;
    lv_obj_t *lbl_hour2;
    lv_obj_t *lbl_hour3;
    lv_obj_t *lbl_hour4;
    lv_obj_t *img_weather_hour1;
    lv_obj_t *img_weather_hour2;
    lv_obj_t *img_weather_hour3;
    lv_obj_t *img_weather_hour4;
    lv_obj_t *lbl_temperature_hour1;
    lv_obj_t *lbl_temperature_hour2;
    lv_obj_t *lbl_temperature_hour3;
    lv_obj_t *lbl_temperature_hour4;
    lv_obj_t *lbl_temperature_external1;
    lv_obj_t *lbl_humidity_external1;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/