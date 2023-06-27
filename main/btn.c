#include "btn.h"

#define BTN_DEF(NAME,GPIO) BIT64(NAME) |

const int64_t BTN_MASKS = 
    #include "btn.inl"
    0;

#undef BTN_DEF


void btnInit(){
    gpio_config_t btn_gpio_config = {
        .pin_bit_mask = BTN_MASKS,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&btn_gpio_config);
}