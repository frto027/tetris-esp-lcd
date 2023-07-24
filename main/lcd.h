#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <driver/gpio.h>

#define HT_CMD_SYS_DIS 0x00
#define HT_CMD_SYS_EN  0x01
#define HT_CMD_LCD_OFF 0x02
#define HT_CMD_LCD_ON  0x03

#define HT_CS GPIO_NUM_5
#define HT_RD GPIO_NUM_17
#define HT_WR GPIO_NUM_16
#define HT_DATA GPIO_NUM_4


/*
GAMEOVER address 32, value 1
PAUSE    address 32, value 2

SPEED_LEVEL_MAN 36.2

HEAD_UP     36.1
HEAD_DOWN   40.1

HI-         41.4
SCORE_00    41.1

MUSIC       40.4

*/

void lcdInit();

void htSendCMD(uint8_t cmd);
void htOn();
void htOff();
void flushScene(bool onlyScreen);
void htWrite(uint8_t address, uint8_t *data, uint32_t size_in_bit);

extern uint8_t scrBuffer[10][20];
extern uint8_t scrNexts[6];

extern int scrSpeedNum, scrLevelNum; /*0 ~ 19*/
extern int scrScore; /*0000 ~ 9999*/
/* booleans */
extern int scrSpeedLevelMan;
extern int scrHandUp, scrHandDown;
extern int scrHigh;
extern int scrScore00;
extern int scrMusic;
extern int scrPause, scrGameOver;

extern int scrNextTetrimino;

#endif