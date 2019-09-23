#include QMK_KEYBOARD_H
#include "../../../oled.h"
#include "../../../is31fl3733.h"
#include "../../rev3.h"
#include "../../../m24m01.h"

#define _QWERTY 0
#define _LOWER 3
#define _RAISE 4
#define _ADJUST 16

enum macro_keycodes {
    KC_SAMPLEMACRO,
};

#define KC______ KC_TRNS
#define KC_XXXXX KC_NO
#define KC_LOWER LOWER
#define KC_RAISE RAISE
#define KC_RST   RESET
#define KC_EEPROM_READ KC_F24
#define KC_EEPROM_WRITE KC_F23
#define KC_EEPROM_PAGE_WRITE KC_F21
#define KC_EEPROM_DUMP KC_F22
#define KC_CTLTB CTL_T(KC_TAB)
#define KC_ALTZ  ALT_T(KC_Z)
#define KC_GUI_EQ RGUI_T(KC_EQUAL)
#define KC_GUI_QUO GUI_T(KC_QUOTE)
#define KC_ALTSLSH ALT_T(KC_SLASH)
#define KC_CTRLX CTL_T(KC_X)

enum keyboard_layers {
    QWERTY,
    LOWER,
    RAISE,
    ADJUST,
};

void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3)
{
    oled_clear();

    bool didPrint = false;
    if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
        layer_on(layer3);
        palSetPad(GPIOB, 14);
        oled_write_ln_P(PSTR("ADJST"), false);
        didPrint = true;
    } else {
        layer_off(layer3);
        palClearPad(GPIOB, 14);
    }

    if (layer_state & (1 << _LOWER)) {
        palSetPad(GPIOB, 13);
        oled_write_ln_P(PSTR("LOWER"), false);
        didPrint = true;
    } else {
        palClearPad(GPIOB, 13);
    }

    if (layer_state & (1 << _RAISE)) {
        palSetPad(GPIOB, 15);
        oled_write_ln_P(PSTR("RAISE"), false);
        didPrint = true;
    } else {
        palClearPad(GPIOB, 15);
    }

    if (!didPrint) {
        oled_write_ln_P(PSTR("BASE"), false);
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [QWERTY] = LAYOUT_kc(
        ESC,      Q,   W,     E,     R,     T, \
        GUI_EQ,   A,   S,     D,     F,     G, \
        LSHIFT,  ALTZ,   X,     C,     V,     B, \
                        LCTL,  LOWER, SPC,  BSPC, \

        Y,     U,     I,     O,     P,     BSLASH, \
        H,     J,     K,     L,     SCLN,  GUI_QUO, \
        N,     M,     COMM,  DOT,  ALTSLSH, RSHIFT, \
        TAB,  ENTER,        RAISE, RCTL \
    ),
    [_LOWER] = LAYOUT_kc( \
                          TRNS,     1,     2,     3,     4,     5, \
                          TRNS,    F1,    F2,    F3,    F4,    F5, \
                          TRNS, LALT, GRAVE,    APP, XXXXX, XXXXX, \
                                      TRNS,   TRNS,  TRNS, TRNS, \

                          6,     7,     8,     9,     0,  TRNS, \
                          F6,    F7,    F8,    F9,   F10,  TRNS, \
                          XXXXX,  LEFT,    UP,  DOWN, RIGHT,  TRNS, \
                          HOME,   END, TRNS,   TRNS
                        ),
    [_RAISE] = LAYOUT_kc( \
                          CIRC,  AMPR,  LPRN,  RPRN,  ASTR, XXXXX, \
                          MINS,   EQL,  LCBR,  RCBR,  PIPE,   GRV, \
                          UNDS,  PLUS,  LBRC,  RBRC,  BSLS,  TILD, \
                                        TRNS,   TRNS,  TRNS,  TRNS, \

                          TRNS,  TRNS, TRNS,  TRNS,  TRNS,  EEPROM_DUMP, \
                          TRNS,  TRNS, TRNS,  TRNS,  EEPROM_PAGE_WRITE,  EEPROM_WRITE, \
                          LCTRL, TRNS, TRNS,  TRNS,  TRNS,  EEPROM_READ, \
                          TRNS, TRNS, TRNS, RST
                        ),
};

