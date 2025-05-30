/* Conway's game of life for the Trinket m0+ and a small lcd display
 * T.L. 2022
 * */

/* ATSAMD21E18 */

#include "util.h"
#include "systick.h"
#include <stdbool.h>

#define CORE_CLOCK ( 16000000U )

#define TIM2_BASE   ( 0x40000000 )
#define GPIOA_BASE  ( 0x48000000 )
#define GPIOB_BASE  ( 0x48000400 )
#define GPIOC_BASE  ( 0x48000800 )
#define GPIOD_BASE  ( 0x48000C00 )
#define GPIOE_BASE  ( 0x48001000 )

typedef struct
{
    uint32_t CR1:32;
    uint32_t CR2:32;
    uint32_t SMCR:32;
    uint32_t DIER:32;
    uint32_t SR:32;
    uint32_t EGR:32;
    uint32_t CCMR1:32;
    uint32_t CCMR2:32;
    uint32_t CCER:32;
    uint32_t CNT:32;
    uint32_t PSC:32;
    uint32_t ARR:32;
    uint32_t CCR1:32;
    uint32_t CCR2:32;
    uint32_t CCR3:32;
    uint32_t CCR4:32;
    uint32_t RESERVED0:32;
    uint32_t DCR:32;
    uint32_t DMAR:32;
    uint32_t TIM2_OR1:32;
    uint32_t TIM2_OR2:32;
}
timer_t;

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

static volatile timer_t * TIM2 = ( timer_t * ) TIM2_BASE;

static volatile gpio_t * GPIO_B = ( gpio_t * ) GPIOB_BASE;

#define PIN (3U)

static void ConfigureGPIO(void)
{
    *((uint32_t *)0x4002104C) |= ( 0x1 << 1 );
    
    GPIO_B->MODER &= ~( 1 << ((PIN << 1U) + 1U) );
    GPIO_B->MODER |= ( 1 << ((PIN << 1U)) );
    
    GPIO_B->ODR |= (1 << PIN);
}

int main ( void )
{
    ConfigureGPIO();
    while(1)
    {

    }
    return 0;
}

