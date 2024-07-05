#include "gpio.h"
#include "util.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 5U )

static gpio_t * GPIO = ( gpio_t * ) 0x40020000;

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 500U);
        GPIO->ODR ^= (1<<LED_PIN);
        lastBlink = SysTick_GetMS();
    }
}

static void Init( void )
{
    /* Lazy way of enabling gpio a */
    *((uint32_t *)0x40023830) |= ( 0x1 << 0 );

    GPIO->MODER &= ~( 1 << 11 );
    GPIO->MODER |= ( 1 << 10 );

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    SysTick_Init(0x270F);
    /* Globally Enable Interrupts */
    asm("CPSIE IF");
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}

