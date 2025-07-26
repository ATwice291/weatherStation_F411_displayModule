
#include "main.h"
#include "ThreadAttributes.hpp"
#include "lvgl.h"
#include "ui.h"

extern "C"
{
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
}

extern MCP2515 ExtCan;
extern ST7735 display;

bool isDisplayInit;

lv_display_t* disp_drv;

static uint8_t buf_2_1[160 * 64 * 2];                        /*A buffer for 10 rows*/
//static uint8_t buf_2_2[160 * 32 * 2]; 

struct FreeRTOSMsTimerImpl
{
  static uint32_t getMs()
  {
    return osKernelGetTickCount();
  }
};

CanMessage msgRx;

osMessageQueueId_t canMessagesQueue;
const osMessageQueueAttr_t QueueCan_attributes = {
       .name = "QueueCan"
};
static constexpr uint32_t MAX_CAN_MESSAGES = 32;

uint16_t outsideHumid;
int16_t outsideTemp;

uint8_t hours;
uint8_t minutes;
uint8_t seconds;
uint8_t day;
uint8_t month;
uint16_t year;
uint8_t dayOfWeek;

uint8_t iconIndex;
int8_t temperature;
uint8_t humidity;
uint16_t pressure;
uint8_t windDegrees;
uint16_t windSpeed;

osThreadId_t guiTaskHandle;
const ThreadAttributes guiTask_attributes("guiTask", 2048 * 4, osPriorityNormal);

osThreadId_t flashTaskHandle;
const ThreadAttributes flashTask_attributes("flashTask", 256 * 4, osPriorityNormal);

osThreadId_t myTask03Handle;
const ThreadAttributes myTask03_attributes("myTask03", 1024 * 4, osPriorityNormal);

void StartGUITask(void *argument);
void StartFlashTask(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

extern volatile unsigned long taskTime;
extern uint32_t SystemCoreClock;

extern "C" void configureTimerForRunTimeStats(void);
extern "C" unsigned long getRunTimeCounterValue(void);
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
extern "C" void vApplicationTickHook(void);

extern "C" void configureTimerForRunTimeStats(void) { taskTime = 0; }
extern "C" unsigned long getRunTimeCounterValue(void) { return taskTime; }
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void) xTask;
  (void) pcTaskName;
  __disable_irq();
  while (1);  // Зависаем тут — можно анализировать по отладчику
}

void led_toggle();
bool modeButtonPressed();
bool plusButtonPressed();
bool minusButtonPressed();
void monitorFlush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
void displayReady();

char temperatureBuffer[10];
char humidBuffer[10];
char pressureBuffer[15];
char dateBuffer[20];
char windSpeedBuffer[10];
char weatherDescription[64] = "Long, long weather description";
char hour[4][5];
char temperature_hour[4][8];
int8_t temperatures[4];
uint8_t iconIndex_hour[4];
uint16_t precipitation_hour[4];

char temperatureExternal1Buffer[10];
char humidExternal1Buffer[10];

void fillDateTimeBuffer();
void fillTemperatureBuffer();

extern "C" void vApplicationTickHook(void) {
  ++taskTime;
  lv_tick_inc(1);
}
void MX_FREERTOS_Init(void)
{
  isDisplayInit = false;

  canMessagesQueue = osMessageQueueNew(MAX_CAN_MESSAGES, sizeof(CanMessage), &QueueCan_attributes);

  guiTaskHandle = osThreadNew(StartGUITask, NULL, &guiTask_attributes);
  flashTaskHandle = osThreadNew(StartFlashTask, NULL, &flashTask_attributes);
  myTask03Handle = osThreadNew(StartTask03, NULL, &myTask03_attributes);
}

