#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_i10d;
extern const lv_img_dsc_t img_i10n;
extern const lv_img_dsc_t img_arrow;
extern const lv_img_dsc_t img_humid;
extern const lv_img_dsc_t img_i01d;
extern const lv_img_dsc_t img_i02d;
extern const lv_img_dsc_t img_i03d;
extern const lv_img_dsc_t img_i04d;
extern const lv_img_dsc_t img_i09d;
extern const lv_img_dsc_t img_i01n;
extern const lv_img_dsc_t img_i02n;
extern const lv_img_dsc_t img_i03n;
extern const lv_img_dsc_t img_i04n;
extern const lv_img_dsc_t img_i09n;
extern const lv_img_dsc_t img_i11d;
extern const lv_img_dsc_t img_i13d;
extern const lv_img_dsc_t img_i50d;
extern const lv_img_dsc_t img_i11n;
extern const lv_img_dsc_t img_i13n;
extern const lv_img_dsc_t img_i50n;
extern const lv_img_dsc_t img_temperature;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[21];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/