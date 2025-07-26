#include "actions.h"
#include "vars.h"
#include "ui.h"

#include "images.h"

extern objects_t objects;

void action_main_info_auto_wiggle(lv_obj_t *obj) {
    static uint32_t last_scroll_time = 0;
    static enum Pages {
        PAGE1,
        PAGE2,
        PAGE3,
        PAGES_COUNT
    }currentPage;
    //static bool scroll_to_right = true;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_scroll_to_x);
    lv_anim_set_values(&a, lv_obj_get_scroll_x(obj), 128);  // от текущей до целевой позиции
    lv_anim_set_time(&a, 750);  // ← вот здесь регулируется скорость (в мс)

    lv_anim_t b;
    lv_anim_init(&b);
    lv_anim_set_var(&b, obj);
    lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_scroll_to_x);
    lv_anim_set_values(&b, lv_obj_get_scroll_x(obj), 256);  // от текущей до целевой позиции
    lv_anim_set_time(&b, 750);  // ← вот здесь регулируется скорость (в мс)

    lv_anim_t c;
    lv_anim_init(&c);
    lv_anim_set_var(&c, obj);
    lv_anim_set_path_cb(&c, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&c, (lv_anim_exec_xcb_t)lv_obj_scroll_to_x);
    lv_anim_set_values(&c, lv_obj_get_scroll_x(obj), 0);  // от текущей до целевой позиции
    lv_anim_set_time(&c, 750);  // ← вот здесь регулируется скорость (в мс)

    lv_anim_t* animations[3] = {&a, &c, &c}; 

    uint32_t now = lv_tick_get();  // системное время LVGL в миллисекундах

    if (now - last_scroll_time > 5000) {  // раз в 3 секунды
        /*if (scroll_to_right) {
            lv_anim_start(&a);
        } else {
            lv_anim_start(&b);
        }*/
        lv_anim_start(animations[currentPage]);
        currentPage = (currentPage + 1) % PAGES_COUNT;
        //scroll_to_right = !scroll_to_right;
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

void update_forecast_chart(lv_obj_t* chart, lv_chart_series_t* series) {
    
    //lv_chart_clear_series();

    static int8_t tmp[4] = {0,0,0,0};

    int8_t* temps = get_var_temperatures();

    if(tmp[0] != temps[0] || tmp[1] != temps[1] || tmp[2] != temps[2] || tmp[3] != temps[3]) {
        int8_t min = temps[0];
        int8_t max = temps[0];
        for(uint8_t i = 1; i < 4; ++i) {
            if(min > temps[i]) {
                min = temps[i];
            }
            if(max < temps[i]) {
                max = temps[i];
            }
        }

        min = min - 2;
        max = max + 2;
        max += (max-min);


        lv_chart_set_next_value2(chart, series, 5, temps[0]);
        lv_chart_set_next_value2(chart, series, 15, temps[1]);
        lv_chart_set_next_value2(chart, series, 25, temps[2]);
        lv_chart_set_next_value2(chart, series, 35, temps[3]);
        lv_chart_set_next_value2(chart, series, 36, temps[3]);

        lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, min, max);

        lv_chart_refresh(chart);

        tmp[0] = temps[0];
        tmp[1] = temps[1];
        tmp[2] = temps[2];
        tmp[3] = temps[3];
    }
}

const lv_image_dsc_t* getImageByIndex(uint8_t newIcon) {
    const lv_image_dsc_t* newImage;
    switch(newIcon) {
        case   1: newImage = &img_i01d; break;
        case   2: newImage = &img_i02d; break;
        case   3: newImage = &img_i03d; break;
        case   4: newImage = &img_i04d; break;
        case   9: newImage = &img_i09d; break;
        case  10: newImage = &img_i10d; break;
        case  11: newImage = &img_i11d; break;
        case  13: newImage = &img_i13d; break;
        case  50: newImage = &img_i50d; break;
        case 129: newImage = &img_i01n; break;
        case 130: newImage = &img_i02n; break;
        case 131: newImage = &img_i03n; break;
        case 132: newImage = &img_i04d; break;
        case 137: newImage = &img_i09d; break;
        case 138: newImage = &img_i10n; break;
        case 139: newImage = &img_i11d; break;
        case 141: newImage = &img_i13d; break;
        case 178: newImage = &img_i50n; break;
        default:  newImage = &img_i50n; break;
    }
    return newImage;
}