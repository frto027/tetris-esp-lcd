#include "lcd.h"

#define CS(x) gpio_set_level(HT_CS, x)
#define WR(x) gpio_set_level(HT_WR, x)
#define DATA(x) gpio_set_level(HT_DATA, x)
#define RD(x) gpio_set_level(HT_RD,x)
#define DELAY_NS(x) vTaskDelay(configTICK_RATE_HZ * x /1000/1000/1000)
#define TCLK do{for(int kk=0;kk<300;kk++) asm("nop");}while(0)
#define TSU do{for(int kk=0;kk<10;kk++) asm("nop");}while(0)
#define TSU1 do{for(int kk=0;kk<50;kk++) asm("nop");}while(0)
#define TCS TSU1
#define SEND(x) DATA(x); TSU; WR(0); TCLK; WR(1); TCLK;

void htSendCMD(uint8_t cmd){
    CS(0); TSU1;
    SEND(1); SEND(0); SEND(0);
    for(int i=7;i>=0;i--){
        SEND(1&(cmd>>i));
    }
    SEND(0);
    CS(1);TSU1;
}

void htOn(){
    htSendCMD(HT_CMD_SYS_EN);
    htSendCMD(HT_CMD_LCD_ON);
}
void htOff(){
    htSendCMD(HT_CMD_LCD_OFF);
    htSendCMD(HT_CMD_SYS_DIS);
}

void htWrite(uint8_t address, uint8_t *data, uint32_t size_in_bit){
    CS(0); TSU1;
    DATA(1); TSU; WR(0); TCLK; WR(1); TCLK;
    DATA(0); TSU; WR(0); TCLK; WR(1); TCLK;
    DATA(1); TSU; WR(0); TCLK; WR(1); TCLK;
    for(int i=7;i>=0;i--){
        DATA(1&(address>>i)); TSU; WR(0); TCLK; WR(1); TCLK;
    }
    for(uint32_t i=0;i<size_in_bit;i++){
        DATA(1&(data[i/8]>>(i%8))); TSU; WR(0); TCLK; WR(1); TCLK;
    }
    CS(1);TCS;
    
}

uint8_t scrBuffer[10][20];
int scrSpeedNum = 0, scrLevelNum = 0; /*0 ~ 19*/
int scrScore = 0; /*0000 ~ 9999*/
/* booleans */
int scrSpeedLevelMan = 0;
int scrHandUp = 0, scrHandDown = 0;
int scrHigh = 0;
int scrScore00 = 0;
int scrMusic = 0;
int scrPause = 0, scrGameOver = 0;

int scrNextTetrimino = 0;

#include "lcdNum.inl"

void flushScene(){
    uint8_t line[2];
    for(int i=0;i<8;i++){
        line[0]=line[1]=0;
        for(int j=0;j<10;j++){
            if(scrBuffer[j][i]){
                line[j/8] |= 1 << (j%8);
            }
        }
        htWrite(i*4,line,16);
    }
    for(int i=8;i<16;i++){
        line[0]=line[1]=0;
        for(int j=0;j<10;j++){
            if(scrBuffer[j][i]){
                line[j/8] |= 1 << (j%8);
            }
        }
        htWrite(188-(i-8)*4,line,16);
    }
    for(int i=16;i<20;i++){
        line[0]=line[1]=0;
        for(int j=0;j<10;j++){
            if(scrBuffer[j][i]){
                line[j/8] |= 1 << (j%8);
            }
        }
        htWrite(68-(i-16)*4,line,16);
    }

    union LcdData lcdData = {};

    if(scrSpeedNum / 10)
        lcdData.a_34 |= 2;
    if(scrLevelNum / 10)
        lcdData.a_38 |= 2;
    
    lcd_data_speed(&lcdData, scrSpeedNum % 10);
    lcd_data_level(&lcdData, scrLevelNum % 10);

    int score = scrScore;
    lcd_data_digit_4(&lcdData, score % 10);
    score /= 10;
    lcd_data_digit_3(&lcdData, score % 10);
    score /= 10;
    lcd_data_digit_2(&lcdData, score % 10);
    score /= 10;
    lcd_data_digit_1(&lcdData, score % 10);
    
    if(scrSpeedLevelMan)
        lcdData.a_36 |= 2;
    if(scrHandUp)
        lcdData.a_36 |= 1;
    if(scrHandDown)
        lcdData.a_40 |= 1;
    if(scrHigh)
        lcdData.a_40 |= 64;
    if(scrScore00)
        lcdData.a_40 |= 16;
    if(scrMusic)
        lcdData.a_40 |= 4;
    if(scrPause)
        lcdData.a_32 |= 2;
    if(scrGameOver)
        lcdData.a_32 |= 1;
    
    switch (scrNextTetrimino)
    {
    case 1:
        lcd_data_tetromino_I(&lcdData);
        break;
    case 2:
        lcd_data_tetromino_O(&lcdData);
        break;
    case 3:
        lcd_data_tetromino_T(&lcdData);
        break;
    case 4:
        lcd_data_tetromino_S(&lcdData);
        break;
    case 5:
        lcd_data_tetromino_Z(&lcdData);
        break;
    case 6:
        lcd_data_tetromino_J(&lcdData);
        break;
    case 7:
        lcd_data_tetromino_L(&lcdData);
        break;
    }

    htWrite(LCD_DATA_ADDR_BEGIN, lcdData.data, LCD_DATA_SIZE_IN_BITS);
}

void lcdInit(){
    gpio_config_t ht_gpio_config = {
        .pin_bit_mask = (1llu<<HT_CS) | (1llu<<HT_RD) | (1llu<<HT_WR) | (1llu<<HT_DATA),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&ht_gpio_config);

    gpio_set_level(HT_CS, 1);
    gpio_set_level(HT_RD,1);
    gpio_set_level(HT_WR,1);
    gpio_set_level(HT_DATA,1);

}