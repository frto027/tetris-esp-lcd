#include <stdio.h>
#include <driver/uart.h>
#include <driver/gptimer.h>
#include <string.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "lcd.h"
#include "btn.h"
#include "wifiinit.h"
#include "libftetris.h"
#include "esp_timer.h"

#include "badapple_midi.inc"

#define BEEP_GPIO GPIO_NUM_19
#define BEEP_BASE 1000*1000
TaskHandle_t tetrisTaskHandle;
void setBeepFreq(int freq);
void test(int addr, uint8_t bit)
{
    htWrite(addr, &bit, 4);
    printf("turn on address %d, value %d\n", addr, bit);
}
void detest(int addr)
{
    uint8_t line[2] = {0, 0};
    htWrite(addr, line, 4);
    printf("turn off address %d\n", addr);
}

#include "badapple.inc"

int current_frame = 0;

void UpdateFrame()
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            scrBuffer[j][i] = !!(badapple[current_frame][i] & (1<<j));
        }
    }

    current_frame = (current_frame + 1) % BADAPPLE_FRAME_COUNT;
}

void tick(void *arg){
    UpdateFrame();
    if(current_frame >= 30*30 && current_frame % 60 == 0){
        if(current_frame == 30*30){
            scrSpeedLevelMan = 1;
            scrHandUp = 1;
        }else{
            scrHandUp = !scrHandUp;
            scrHandDown = !scrHandDown;
        }
        if(current_frame >= 180*30){
            scrHandDown = scrHandUp = 1;
        }
        flushScene(false);
    }else{
        flushScene(true);
    }

    if(current_frame < sizeof(badapple_freqs) / sizeof(*badapple_freqs)){
        setBeepFreq(badapple_freqs[current_frame]);
    }else{
        setBeepFreq(0);
    }
}
esp_timer_create_args_t timer_create_args = {
    .callback = &tick,
    .name="tick-timer"
};
esp_timer_handle_t timer_handle;

void tetrisTask(void *p)
{
    vTaskDelay(20 / portTICK_PERIOD_MS);
    htOn();
    scrSpeedLevelMan = 0;
    scrHandUp = 0;
    flushScene(false);

    // esp_timer_create(&timer_create_args, &timer_handle);
    // esp_timer_start_periodic(timer_handle, 1000/60);

    while(1){
        tick(NULL);
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}
esp_err_t data_handler(httpd_req_t *req)
{
    return ESP_OK;
}
gptimer_handle_t gptimer = NULL;

typedef struct {
    uint64_t event_count;
} example_queue_element_t;

static bool beep_switch(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    static bool cur = false;
    gpio_set_level(BEEP_GPIO, cur);
    cur = !cur;
    return true;
}

gptimer_alarm_config_t alarm_config = {
    .reload_count = 0, // counter will reload with 0 on alarm event
    .alarm_count = 1000, // period = 1s @resolution 1MHz
    .flags.auto_reload_on_alarm = true, // enable auto-reload
};


void beepInit(){
    gpio_config_t beep_gpio_cfg = {
        .pin_bit_mask = (1llu<<BEEP_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&beep_gpio_cfg);

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * BEEP_BASE, 
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = beep_switch, // register user callback
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void setBeepFreq(int freq){
    static int last_freq = 1;
    if(freq == last_freq)
        return;
    if(last_freq && !freq){
        gptimer_stop(gptimer);
    }
    if(freq && !last_freq){
        gptimer_start(gptimer);
    }
    last_freq = freq;
    if(freq){
        alarm_config.alarm_count = BEEP_BASE / freq / 2;
        if(alarm_config.alarm_count == 0){
            alarm_config.alarm_count = 1;
        }
        ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    }
}

void app_main(void)
{
    btnInit();
    lcdInit();
    esp_timer_init();
    beepInit();
    // wifiinit();
    xTaskCreate(tetrisTask, "tetris_task", 2000, NULL, tskIDLE_PRIORITY, &tetrisTaskHandle);
}