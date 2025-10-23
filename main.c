#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdbool.h>

#define FREQ 125
#define TOP_FREQ 999

#define SW2 7 // right button - decreases brightness
#define SW1 8 // middle button
#define SW0 9 // left button - increases brightness
#define BUTTONS_SIZE 3

#define D1 22 // right LED
#define D2 21 // middle LED
#define D3 20 // left LED
#define LEDS_SIZE 3

void ini_buttons(const uint *buttons);
void ini_leds(const uint *leds, uint brightness);
bool light_switch(const uint *leds, uint brightness, bool on);
void set_brightness(const uint *leds, uint brightness);
uint clamp(int br);

int main() {
    const uint buttons[] = {SW2, SW1, SW0};
    const uint leds[] = {D1, D2, D3};
    uint brightness = 500;

    // Initialize chosen serial port
    stdio_init_all();
    // Initialize buttons
    ini_buttons(buttons);
    // Initialize LED pins
    ini_leds(leds, brightness);

    bool previous_state = true;
    bool lightsOn = false;

    while (true) {

        const bool sw1_state = gpio_get(SW1);
        if (previous_state && !sw1_state) {
            // Turn lights on
            if (!lightsOn) {
                if (brightness <= 0) brightness = 500;
                lightsOn = light_switch(leds, brightness, true);
            }
            // Turn lights off
            else
                lightsOn = light_switch(leds, 0, false);
        }

        if (lightsOn) {
            // Increase lighting
            if (!gpio_get(SW2)) {
                printf("Brightness before: %d\r\n", brightness);
                brightness = clamp((int)brightness - 40);
                set_brightness(leds, brightness);
                printf("Brightness after: %d\r\n", brightness);
            }
            // Decrease lighting
            if (!gpio_get(SW0)) {
                printf("Brightness before: %d\r\n", brightness);
                brightness = clamp((int)brightness + 40);
                set_brightness(leds, brightness);
                printf("Brightness after: %d\r\n", brightness);
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

void ini_leds(const uint *leds, const uint brightness) {
    for (int i = 0; i < 3; i++) {
        gpio_set_function(leds[i], GPIO_FUNC_PWM);
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]); // Get slice
        const uint chan = pwm_gpio_to_channel(leds[i]); // Channel GPIO pin
        //pwm_set_enabled(slice_num, false); // Stop PWM

        pwm_config config = pwm_get_default_config(); // Get default PWM configuration
        pwm_config_set_clkdiv_int(&config, FREQ); // Set clock divider
        pwm_config_set_wrap(&config, TOP_FREQ); // Set wrap (TOP)

        pwm_init(slice_num, &config, false); // start set to false
        pwm_set_chan_level(slice_num, chan, brightness); // duty cycle
        //gpio_set_function(leds[i], GPIO_FUNC_PWM);
        pwm_set_enabled(leds[i], true); // Start PWM
    }
}

bool light_switch(const uint *leds, const uint brightness, const bool on) {

    for (int i = 0; i < LEDS_SIZE; i++) {
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]);
        const uint chan = pwm_gpio_to_channel(leds[i]);
        pwm_set_chan_level(slice_num, chan, brightness);
        if (on) pwm_set_enabled(slice_num, on);
    }
    if (on) {
        printf("Lights on\r\n");
        return true;
    }
    printf("Lights off\r\n");
    return false;
}

void set_brightness(const uint *leds, const uint brightness) {
    for (int i = 0; i < LEDS_SIZE; i++) {
        const uint slice_num = pwm_gpio_to_slice_num(leds[i]);
        const uint chan = pwm_gpio_to_channel(leds[i]);
        pwm_set_chan_level(slice_num, chan, brightness);
    }
}

uint clamp(const int br) {
    if (br < 0)
        return 0;

    if (br > TOP_FREQ)
        return TOP_FREQ;

    return br;
}