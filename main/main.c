#include <stdio.h>
#include <driver/uart.h>
#include <string.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "lcd.h"
#include "btn.h"

TaskHandle_t tetrisTaskHandle;

void test(int addr, uint8_t bit){
    htWrite(addr,&bit,4);
    printf("turn on address %d, value %d\n",addr,bit);
}
void detest(int addr){
    uint8_t line[2] = {0,0};
    htWrite(addr,line,4);
    printf("turn off address %d\n",addr);

}


void tetrisTask(void *p){
    vTaskDelay(20 / portTICK_PERIOD_MS);
    htOn();
    int i=0;
    while(1){
        i++;
        for(int m=0;m<10;m++)
        for(int n=0;n<20;n++)
        scrBuffer[m][n]=i%2 != (n%2 != m%2);

        scrSpeedNum = (scrSpeedNum + 1) % 20;
        scrLevelNum = (scrLevelNum + 1) % 20;

        scrScore = (scrScore + 1) % 10000;

        scrSpeedLevelMan = !scrSpeedLevelMan;
        scrHandDown = !scrSpeedLevelMan;
        scrHandUp = !scrSpeedLevelMan;
        scrHigh = scrSpeedLevelMan;
        scrScore00 = scrHigh;
        scrMusic = !scrScore00;
        scrPause = scrMusic;
        scrGameOver = !scrPause;
        scrNextTetrimino = (scrNextTetrimino + 1) % 8;

        flushScene();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    btnInit();
    lcdInit();
    xTaskCreate(tetrisTask, "tetris_task",2000, NULL, tskIDLE_PRIORITY, &tetrisTaskHandle);
}