void StartGUITask(void *argument)
{
  (void)argument;
  
  
  display.init<FreeRTOSMsTimerImpl>(St7735DisplayRotation::VERTICAL);
  display.setDisplayReadyCallback(displayReady);
  lv_init();

  disp_drv = lv_display_create(display.getWidth(), display.getHeight());

  //lv_display_set_buffers(disp_drv, buf_2_1, buf_2_2, 160*32*2,
 //                           LV_DISPLAY_RENDER_MODE_PARTIAL);
                            
  lv_display_set_buffers(disp_drv, buf_2_1, NULL, 160*64*2,
                            LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_display_set_flush_cb(disp_drv, monitorFlush);
  
  ui_init();

  osDelay(1);
  isDisplayInit = true;
  //uint32_t shift = 0;
  for (;;)
  {
	  lv_timer_handler();
    ui_tick();
    //display.drawImage(0, 0, 160, 128, (const uint16_t *)(0x08000000 + shift));
    //shift += 0x10;
    osDelay(5);
  }
}

void StartFlashTask(void *argument)
{
  (void)argument;
  for(;;) {
    osDelay(1);
    if(isDisplayInit) {break;}
  }
  for (;;)
  {
    //led_toggle();
    osDelay(1);
  }
}

void checkInputMessage(CanMessage msg) {
  if (msgRx.id == 0x111 || msgRx.id == 0x710) {
    outsideTemp = static_cast<int16_t>(msgRx.data[1] << 8 | msgRx.data[0]);
    outsideHumid = static_cast<uint16_t>(msgRx.data[3] << 8 | msgRx.data[2]);
  }
  if (msgRx.id == 0x011) {
    hours = msgRx.data[0];
    minutes = msgRx.data[1];
    seconds = msgRx.data[2];
    day = msgRx.data[3]; 
    month = msgRx.data[4]+1;
    year = 1900 + msgRx.data[5];
    dayOfWeek = msgRx.data[6];
    fillDateTimeBuffer();
  }
  if (msgRx.id == 0x013) {
    uint8_t stringPart = msgRx.data[0] * 7;
    
    uint8_t usefulSymbols = (msgRx.len > 1)?msgRx.len:0;
    uint8_t index = 1;
    while (usefulSymbols) {
      weatherDescription[stringPart] = msgRx.data[index];
      ++stringPart;
      ++index;
      --usefulSymbols;
    }
  }
  if (msgRx.id == 0x015) {
    iconIndex = msgRx.data[0];
    temperature = static_cast<int8_t>(msgRx.data[1]);
    //fillTemperatureBuffer();
    lv_snprintf(temperatureBuffer, 9, "%i%c%c", temperature, 0xC2, 0xB0);
    humidity = msgRx.data[2];
    lv_snprintf(humidBuffer, 9, "%i%%", humidity);
    pressure = (msgRx.data[3]<<8)+msgRx.data[4]; 
    lv_snprintf(pressureBuffer, 14, "%ihPa", pressure);
    windDegrees = msgRx.data[5];
    windSpeed = (msgRx.data[6]<<8)+msgRx.data[7]; 
    lv_snprintf(windSpeedBuffer, 9, "%d.%01dm/s", windSpeed/10, windSpeed%10);
  }
  if (msgRx.id == 0x017) {
    uint8_t ind = msgRx.data[0];
    if(ind > 3) ind = 3;
    lv_snprintf(hour[ind], 5, "%02dh", msgRx.data[1]);
    iconIndex_hour[ind] = msgRx.data[2];
    temperatures[ind] = static_cast<int8_t>(msgRx.data[3]);
    lv_snprintf(temperature_hour[ind], 9, "%i", temperatures[ind]);
    precipitation_hour[ind] = (msgRx.data[4]<<8)+msgRx.data[5];
  }
}

void StartTask03(void *argument) {
  (void)argument;
  ExtCan.init<MyCanParams>();

  CanMessage msgExtSensorRequest;
  msgExtSensorRequest.id = 0x110;
  msgExtSensorRequest.len = 0;

  CanMessage msgEspTimeRequest;
  msgEspTimeRequest.id = 0x010;
  msgEspTimeRequest.len = 0;

  CanMessage msgEspWeatherDescriptionRequest;
  msgEspWeatherDescriptionRequest.id = 0x012;
  msgEspWeatherDescriptionRequest.len = 0;

  CanMessage msgEspWeatherRequest;
  msgEspWeatherRequest.id = 0x014;
  msgEspWeatherRequest.len = 0;

  CanMessage msgEspForecastRequest;
  msgEspForecastRequest.id = 0x016;
  msgEspForecastRequest.len = 0;

  for (;;) {
    if(MCP2515_INT::read() == 0) {
      ExtCan.init<MyCanParams>();
    }
    ExtCan.sendMessage(0, msgExtSensorRequest);
    osDelay(1);
    ExtCan.sendMessage(0, msgEspTimeRequest);
    osDelay(1);
    ExtCan.sendMessage(0, msgEspWeatherDescriptionRequest);
    osDelay(1);
    ExtCan.sendMessage(0, msgEspWeatherRequest);
    osDelay(1);
    ExtCan.sendMessage(0, msgEspForecastRequest);
    osDelay(1);

    while(osMessageQueueGet(canMessagesQueue, &msgRx, NULL, 0) == osOK) {
      checkInputMessage(msgRx);
    }
    osDelay(200);
  }
}

bool modeButtonPressed()
{
  static bool lastState = false;
  bool currState = ModeButton::read();
  bool pressed = lastState && !currState;
  lastState = currState;
  return pressed;
}

bool plusButtonPressed()
{
  static bool lastState = false;
  bool currState = PlusButton::read();
  bool pressed = lastState && !currState;
  lastState = currState;
  return pressed;
}

bool minusButtonPressed()
{
  static bool lastState = false;
  bool currState = MinusButton::read();
  bool pressed = lastState && !currState;
  lastState = currState;
  return pressed;
}

void led_toggle()
{
  Led1::set();
  Led2::reset();
  osDelay(500);

  Led1::reset();
  Led2::set();
  osDelay(500);
}

void monitorFlush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
  LV_UNUSED(disp);
	if(area->x2 < 0) return;
	if(area->y2 < 0) return;
	if(area->x1 > display.getWidth()) return;
	if(area->y1 > display.getHeight()) return;

  uint16_t w = area->x2-area->x1+1;
  uint16_t h = area->y2-area->y1+1;

  //lv_draw_sw_rgb565_swap(px_map, w*h);

  display.drawImage(area->x1, area->y1, w, h, (const uint16_t *)px_map);
}

