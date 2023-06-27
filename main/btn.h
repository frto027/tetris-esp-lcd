#ifndef BTN_H
#define BTN_H

#include <driver/gpio.h>
#include <stdint.h>

#define BTN_DEF(NAME, GPIO) NAME = GPIO,
enum BTNS{
    #include "btn.inl"
};
#undef BTN_DEF

void btnInit();

#define IS_BTN(BTN) (!gpio_get_level(BTN))

#endif