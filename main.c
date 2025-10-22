#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdbool.h>

#define SW2 7 // right button - decreases brightness
#define SW1 8 // middle button
#define SW0 9 // left button - increases brightness
#define BUTTONS_SIZE 3

#define D1 22 // right LED
#define D2 21 // middle LED
#define D3 20 // left LED
#define LEDS_SIZE 3

void ini_buttons(const uint *buttons);
void ini_leds(const uint *leds);
bool lights_on(const uint *leds, uint8_t brightness);
bool lights_off(const uint *leds);
void set_brightness(const uint *leds, uint8_t brightness);
uint8_t clamp8(int br);

int main() {
    const uint buttons[] = {SW2, SW1, SW0};
    const uint leds[] = {D1, D2, D3};

    uint8_t brightness = 200;

    // Initialize chosen serial port
    stdio_init_all();
    // Initialize buttons
    ini_buttons(buttons);
    // Initialize LED pins
    ini_leds(leds);

    bool previous_state = true;
    bool lightsOn = false;

    while (true) {

        bool sw1_state = gpio_get(SW1);
        if (previous_state && !sw1_state) {
            if (!lightsOn)
                lightsOn = lights_on(leds, brightness);
            else
                lightsOn = lights_off(leds);
        }

        if (lightsOn) {
            if (!gpio_get(SW2)) {
                brightness = clamp8(brightness - 20);
                set_brightness(leds, brightness);
            }

            if (!gpio_get(SW0)) {
                brightness = clamp8(brightness + 20);
                set_brightness(leds, brightness);
            }
        }

        sleep_ms(250);
        previous_state = sw1_state;
    }
}

void ini_buttons(const uint *buttons) {
    for (int i = 0; i < BUTTONS_SIZE; i++) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }
}

void ini_leds(const uint *leds) {
    for (int i = 0; i < 3; i++) {
        gpio_set_function(leds[i], GPIO_FUNC_PWM);
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_wrap(&config, 255);
        pwm_init(slice_num, &config, true);
    }
}

bool lights_on(const uint *leds, const uint8_t brightness) {
    for (int i = 0; i < LEDS_SIZE; i++) {
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]);
        pwm_set_enabled(slice_num, true);
        pwm_set_gpio_level(leds[i], brightness);
    }
    return true;
}

bool lights_off(const uint *leds) {
    for (int i = 0; i < LEDS_SIZE; i++) {
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]);
        pwm_set_enabled(slice_num, false);
    }
    return false;
}

void set_brightness(const uint *leds, const uint8_t brightness) {
    for (int i = 0; i < LEDS_SIZE; i++) {
        pwm_set_gpio_level(leds[i], brightness);
    }
}

uint8_t clamp8(const int br) {
    if (br < 0)
        return 0;

    if (br > 255)
        return 255;

    return br;
}