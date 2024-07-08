#include "gpio.h"

#define GPIOA_BASE   ( 0x40020000 )
#define GPIOB_BASE   ( 0x40020400 )
#define GPIOC_BASE   ( 0x40020800 )
#define GPIOD_BASE   ( 0x40020C00 )
#define GPIOE_BASE   ( 0x40041000 )

typedef struct
{
    uint32_t MODER:32;
    uint32_t OTYPER:32;
    uint32_t OSPEEDR:32;
    uint32_t PUPDR:32;
    uint32_t IDR:32;
    uint32_t ODR:32;
    uint32_t BSRR:32;
    uint32_t LCKR:32;
    uint32_t AFRL:32;
    uint32_t AFRH:32;
    uint32_t BRR:32;
} gpio_t;

static gpio_t * const GPIO = ( gpio_t * ) 0x40020000;

extern void GPIO_Init(void)
{
    /* Lazy way of enabling gpio a */
    *((uint32_t *)0x40023830) |= ( 0x1 << 0 );
}

extern void GPIO_ConfigureOutput(uint16_t pin)
{
    GPIO->MODER &= ~( 1 << ((pin << 1U) + 1U) );
    GPIO->MODER |= ( 1 << ((pin << 1U)) );
}

extern void GPIO_Toggle(uint16_t pin)
{
        GPIO->ODR ^= (1 << pin);
}

