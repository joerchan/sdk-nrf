#pragma once

#include <stdint.h>
#include <nrfx.h>
/* hal_gpio.h */

typedef volatile uint32_t * hal_event_t;
#define HAL_EVENT_NONE NULL

typedef volatile uint32_t * hal_task_t;
#define HAL_TASK_NONE NULL

typedef enum
{
    HAL_GPIO_PIN_MCUSEL_APP        = GPIO_PIN_CNF_MCUSEL_AppMCU,     ///< Pin controlled by Application MCU.
    HAL_GPIO_PIN_MCUSEL_NETWORK    = GPIO_PIN_CNF_MCUSEL_NetworkMCU, ///< Pin controlled by Network MCU.
    HAL_GPIO_PIN_MCUSEL_PERIPHERAL = GPIO_PIN_CNF_MCUSEL_Peripheral, ///< Pin controlled by dedicated peripheral.
    HAL_GPIO_PIN_MCUSEL_TND        = GPIO_PIN_CNF_MCUSEL_TND,        ///< Pin controlled by Trace and Debug Subsystem.
} hal_gpio_pin_mcusel_t;


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

static inline uint32_t m_gpio_port_num_get(uint32_t pin_number)
{
#if defined(NRF_P1)
    return (pin_number < 32) ? 0 : 1;
#else
    return 0;
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

static inline void hal_gpio_mcusel_set(uint32_t pin, hal_gpio_pin_mcusel_t mcu)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    uint32_t cnf = p_gpio_port->PIN_CNF[port_pin] & ~GPIO_PIN_CNF_MCUSEL_Msk;
    p_gpio_port->PIN_CNF[port_pin] = cnf | (mcu << GPIO_PIN_CNF_MCUSEL_Pos);
}

