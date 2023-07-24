#include <stdio.h>
#include <driver/uart.h>
#include <string.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "lcd.h"
#include "btn.h"
#include "wifiinit.h"
#include "libftetris.h"
#include "esp_timer.h"

TaskHandle_t tetrisTaskHandle;

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

void app_main(void)
{
    btnInit();
    lcdInit();
    esp_timer_init();
    // wifiinit();
    xTaskCreate(tetrisTask, "tetris_task", 2000, NULL, tskIDLE_PRIORITY, &tetrisTaskHandle);
}