void displayReady() {
  lv_display_flush_ready(disp_drv);
}

char* addSymbol(char *buf, char c) {
    buf[0] = c;
    return buf+1;
}

char* format2digit(char *buf, int value) {
    buf[0] = '0' + (value / 10);
    buf[1] = '0' + (value % 10);
    return buf+2;
}

char* format4digit(char *buf, int value) {
    buf[0] = '0' + (value / 1000) % 10;
    buf[1] = '0' + (value / 100) % 10;
    buf[2] = '0' + (value / 10) % 10;
    buf[3] = '0' + (value % 10);
    return buf+4;
}

void fillDateTimeBuffer() {
  char* buf = dateBuffer;
  buf = format2digit(buf, day);
  buf = addSymbol(buf, '.');
  buf = format2digit(buf, month);
  buf = addSymbol(buf, '.');
  buf = format2digit(buf, year%100);
  buf = addSymbol(buf, ' ');
  buf = addSymbol(buf, ' ');
  buf = format2digit(buf, hours);
  buf = addSymbol(buf, ':');
  buf = format2digit(buf, minutes);
  buf = addSymbol(buf, 0);
}

void fillTemperatureBuffer() {
  char* buf = temperatureBuffer;
  int8_t tmp = temperature;
  if(tmp < 0) {
    buf = addSymbol(buf, '-');
    tmp = -tmp;
  }
  buf = format2digit(buf, tmp);
  buf = addSymbol(buf, 0xC2);
  buf = addSymbol(buf, 0xB0);
  buf = addSymbol(buf, 0);
}

extern "C" char* get_var_pressure() {return pressureBuffer; }

extern "C" const char *get_var_date() {return dateBuffer; }
extern "C" const char *get_var_humidity() {return humidBuffer; }
extern "C" const char *get_var_temperature() {return temperatureBuffer; }

extern "C" uint8_t get_var_icon_index() {return iconIndex; }
extern "C" uint8_t* get_var_iconHours() {return iconIndex_hour; }

extern "C" uint16_t get_wind_direction() {
  uint16_t windDirection = (windDegrees * 100 + 1800) % 3600;
  return windDirection;
}

extern "C" const char *get_var_wind_speed() { return windSpeedBuffer; }

extern "C" const char *get_var_weather_description() { return weatherDescription; }

extern "C" const char *get_var_hour1() { return hour[0]; }
extern "C" const char *get_var_hour2() { return hour[1]; }
extern "C" const char *get_var_hour3() { return hour[2]; }
extern "C" const char *get_var_hour4() { return hour[3]; }

extern "C" const char *get_var_temperature_hour1() { return temperature_hour[0]; }
extern "C" const char *get_var_temperature_hour2() { return temperature_hour[1]; }
extern "C" const char *get_var_temperature_hour3() { return temperature_hour[2]; }
extern "C" const char *get_var_temperature_hour4() { return temperature_hour[3]; }

extern "C" int8_t* get_var_temperatures() { return temperatures; }

extern "C" const char *get_var_temperature_external1() { return temperatureExternal1Buffer; }
extern "C" const char *get_var_humidity_external1() { return humidExternal1Buffer; }