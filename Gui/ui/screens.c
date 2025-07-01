#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 128, 160);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    add_style_page(obj);
    {
        lv_obj_t *parent_obj = obj;
        {
            // lbl_date_time
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_date_time = obj;
            lv_obj_set_pos(obj, 0, 5);
            lv_obj_set_size(obj, 128, 15);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 0, 104);
            lv_obj_set_size(obj, 32, 32);
            lv_image_set_src(obj, &img_humid);
        }
        {
            // lbl_humidity
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_humidity = obj;
            lv_obj_set_pos(obj, 32, 104);
            lv_obj_set_size(obj, 28, 32);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // image_wind_direction
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.image_wind_direction = obj;
            lv_obj_set_pos(obj, 60, 104);
            lv_obj_set_size(obj, 32, 32);
            lv_image_set_src(obj, &img_arrow);
            lv_image_set_rotation(obj, 900);
        }
        {
            // lbl_wind_speed
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lbl_wind_speed = obj;
            lv_obj_set_pos(obj, 92, 104);
            lv_obj_set_size(obj, 36, 32);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // c_temp_and_graph
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.c_temp_and_graph = obj;
            lv_obj_set_pos(obj, 0, 21);
            lv_obj_set_size(obj, 128, 80);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_scroll_dir(obj, LV_DIR_HOR);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // img_weather
                    lv_obj_t *obj = lv_image_create(parent_obj);
                    objects.img_weather = obj;
                    lv_obj_set_pos(obj, 48, 0);
                    lv_obj_set_size(obj, 80, 64);
                    lv_image_set_src(obj, &img_i11n);
                }
                {
                    // lbl_temperature_black
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_temperature_black = obj;
                    lv_obj_set_pos(obj, 5, 16);
                    lv_obj_set_size(obj, 64, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_pf_din_pro_medium, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_letter_space(obj, -3, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff6e6e6e), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // lbl_temperature_white
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.lbl_temperature_white = obj;
                    lv_obj_set_pos(obj, 4, 15);
                    lv_obj_set_size(obj, 64, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &ui_font_pf_din_pro_medium, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_letter_space(obj, -3, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffeaeaea), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 64);
                    lv_obj_set_size(obj, 128, LV_SIZE_CONTENT);
                    lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "Overcast clouds");
                }
                {
                    // graph_forecast
                    lv_obj_t *obj = lv_chart_create(parent_obj);
                    objects.graph_forecast = obj;
                    lv_obj_set_pos(obj, 128, 0);
                    lv_obj_set_size(obj, 128, 80);
                    lv_obj_add_event_cb(obj, action_chart_invalidate, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_chart_set_range(obj, LV_CHART_AXIS_PRIMARY_X, -1, 41);
                    lv_obj_refresh_ext_draw_size(obj);
                    lv_chart_series_t *ser = lv_chart_add_series(obj, lv_palette_main(LV_PALETTE_DEEP_ORANGE), LV_CHART_AXIS_PRIMARY_Y);
                    lv_chart_set_type(obj, LV_CHART_TYPE_SCATTER);
                    lv_chart_set_point_count(obj, 5);
                    lv_chart_set_next_value2(obj, ser, 5, 22);
                    lv_chart_set_next_value2(obj, ser, 15, 21);
                    lv_chart_set_next_value2(obj, ser, 25, 20);
                    lv_chart_set_next_value2(obj, ser, 35, 20);
                    lv_chart_set_next_value2(obj, ser, 36, 20);
                    lv_chart_refresh(obj);
                }
            }
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        const char *new_val = get_var_date();
        const char *cur_val = lv_label_get_text(objects.lbl_date_time);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lbl_date_time;
            lv_label_set_text(objects.lbl_date_time, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_humidity();
        const char *cur_val = lv_label_get_text(objects.lbl_humidity);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lbl_humidity;
            lv_label_set_text(objects.lbl_humidity, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_humidity();
        const char *cur_val = lv_label_get_text(objects.lbl_wind_speed);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lbl_wind_speed;
            lv_label_set_text(objects.lbl_wind_speed, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_temperature();
        const char *cur_val = lv_label_get_text(objects.lbl_temperature_black);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lbl_temperature_black;
            lv_label_set_text(objects.lbl_temperature_black, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_temperature();
        const char *cur_val = lv_label_get_text(objects.lbl_temperature_white);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.lbl_temperature_white;
            lv_label_set_text(objects.lbl_temperature_white, new_val);
            tick_value_change_obj = NULL;
        }
    }
    action_main_info_auto_wiggle(objects.c_temp_and_graph);
    lv_chart_refresh(objects.graph_forecast);
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
