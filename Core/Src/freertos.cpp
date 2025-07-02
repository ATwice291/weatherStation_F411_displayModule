
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

static uint8_t buf_2_1[160 * 32 * 2];                        /*A buffer for 10 rows*/
static uint8_t buf_2_2[160 * 32 * 2]; 

struct FreeRTOSMsTimerImpl
{
  static uint32_t getMs()
  {
    return osKernelGetTickCount();
  }
};

CanMessage msgRx;

uint16_t outsideHumid;
int16_t outsideTemp;

osThreadId_t guiTaskHandle;
const ThreadAttributes guiTask_attributes("guiTask", 1024 * 4, osPriorityNormal);

osThreadId_t flashTaskHandle;
const ThreadAttributes flashTask_attributes("flashTask", 256 * 4, osPriorityNormal);

osThreadId_t myTask03Handle;
const ThreadAttributes myTask03_attributes("myTask03", 256 * 4, osPriorityNormal);

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

extern "C" const char * get_var_();
char temperatureBuffer[10];
char humidBuffer[10];
char dateBuffer[20];

extern "C" void vApplicationTickHook(void) {
  ++taskTime;
}
void MX_FREERTOS_Init(void)
{
  isDisplayInit = false;

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

  lv_display_set_buffers(disp_drv, buf_2_1, buf_2_2, 160*32*2,
                            LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp_drv, monitorFlush);
  
  ui_init();

  osDelay(1);
  isDisplayInit = true;
  //uint32_t shift = 0;
  for (;;)
  {
	  lv_timer_handler();
    lv_tick_inc(1);
    ui_tick();
    //display.drawImage(0, 0, 160, 128, (const uint16_t *)(0x08000000 + shift));
    //shift += 0x10;
    osDelay(1);
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

void StartTask03(void *argument)
{
  (void)argument;
  ExtCan.init<MyCanParams>();

  uint8_t canStatus;
  CanMessage msgTx;
  msgTx.id = 0x110;
  msgTx.len = 0;
  for (;;)
  {
    ExtCan.sendMessage(0, msgTx);
    osDelay(1);
    ExtCan.readStatus(canStatus);
    if (canStatus & 0x03)
    {
      uint8_t fifoNum = (canStatus & 0x01) ? 0 : 1;
      ExtCan.receiveMessage(fifoNum, msgRx);
      if (msgRx.id == 0x111 || msgRx.id == 0x710)
      {
        outsideTemp = static_cast<int16_t>(msgRx.data[1] << 8 | msgRx.data[0]);
        outsideHumid = static_cast<uint16_t>(msgRx.data[3] << 8 | msgRx.data[2]);
      }
    }
    osDelay(100);
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

  lv_draw_sw_rgb565_swap(px_map, w*h);

  display.drawImage(area->x1, area->y1, w, h, (const uint16_t *)px_map);
}

void displayReady() {
  lv_display_flush_ready(disp_drv);
}

extern "C" const char * get_var_() {
  uint8_t i = 0;
  int16_t tmp = outsideTemp;
  if(tmp < 0) {
    temperatureBuffer[i++] = '-';
    tmp = -tmp;
  }
  temperatureBuffer[i++] = '0'+(tmp / 1000);
  tmp = tmp % 1000;
  temperatureBuffer[i++] = '0'+(tmp / 100);
  tmp = tmp % 100;
  temperatureBuffer[i++] = '.';
  temperatureBuffer[i++] = '0'+(tmp / 10);
  //temperatureBuffer[i++] = 0xC2;
  //temperatureBuffer[i++] = 0xB0;
  //temperatureBuffer[i++] = 'C';
  temperatureBuffer[i++] = 0;
  return temperatureBuffer;
}

extern "C" char* get_var_pressure() {
  uint8_t i = 0;
  int16_t tmp = outsideHumid;
  
  humidBuffer[i++] = '0'+(tmp / 1000);
  tmp = tmp % 1000;
  humidBuffer[i++] = '0'+(tmp / 100);
  tmp = tmp % 100;
  humidBuffer[i++] = '.';
  humidBuffer[i++] = '0'+(tmp / 10);
  humidBuffer[i++] = '0'+(tmp % 10);
  humidBuffer[i++] = 0;
  return humidBuffer;
}

extern "C" const char *get_var_date() {
  uint8_t i = 0;
  dateBuffer[i++] = '2';
  dateBuffer[i++] = '2';
  dateBuffer[i++] = ':';
  dateBuffer[i++] = '5';
  dateBuffer[i++] = '9';
  dateBuffer[i++] = ':';
  dateBuffer[i++] = '2';
  dateBuffer[i++] = '2';
  dateBuffer[i++] = 0;
  return dateBuffer;
}
//extern void set_var_date(const char *value);
extern "C" const char *get_var_humidity() {
  uint8_t i = 0;
  int16_t tmp = outsideHumid;
  
  humidBuffer[i++] = '0'+(tmp / 1000);
  tmp = tmp % 1000;
  humidBuffer[i++] = '0'+(tmp / 100);
  tmp = tmp % 100;
  humidBuffer[i++] = '.';
  humidBuffer[i++] = '0'+(tmp / 10);
  humidBuffer[i++] = '0'+(tmp % 10);
  humidBuffer[i++] = 0;
  return humidBuffer;
}
//extern void set_var_humidity(const char *value);
extern "C" const char *get_var_temperature() {
  uint8_t i = 0;
  int16_t tmp = outsideTemp;
  if(tmp < 0) {
    temperatureBuffer[i++] = '-';
    tmp = -tmp;
  }
  temperatureBuffer[i++] = '0'+(tmp / 1000);
  tmp = tmp % 1000;
  temperatureBuffer[i++] = '0'+(tmp / 100);
  tmp = tmp % 100;
  temperatureBuffer[i++] = 0;
  return temperatureBuffer;
}

extern "C" uint16_t get_wind_direction() {
  static uint16_t windDirection;
  ++windDirection;
  if(windDirection >= 3600) {
    windDirection = 0;
  }
  return windDirection;
}

extern "C" const char *get_var_hour1() {
  return temperatureBuffer;
}
//extern void set_var_temperature(int32_t value);