uint8_t data[8] = {0,0,0,0,0,0,0,0};
uint8_t cnter = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{
    is31_state *state = &left_hand;
    uint8_t rowOffset = 0;
    if (record->event.key.row >= 4) {
        state = &right_hand;
        rowOffset = 4;
    }

    if (record->event.pressed) {
        IS31FL3733_state_set_color(state, record->event.key.row-rowOffset, record->event.key.col, !(IS_LAYER_ON(_LOWER) || IS_LAYER_ON(_RAISE)) ? 255 : 0, IS_LAYER_ON(_LOWER) ? 255 : 0, IS_LAYER_ON(_RAISE) ? 255 : 0);
        // IS31FL3733_state_configure_led_abm(state, state->matrix[record->event.key.row-rowOffset * 6 + record->event.key.col].b, IS31FL3733_LED_MODE_ABM2);
    } else {
        IS31FL3733_state_set_color(state, record->event.key.row-rowOffset, record->event.key.col, 0, 0, 0);
        // IS31FL3733_state_configure_led_abm(state, state->matrix[record->event.key.row-rowOffset * 6 + record->event.key.col].b, IS31FL3733_LED_MODE_ABM2);
    }
    IS31FL3733_state_update_pwm_buffers(state);

    switch (keycode) {
        case KC_EEPROM_PAGE_WRITE: {
            if (record->event.pressed) return false;
            cnter++;

            for (int i = 0; i < 8; i++) {
                data[i] = cnter;
            }
            uint8_t result;

            result = m24m01_page_write(0x50, 0, &data[0], sizeof(data));
            printf("wrote %d bytes to eeprom with result: %d (%d) (expected: %2X)\n", sizeof(data), result, i2c2_getErrors(), cnter);
            return false;
        }
        case KC_EEPROM_DUMP: {
            if (record->event.pressed) return false;

            uint16_t totalNumPages = 1024/128;
            uint8_t data[128];
            for (uint8_t i = 0; i < totalNumPages; i++) {
                uint8_t result = m24m01_page_read(0x50, i*128, &data[0], sizeof(data));
                printf("read %d byte from eeprom with result: %d\n", sizeof(data), result);
                for (int i = 0; i < 128; i++) {
                    printf("%2X", data[i]);
                    if (i < 127) printf(", ");
                }
                printf("\n");
            }
            return false;
        }
        case KC_EEPROM_WRITE: {
            if (record->event.pressed) return false;
            cnter++;

            for (int i = 0; i < 8; i++) {
                data[i] = cnter;
            }
            uint8_t result;
            for (int i = 0; i < 2; i++) {
                result = m24m01_byte_write(0x50, i, cnter);
            }
            printf("wrote data 1 by 1: %d\n", result);

            return false;
        }
        case KC_EEPROM_READ: {
            if (record->event.pressed) return false;
            for (int i = 0; i < 8; i++) {
                data[i] = 0xFF;
            }

            uint8_t result;
            // read first 8 bytes starting at 0/0
            result = m24m01_random_byte_read(0x50, 0, &data[0]);
            for (int i = 0; i < 7; i++) {
                result = m24m01_byte_read(0x50, &data[i+1]);
            }

            printf("read %d byte 1 by 1 from eeprom with result: %d\n", sizeof(data), result);
            for (int i = 0; i < 8; i++) {
                printf("%2X", data[i]);
                if (i < 7) printf(", ");
            }
            printf("\n");
            return false;
        }
        case KC_RST: // Custom RESET code
            if (!record->event.pressed) {
                // reset_keyboard();
                uint8_t error, address;
                uint8_t nDevices;

                printf("Scanning...\n");

                nDevices = 0;
                for(address = 1; address < 255; address++ )
                {
                    // The i2c_scanner uses the return value of
                    // the Write.endTransmisstion to see if
                    // a device did acknowledge to the address.
                    error = i2c2_isDeviceReady(address, EEPROM_LONG_TIMEOUT);

                    if (error == 0)
                    {
                        printf("I2C device found at address %x\n", address);

                        nDevices++;
                    }
                    else
                    {
                        printf("error %d at address %x\n", error, address);
                    }
                }

                printf("done: found %d\n", nDevices);
            }
            return false;
        case QWERTY:
            return false;
            break;
        case LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_LOWER);
                update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
            }
            return false;
            break;
        case RAISE:
            if (record->event.pressed) {
                layer_on(_RAISE);
                update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_RAISE);
                update_tri_layer_RGB(_LOWER, _RAISE, _ADJUST);
            }
            return false;
            break;
        case ADJUST:
            if (record->event.pressed) {
                layer_on(_ADJUST);
            } else {
                layer_off(_ADJUST);
            }
            return false;
            break;
    }
    return true;
}
