#include "actions.h"
#include "vars.h"
#include "ui.h"

extern objects_t objects;

void action_main_info_auto_wiggle(lv_obj_t *obj) {
    static uint32_t last_scroll_time = 0;
    static bool scroll_to_right = true;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_scroll_to_x);
    lv_anim_set_values(&a, lv_obj_get_scroll_x(obj), 128);  // от текущей до целевой позиции
    lv_anim_set_time(&a, 200);  // ← вот здесь регулируется скорость (в мс)

    lv_anim_t b;
    lv_anim_init(&b);
    lv_anim_set_var(&b, obj);
    lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_scroll_to_x);
    lv_anim_set_values(&b, lv_obj_get_scroll_x(obj), 0);  // от текущей до целевой позиции
    lv_anim_set_time(&b, 200);  // ← вот здесь регулируется скорость (в мс)

    uint32_t now = lv_tick_get();  // системное время LVGL в миллисекундах

    if (now - last_scroll_time > 5000) {  // раз в 3 секунды
        //lv_obj_t *scr = objects.c_temp_and_graph;
        if (scroll_to_right) {
            lv_anim_start(&a);
            //lv_obj_scroll_to_x(obj, 128, LV_ANIM_ON);
        } else {
            //lv_obj_scroll_to_x(obj, 0, LV_ANIM_ON);
            lv_anim_start(&b);
        }
        scroll_to_right = !scroll_to_right;
        last_scroll_time = now;
    }
    
    lv_image_set_rotation(objects.image_wind_direction, get_wind_direction());
}
/*

                    lv_chart_set_range(obj, LV_CHART_AXIS_PRIMARY_X, -2, 42);
                    lv_chart_series_t *ser = lv_chart_add_series(obj, lv_palette_main(LV_PALETTE_ORANGE), LV_CHART_AXIS_PRIMARY_Y);
                    lv_chart_set_next_value2(obj, ser, 5, 22);
                    lv_chart_set_next_value2(obj, ser, 15, 21);
                    lv_chart_set_next_value2(obj, ser, 25, 20);
                    lv_chart_set_next_value2(obj, ser, 35, 20);
                    lv_chart_refresh(obj);

                    */
void action_chart_invalidate(lv_event_t * e) {
    lv_obj_t * chart = lv_event_get_target(e);
    lv_obj_invalidate(chart);
}