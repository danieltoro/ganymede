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
#define KC_EEPROM_RDY KC_F20
#define KC_ALTZ  ALT_T(KC_Z)
#define KC_GUI_EQ RGUI_T(KC_EQUAL)
#define KC_GUI_QUO GUI_T(KC_QUOTE)
#define KC_ALTSLSH ALT_T(KC_SLSH)

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

                          TRNS,  TRNS, TRNS,  TRNS,  EEPROM_RDY,  EEPROM_DUMP, \
                          TRNS,  TRNS, TRNS,  TRNS,  EEPROM_PAGE_WRITE,  EEPROM_WRITE, \
                          LCTRL, TRNS, TRNS,  TRNS,  TRNS,  EEPROM_READ, \
                          TRNS, TRNS, TRNS, RST
                        ),
};

uint8_t data[8] = {0,0,0,0,0,0,0,0};
uint8_t cnter = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{
    if (record->event.key.row >= 4) {
        right_hand_colors[record->event.key.row-4][record->event.key.col].r = 255;
        right_hand_colors[record->event.key.row-4][record->event.key.col].incrementR = -25;
        right_hand_colors[record->event.key.row-4][record->event.key.col].limitR = 255;
    } else {
        left_hand_colors[record->event.key.row][record->event.key.col].r = 255;
        left_hand_colors[record->event.key.row][record->event.key.col].incrementR = -25;
        left_hand_colors[record->event.key.row][record->event.key.col].limitR = 255;
    }

    switch (keycode) {
        case KC_EEPROM_RDY: {
            if (record->event.pressed) return false;

            uint8_t result = init_m24m01();
            if (result == 0) {
                xprintf("M24M01 ready\n");
            } else {
                xprintf("M24M01 not ready: %d\n", result);
            }
            return false;
        }
        case KC_EEPROM_PAGE_WRITE: {
            if (record->event.pressed) return false;
            cnter++;

            for (int i = 0; i < 8; i++) {
                data[i] = cnter;
            }
            uint8_t result;
            result = m24m01_page_write(EEPROM_ADDRESS, 0, &data[0], sizeof(data));
            result = result;
            xprintf("wrote %d bytes to eeprom with result: %d (%d) (expected: %2X)\n", sizeof(data), result, i2c2_getErrors(), cnter);
            return false;
        }
        case KC_EEPROM_DUMP: {
            if (record->event.pressed) return false;

            uint16_t totalNumPages = 1024/128;
            uint8_t data[128];
            for (uint8_t i = 0; i < totalNumPages; i++) {
                uint8_t result = m24m01_page_read(EEPROM_ADDRESS, i*128, &data[0], sizeof(data));
                result = result;
                xprintf("read %d byte from eeprom with result: %d\n", sizeof(data), result);
                for (int i = 0; i < 128; i++) {
                    xprintf("%2X", data[i]);
                    if (i < 127) xprintf(", ");
                }
                xprintf("\n");
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
                result = m24m01_byte_write(EEPROM_ADDRESS, i, cnter);
                result = result;
            }
            xprintf("wrote data 1 by 1: %d\n", result);

            return false;
        }
        case KC_EEPROM_READ: {
            if (record->event.pressed) return false;
            for (int i = 0; i < 8; i++) {
                data[i] = 0xFF;
            }

            uint8_t result;
            // read first 8 bytes starting at 0/0
            result = m24m01_random_byte_read(EEPROM_ADDRESS, 0, &data[0]);
            result = result;
            for (int i = 0; i < 7; i++) {
                result = m24m01_byte_read(EEPROM_ADDRESS, &data[i+1]);
            }

            xprintf("read %d byte 1 by 1 from eeprom with result: %d\n", sizeof(data), result);
            for (int i = 0; i < 8; i++) {
                xprintf("%2X", data[i]);
                if (i < 7) xprintf(", ");
            }
            xprintf("\n");
            return false;
        }
        case KC_RST: // Custom RESET code
            if (!record->event.pressed) {
                // reset_keyboard();
                uint8_t error, address;
                uint8_t numberOfDevices;

                numberOfDevices = 0;
                for(address = 1; address < 255; address++ )
                {
                    error = i2c2_isDeviceReady(address, EEPROM_LONG_TIMEOUT);

                    if (error == 0) {
                        xprintf("I2C device found at address %x\n", address);
                        numberOfDevices++;
                    }
                }

                xprintf("done: found %d\n", numberOfDevices);
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