static inline void hal_gpio_cfg_input(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

#if defined(GPIO_PIN_CNF_MCUSEL_Msk)
    /* Preserve MCUSEL setting. */
    uint32_t cnf = p_gpio_port->PIN_CNF[port_pin] & GPIO_PIN_CNF_MCUSEL_Msk;
#else
    uint32_t cnf = 0;
#endif

    cnf |= (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
           (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
           (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
           (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
           (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);

    p_gpio_port->PIN_CNF[port_pin] = cnf;
}

static inline void hal_gpio_cfg_output(uint32_t pin)
{
    NRF_GPIO_Type * p_gpio_port = m_gpio_port_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

    p_gpio_port->DIRSET = 1 << port_pin;
}

#define SUBSCRIBE_EN_Pos (31UL)                      /*!< Position of EN field (this is the same for all SUBSCRIBE-registers) */
#define SUBSCRIBE_EN_Msk (0x1UL << SUBSCRIBE_EN_Pos) /*!< Bit mask of EN field. */
#define PUBLISH_EN_Pos   (31UL)                      /*!< Position of EN field (this is the same for all PUBLISH-registers) */
#define PUBLISH_EN_Msk   (0x1UL << PUBLISH_EN_Pos)   /*!< Bit mask of EN field. */

static inline void _ppi_config(uint32_t ppi_ch, hal_event_t event, hal_task_t task)
{
#if defined(DPPI_PRESENT)
    *task = ppi_ch | SUBSCRIBE_EN_Msk;
    *event = ppi_ch | PUBLISH_EN_Msk;
    NRF_DPPIC->CHENSET = (1 << ppi_ch);
#elif defined(PPI_PRESENT)
    NRF_PPI->CH[ppi_ch].TEP    = (uint32_t)(task);
    NRF_PPI->CH[ppi_ch].EEP    = (uint32_t)(event);
    NRF_PPI->CHENSET = (1 << ppi_ch);
#else
    #error "Neither PPI nor DPPI are present"
#endif
}

static inline void _gpiote_config(uint32_t pin, uint32_t gpiote_ch, uint32_t polarity)
{
    uint32_t port_num = m_gpio_port_num_get(pin);
    uint32_t port_pin = m_gpio_port_pin_get(pin);

  NRF_GPIOTE->CONFIG[gpiote_ch] =
    (polarity << GPIOTE_CONFIG_POLARITY_Pos) |
#if defined(NRF_P1)
    (port_num                      << GPIOTE_CONFIG_PORT_Pos)     |
#endif
    (port_pin                      << GPIOTE_CONFIG_PSEL_Pos)     |
    (GPIOTE_CONFIG_MODE_Task       << GPIOTE_CONFIG_MODE_Pos)     |
    (0                             << GPIOTE_CONFIG_OUTINIT_Pos);
}

static uint8_t m_ppi_channel = 0;
static uint8_t m_gpiote_channel = 0;

static inline void _pin_event_config(uint32_t pin, hal_event_t event)
{
#ifdef NRF53_SERIES
    hal_task_t gpiote_task = &NRF_GPIOTE->SUBSCRIBE_OUT[m_gpiote_channel];
#else
    hal_task_t gpiote_task = &NRF_GPIOTE->TASKS_OUT[m_gpiote_channel];
#endif

    _ppi_config(m_ppi_channel, event, gpiote_task );
    m_ppi_channel++;

    _gpiote_config(pin, m_gpiote_channel, GPIOTE_CONFIG_POLARITY_Toggle);
    m_gpiote_channel++;
}

#if 0
#define HAL_GPIOTE_CHANNEL_NONE UINT8_MAX

static hal_gpiote_channel_t m_gpiote_channel_get(void)
{
    for (int i = 0; i < GPIOTE_CH_NUM; i++)
    {
        if (m_ppi_channels_used[i][PPI_CH_FIRST] == HAL_PPI_CHANNEL_NONE)
        {
            return i;
        }
    }

    return HAL_GPIOTE_CHANNEL_NONE;
}

hal_gpiote_channel_t hal_gpiote_pin_event_setup(uint32_t pin, hal_event_t event, bool enabled)
{
    hal_gpiote_channel_t gpiote_channel = m_gpiote_channel_get();

    if (gpiote_channel != HAL_GPIOTE_CHANNEL_NONE)
    {
        hal_ppi_channel_t ppi_channel =
            hal_ppi_channel_get(event, &NRF_GPIOTE->TASKS_OUT[gpiote_channel]);

        if (ppi_channel != HAL_PPI_CHANNEL_NONE)
        {
            if (enabled)
            {
                hal_ppi_channel_enable(ppi_channel);
            }
            else
            {
                hal_ppi_channel_disable(ppi_channel);
            }
            hal_gpio_cfg_output(pin);
            NRF_GPIOTE->CONFIG[gpiote_channel] =
                (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                (pin << GPIOTE_CONFIG_PSEL_Pos) |
                (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
                (0 << GPIOTE_CONFIG_OUTINIT_Pos);

            m_ppi_channels_used[gpiote_channel][PPI_CH_FIRST] = ppi_channel;
        }
    }

    return gpiote_channel;
}
#endif

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

static inline void debug_pin_event(uint32_t pin, hal_event_t event)
{
    _pin_event_config(debug_pin(pin), event);
    // hal_gpiote_pin_event_setup(debug_pin(pin), event, true);
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

static inline void debug_pin_cpuapp_enable(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    hal_gpio_cfg_output(debug_pin(pin));
#endif
}

static inline void debug_pin_cpuapp_range_enable(uint32_t begin, uint32_t end)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    for (uint32_t i = begin; i <= end; i++)
    {
        debug_pin_enable(i);
    }
#endif
}

static inline void debug_pin_cpuapp_set(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    hal_gpio_pin_set(debug_pin(pin));
#endif
}

static inline void debug_pin_cpuapp_clear(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    hal_gpio_pin_clear(debug_pin(pin));
#endif
}

static inline void debug_pin_cpuapp_toggle(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    hal_gpio_pin_toggle(debug_pin(pin));
#endif
}

static inline void debug_pin_cpuapp_event(uint32_t pin, hal_event_t event)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    _pin_event_config(debug_pin(pin), event);
    // hal_gpiote_pin_event_setup(debug_pin(pin), event, true);
#endif
}

static inline void debug_pin_cpuapp_range_test(uint32_t begin, uint32_t end)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    for (int i = begin; i <= end; i++)
    {
        debug_pin_set(i);
    }

    for (int i = begin; i <= end; i++)
    {
        debug_pin_clear(i);
    }
#endif
}


static inline void debug_pin_cpunet_enable(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUAPP)
    hal_gpio_mcusel_set(pin, HAL_GPIO_PIN_MCUSEL_NETWORK);
#endif
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    hal_gpio_cfg_output(debug_pin(pin));
#endif
}

static inline void debug_pin_cpunet_range_enable(uint32_t begin, uint32_t end)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    for (uint32_t i = begin; i <= end; i++)
    {
        debug_pin_cpunet_enable(i);
    }
#endif
}

static inline void debug_pin_cpunet_set(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    hal_gpio_pin_set(debug_pin(pin));
#endif
}

static inline void debug_pin_cpunet_clear(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    hal_gpio_pin_clear(debug_pin(pin));
#endif
}

static inline void debug_pin_cpunet_toggle(uint32_t pin)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    hal_gpio_pin_toggle(debug_pin(pin));
#endif
}

static inline void debug_pin_cpunet_event(uint32_t pin, hal_event_t event)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    _pin_event_config(debug_pin(pin), event);
    // hal_gpiote_pin_event_setup(debug_pin(pin), event, true);
#endif
}

static inline void debug_pin_cpunet_range_test(uint32_t begin, uint32_t end)
{
#if defined(CONFIG_SOC_NRF5340_CPUNET)
    for (int i = begin; i <= end; i++)
    {
        debug_pin_set(i);
    }

    for (int i = begin; i <= end; i++)
    {
        debug_pin_clear(i);
    }
#endif
}

