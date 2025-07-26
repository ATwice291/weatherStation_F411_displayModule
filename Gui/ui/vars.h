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
    FLOW_GLOBAL_VARIABLE_TEMPERATURE = 2,
    FLOW_GLOBAL_VARIABLE_HOUR1 = 3,
    FLOW_GLOBAL_VARIABLE_WIND_SPEED = 4,
    FLOW_GLOBAL_VARIABLE_WEATHER_DESCRIPTION = 5,
    FLOW_GLOBAL_VARIABLE_HOUR2 = 6,
    FLOW_GLOBAL_VARIABLE_HOUR3 = 7,
    FLOW_GLOBAL_VARIABLE_HOUR4 = 8,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE_HOUR1 = 9,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE_HOUR2 = 10,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE_HOUR3 = 11,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE_HOUR4 = 12,
    FLOW_GLOBAL_VARIABLE_TEMPERATURE_EXTERNAL1 = 13,
    FLOW_GLOBAL_VARIABLE_HUMIDITY_EXTERNAL1 = 14,
    FLOW_GLOBAL_VARIABLE_PRESSURE = 15
};

// Native global variables

extern const char *get_var_date();
extern void set_var_date(const char *value);
extern const char *get_var_humidity();
extern void set_var_humidity(const char *value);
extern const char *get_var_temperature();
extern void set_var_temperature(const char *value);
extern const char *get_var_hour1();
extern void set_var_hour1(const char *value);
extern const char *get_var_wind_speed();
extern void set_var_wind_speed(const char *value);
extern const char *get_var_weather_description();
extern void set_var_weather_description(const char *value);
extern const char *get_var_hour2();
extern void set_var_hour2(const char *value);
extern const char *get_var_hour3();
extern void set_var_hour3(const char *value);
extern const char *get_var_hour4();
extern void set_var_hour4(const char *value);
extern const char *get_var_temperature_hour1();
extern void set_var_temperature_hour1(const char *value);
extern const char *get_var_temperature_hour2();
extern void set_var_temperature_hour2(const char *value);
extern const char *get_var_temperature_hour3();
extern void set_var_temperature_hour3(const char *value);
extern const char *get_var_temperature_hour4();
extern void set_var_temperature_hour4(const char *value);
extern const char *get_var_temperature_external1();
extern void set_var_temperature_external1(const char *value);
extern const char *get_var_humidity_external1();
extern void set_var_humidity_external1(const char *value);
extern const char *get_var_pressure();
extern void set_var_pressure(const char *value);

extern uint8_t get_var_icon_index();
extern uint8_t* get_var_iconHours();
extern uint16_t get_wind_direction();

extern int8_t* get_var_temperatures();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/