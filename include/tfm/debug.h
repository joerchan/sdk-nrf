#pragma once

#include <stdint.h>
#include <nrfx.h>
/* hal_gpio.h */


static inline uint32_t m_gpio_port_pin_get(uint32_t pin_number)
{
#if defined(NRF_P1)
    return pin_number % 32;
#else
    return pin_number;
#endif
}

static inline NRF_GPIO_Type * m_gpio_port_get(uint32_t pin_number)
{
#if defined(NRF_P1)
    return (pin_number < 32) ? NRF_P0 : NRF_P1;
#else
    return NRF_P0;
#endif
}

static inline void hal_gpio_pin_set(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    p_gpio_port->OUTSET = 1 << port_pin;
}

static inline void hal_gpio_pin_clear(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    p_gpio_port->OUTCLR = 1 << port_pin;
}

static inline void hal_gpio_pin_toggle(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);
    uint32_t pins = p_gpio_port->OUT;

    p_gpio_port->OUTSET = ~pins & (1UL << port_pin);
    p_gpio_port->OUTCLR = pins & (1UL << port_pin);
}

static inline uint32_t hal_gpio_pin_read(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    return (p_gpio_port->IN >> port_pin) & 1UL;
}

static inline void hal_gpio_cfg_input(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    p_gpio_port->PIN_CNF[port_pin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
                                     (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                     (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                                     (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                     (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
}

static inline void hal_gpio_cfg_output(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    p_gpio_port->DIRSET = 1 << port_pin;
}

/* debug_pin.h */

// #if defined (NRF52840_XXAA) || defined(NRF52833_XXAA)
#define _DBP0  (32 +  0)
#define _DBP1  (32 +  1)
#define _DBP2  (32 +  4)
#define _DBP3  (32 +  5)
#define _DBP4  (32 +  6)
#define _DBP5  (32 +  7)
#define _DBP6  (32 +  8)
#define _DBP7  (32 +  9)

#define _DBP8  ( 0 + 29)
#define _DBP9  ( 0 + 24)
#define _DBP10 ( 0 + 22)
#define _DBP11 ( 0 + 20)
#define _DBP12 ( 0 + 18)
#define _DBP13 ( 0 + 16) /* Not working */
#define _DBP14 ( 0 + 14) /* Not working */
#define _DBP15 ( 0 + 12)

/* Introducing a delay on nRF52 to slow down pin toggling where needed */
#define DBP_DELAY                                                                                  \
    {                                                                                              \
        __NOP();                                                                                   \
        __NOP();                                                                                   \
        __NOP();                                                                                   \
        __NOP();                                                                                   \
    }
// #endif /* NRF52840_XXAA */

#define DEBUG_PIN_PORTA_BEGIN 0
#define DEBUG_PIN_PORTA_END 7
#define DEBUG_PIN_PORTB_BEGIN 8
#define DEBUG_PIN_PORTB_END 15
#define DEBUG_PIN_COUNT 16

#define DEBUG_PRINT(var) debug_printf(#var ":%d", var)

static inline uint32_t debug_pin(uint32_t pin)
{
#if !defined(UNIT_TEST)
    static const uint32_t _dbpin[DEBUG_PIN_COUNT] = {_DBP0,  _DBP1,  _DBP2,  _DBP3, _DBP4,  _DBP5,
                                                     _DBP6,  _DBP7,  _DBP8,  _DBP9, _DBP10, _DBP11,
                                                     _DBP12, _DBP13, _DBP14, _DBP15};

    return _dbpin[pin];
#else
    return pin;
#endif
}

static inline void debug_pin_enable(uint32_t pin)
{
    hal_gpio_cfg_output(debug_pin(pin));
}

static inline void debug_pin_range_enable(uint32_t begin, uint32_t end)
{
    for (uint32_t i = begin; i <= end; i++)
    {
        debug_pin_enable(i);
    }
}

static inline void debug_pin_set(uint32_t pin)
{
    hal_gpio_pin_set(debug_pin(pin));
}

static inline void debug_pin_clear(uint32_t pin)
{
    hal_gpio_pin_clear(debug_pin(pin));
}

static inline void debug_pin_toggle(uint32_t pin)
{
    hal_gpio_pin_toggle(debug_pin(pin));
}

static inline void debug_pin_range_test(uint32_t begin, uint32_t end)
{
    for (int i = begin; i <= end; i++)
    {
        debug_pin_set(i);
    }

    for (int i = begin; i <= end; i++)
    {
        debug_pin_clear(i);
    }
}
