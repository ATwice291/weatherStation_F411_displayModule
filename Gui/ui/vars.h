#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_DATE = 0,
    FLOW_GLOBAL_VARIABLE_HUMIDITY = 1,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE = 2
};

// Native global variables

extern const char *get_var_date();
extern void set_var_date(const char *value);
extern const char *get_var_humidity();
extern void set_var_humidity(const char *value);
extern const char *get_var_temperature();
extern void set_var_temperature(const char *value);

extern uint16_t get_wind_direction();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/