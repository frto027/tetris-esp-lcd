#include <stdio.h>
#include <driver/uart.h>
#include <string.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "lcd.h"
#include "btn.h"
#include "wifiinit.h"
#include "libftetris.h"

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

FTE_GAME game;
FTE_7BAG bag;

fte_tetromino_t queue[6];
fte_tetromino_t hold;
int holded;
int move_next_fail_count;

unsigned int tick;

// cache 6 tetromino to show next
fte_tetromino_t PickItem()
{
    fte_tetromino_t ret = queue[0];
    for (int i = 0; i < 5; i++)
    {
        queue[i] = queue[i + 1];
    }
    queue[5] = fte7BagGenItem(&bag);
    return ret;
}

int died = 0;

void MySpawnNext()
{
    int count;
    died |= fteSpawnNext(&game, PickItem(), &count) == FTE_RESULT_MOVE_FAILED;
    move_next_fail_count = 0;
    holded = 0;
    if (count == 4)
    {
        scrScore += 70;
    }
}

void MyHold()
{
    if (holded)
    {
        return;
    }
    if (hold == FTE_TETROMINO_NONE)
    {
        hold = fteGetCurrentTetromino(&game);
        fteReplaceCurrentTetromino(&game, PickItem());
    }
    else
    {
        fte_tetromino_t nhold = fteGetCurrentTetromino(&game);
        fteReplaceCurrentTetromino(&game, hold);
        hold = nhold;
    }
    holded = 1;
}

static unsigned int first_pressed[10] = {0};
static unsigned int last_repeat[10] = {0};

int RepeatCheckInput(int x)
{
#define FIRST_DURATION 40
#define REPEAT_DURATION 10
    if (!first_pressed[x])
    {
        first_pressed[x] = last_repeat[x] = tick;
        return 1;
    }
    if (first_pressed[x] + FIRST_DURATION > tick)
        return 0;
    if (last_repeat[x] + REPEAT_DURATION > tick)
        return 0;
    last_repeat[x] = tick;
    return 1;
}
void ResetCheckInput(int x)
{
    first_pressed[x] = 0;
    last_repeat[x] = 0;
}

void HandleInput()
{
    if (died)
    {
        return;
    }

    if (IS_BTN(BTN_L))
    {
        if (RepeatCheckInput(0) && fteMoveLeft(&game))
            move_next_fail_count = 0;
    }
    else
        ResetCheckInput(0);
    if (IS_BTN(BTN_R))
    {
        if (RepeatCheckInput(1) && fteMoveRight(&game))
            move_next_fail_count = 0;
    }
    else
        ResetCheckInput(1);
    if (IS_BTN(BTN_D))
    {
        if (RepeatCheckInput(2))
            fteMoveDown(&game);
    }
    else
        ResetCheckInput(2);
    if (IS_BTN(BTN_U))
    {
        if (RepeatCheckInput(3))
        {
            while (fteMoveDown(&game) != FTE_RESULT_MOVE_FAILED)
                ;
            MySpawnNext();
        }
    }
    else
        ResetCheckInput(3);
    if (IS_BTN(BTN_ROT_L))
    {
        if (RepeatCheckInput(4) && fteRotLeft(&game) != FTE_RESULT_MOVE_FAILED)
            move_next_fail_count = 0;
    }
    else
        ResetCheckInput(4);
    if (IS_BTN(BTN_ROT_R))
    {
        if (RepeatCheckInput(5) && fteRotRight(&game) != FTE_RESULT_MOVE_FAILED)
            move_next_fail_count = 0;
    }
    else
        ResetCheckInput(5);
    if (IS_BTN(BTN_SOUND))
    {
        if (RepeatCheckInput(6))
            MyHold(&game);
    }
    else
        ResetCheckInput(6);
}

void GameStart(unsigned int seed)
{
    fteGameInit(&game);
    fte7BagInit(&bag, seed);
    fteGameSetGhost(&game, 0);
    for (int i = 0; i < 6; i++)
    {
        queue[i] = fte7BagGenItem(&bag);
    }
    hold = FTE_TETROMINO_NONE;
    MySpawnNext();
}

void UpdateFrame()
{
    static unsigned long long lasttime = 0;
    static int fail_count = 0;
    unsigned long long thistime = tick;

    HandleInput();

    if (thistime - lasttime > 100)
    {
        lasttime = thistime;
        if (fteMoveDown(&game) == FTE_RESULT_MOVE_FAILED)
        {
            move_next_fail_count++;
        }
        if (move_next_fail_count >= 3)
        {
            MySpawnNext();
        }
    }

    for (int i = 20; i >= 0; i--)
    {
        for (int j = 0; j < FTE_WIDTH; j++)
        {
            scrBuffer[j][i] = !!game.colors[j][i];
        }
    }
    scrGameOver = died;
    scrNextTetrimino = hold;
}

void tetrisTask(void *p)
{
    vTaskDelay(20 / portTICK_PERIOD_MS);
    htOn();
    tick = 0;
    GameStart(20);
    while (1)
    {
        UpdateFrame();
        flushScene();
        vTaskDelay(10 / portTICK_PERIOD_MS);
        tick += 10;
    }
}
esp_err_t data_handler(httpd_req_t *req)
{
    static char buff[512];

    /* 发送回简单的响应数据包 */
    char queue_str[7];
    for(int i=0;i<6;i++){
        queue_str[i] = ("-IOTSZJL")[queue[i]];
    }
    queue_str[6] = '\0';
    sprintf(buff, "{\"next\":\"%s\"}", queue_str);
    httpd_resp_send(req, buff, strlen(buff));
    return ESP_OK;
}

void app_main(void)
{
    btnInit();
    lcdInit();
    wifiinit();
    xTaskCreate(tetrisTask, "tetris_task", 2000, NULL, tskIDLE_PRIORITY, &tetrisTaskHandle);
}