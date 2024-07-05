#include "gpio.h"
#include "util.h"
#include "systick.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_PIN ( 7U )

static gpio_t * GPIO = ( gpio_t * ) GPIOB_BASE;

static void Loop( void )
{
    static uint32_t lastBlink = 0U;
    while( 1 )
    {
        while((SysTick_GetMS() - lastBlink) < 500U);
        GPIO->ODR |= LED_PIN;
        lastBlink = SysTick_GetMS();
    }
}

static void Init( void )
{
    /* Lazy way of enabling gpio b */
    *((uint32_t *)0x40021034) |= ( 0x1 << 1 );

    GPIO->MODER &= ~( 1 << 15 );
    GPIO->MODER |= ( 1 << 14 );

    /* SysTick Calibration value for 10ms tick as per ARM datasheet
     * 1MHz Clock / 100Hz tick = 0x2710
     * Need to subtract 1 because count ends at 0 so
     * Calibration value is 0x270F
     */
    SysTick_Init(0x270F);
}

int main ( void )
{
    Init();
    Loop();

    return 0;
}

